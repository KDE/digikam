/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 *          Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date   : 2005-11-01
 * Description : A digital camera RAW files loader for DImg 
 *               framework using an external dcraw instance.
 *
 * Copyright 2005-2006 by Gilles Caulier and Marcel Wiesweg
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// This line must be commented to prevent any latency time
// when we use threaded image loader interface for each image
// files io. Uncomment this line only for debugging.
#define ENABLE_DEBUG_MESSAGES

// C++ includes.

#include <cstdio>
#include <cstdlib>
#include <cmath>

// QT includes.

#include <qapplication.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qcstring.h>
#include <qimage.h>
#include <qtimer.h>

// KDE includes.

#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>

// Local includes.

#include "dimg.h"
#include "dimgloaderobserver.h"
#include "dcrawbinary.h"
#include "rawloader.h"

namespace Digikam
{

RAWLoader::RAWLoader(DImg* image, RawDecodingSettings rawDecodingSettings)
         : DImgLoader(image)
{
    m_sixteenBit          = rawDecodingSettings.sixteenBitsImage;
    m_rawDecodingSettings = rawDecodingSettings;

    m_observer            = 0;
    m_process             = 0;
    m_queryTimer          = 0;
    m_running             = false;
    m_normalExit          = false;
    m_data                = 0;
    m_dataPos             = 0;

    m_width               = 0;
    m_height              = 0;
    m_rgbmax              = 0;
}

bool RAWLoader::load(const QString& filePath, DImgLoaderObserver *observer)
{
    readMetadata(filePath, DImg::RAW);
    
    // NOTE: Here, we don't check a possible embeded work-space color profile using 
    // the method checkExifWorkingColorSpace() like with JPEG, PNG, and TIFF loaders, 
    // because RAW file are always in linear mode.
    
    return ( loadFromDcraw(filePath, observer) );
}

bool RAWLoader::loadFromDcraw(const QString& filePath, DImgLoaderObserver *observer)
{
    m_observer = observer;
    m_filePath = filePath;
    m_running  = true;
    m_normalExit = false;

    // trigger startProcess and loop to wait dcraw decoding
    QApplication::postEvent(this, new QCustomEvent(QEvent::User));


    // The time from starting dcraw to when it first outputs something takes
    // much longer than the time while it outputs the data and the time while
    // we process the data.
    // We do not have progress information for this, but it is much more promising to the user
    // if there is progress which does not stay at a fixed value.
    // So we make up some progress (0% - 90%), using the file size as an indicator how long it might take.
    QTime dcrawStartTime = QTime::currentTime();
    int fileSize = QFileInfo(m_filePath).size();
    // This is the magic number that describes how fast the function grows
    // It _should_ be dependent on how fast the computer is, but we dont have this piece of information
    // So this is a number that works well on my computer.
    double K50 = 3000.0*fileSize;
    int checkpointTime = 0;

    int checkpoint = 0;

    // The shuttingDown is a hack needed to prevent hanging when this KProcess-based loader
    // is waiting for the process to finish, but the main thread is waiting
    // for the thread to finish and no KProcess events are delivered.
    // Remove when porting to Qt4.
    while (m_running && !m_observer->isShuttingDown())
    {

        if (m_dataPos == 0)
        {
            int elapsedMsecs = dcrawStartTime.msecsTo(QTime::currentTime());
            if (elapsedMsecs > checkpointTime)
            {
                checkpointTime += 300;
            }
            if (observer)
            {
                // What we do here is a sigmoidal curve, it starts slowly,
                // then grows more rapidly, slows down again and
                // get asymptotically closer to the maximum.
                // (this is the Hill Equation, 2.8 the Hill Coefficient, to pour some blood in this)
                double elapsedMsecsPow = pow(elapsedMsecs, 2.8);
                double part = (elapsedMsecsPow) / (K50 + elapsedMsecsPow);
                observer->progressInfo(m_image, 0.9*part );
            }
        }
        else if (m_dataPos > checkpoint)
        {
            // While receiving data, progress from 90% to 95%
            int size = m_width * m_height * (m_rawDecodingSettings.sixteenBitsImage ? 6 : 3);
            checkpoint += granularity(observer, size, 0.05);
            if (observer)
                observer->progressInfo(m_image, 0.9 + 0.05*(((float)m_dataPos)/((float)size)) );
        }

        QMutexLocker lock(&m_mutex);
        m_condVar.wait(&m_mutex, 10);
        //kdDebug() << "Waiting for dcraw, is running " << process.isRunning() << endl;
    }

    if (!m_normalExit)
    {
        delete [] m_data;
        m_data = 0;
        return false;
    }

    // -------------------------------------------------------------------
    // Get image data

    if (m_rawDecodingSettings.sixteenBitsImage)       // 16 bits image
    {
        uchar *image = new uchar[m_width*m_height*8];

        unsigned short *dst = (unsigned short *)image;
        uchar          *src = m_data;
        float fac           = 65535.0 / m_rgbmax;
        checkpoint          = 0;

        for (int h = 0; h < m_height; h++)
        {

            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, m_height, 0.1);
                if (!observer->continueQuery(m_image))
                {
                    delete [] m_data;
                    m_data = 0;
                    return false;
                }
                observer->progressInfo(m_image, 0.95 + 0.05*(((float)h)/((float)m_height)) );
            }

            for (int w = 0; w < m_width; w++)
            {
                dst[0] = (unsigned short)((src[4]*256 + src[5]) * fac);      // Blue
                dst[1] = (unsigned short)((src[2]*256 + src[3]) * fac);      // Green
                dst[2] = (unsigned short)((src[0]*256 + src[1]) * fac);      // Red
                dst[3] = 0xFFFF;

                dst += 4;
                src += 6;
            }
        }

        imageData() = (uchar *)image;
    }
    else        // 8 bits image
    {
        uchar *image = new uchar[m_width*m_height*4];
        uchar *dst   = image;
        uchar *src   = m_data;
        checkpoint   = 0;

        for (int h = 0; h < m_height; h++)
        {

            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, m_height, 0.1);
                if (!observer->continueQuery(m_image))
                {
                    delete [] m_data;
                    m_data = 0;
                    return false;
                }
                observer->progressInfo(m_image, 0.95 + 0.05*(((float)h)/((float)m_height)) );
            }

            for (int w = 0; w < m_width; w++)
            {
                // No need to adapt RGB components accordinly with rgbmax value because dcraw
                // always return rgbmax to 255 in 8 bits/color/pixels.

                dst[0] = src[2];    // Blue
                dst[1] = src[1];    // Green
                dst[2] = src[0];    // Red
                dst[3] = 0xFF;      // Alpha

                dst += 4;
                src += 3;
            }
        }

        imageData()  = image;
    }

    delete [] m_data;
    m_data = 0;

    //----------------------------------------------------------

    imageWidth()  = m_width;
    imageHeight() = m_height;
    imageSetAttribute("format", "RAW");

    return true;
}

void RAWLoader::customEvent(QCustomEvent *)
{
    // KProcess (because of QSocketNotifier) is not reentrant.
    // We must only use it from the main thread.
    startProcess();

    // set up timer to call continueQuery at regular intervals
    if (m_running)
    {
        m_queryTimer = new QTimer;
        connect(m_queryTimer, SIGNAL(timeout()),
                this, SLOT(slotContinueQuery()));
        m_queryTimer->start(30);
    }
}

void RAWLoader::startProcess()
{
    if (m_observer)
    {
        if (!m_observer->continueQuery(m_image))
        {
            m_running    = false;
            m_normalExit = false;
            return;
        }
    }

    // create KProcess and build argument list

    m_process = new KProcess;

    connect(m_process, SIGNAL(processExited(KProcess *)),
            this, SLOT(slotProcessExited(KProcess *)));
             
    connect(m_process, SIGNAL(receivedStdout(KProcess *, char *, int)),
            this, SLOT(slotReceivedStdout(KProcess *, char *, int)));
             
    connect(m_process, SIGNAL(receivedStderr(KProcess *, char *, int)),
            this, SLOT(slotReceivedStderr(KProcess *, char *, int)));

    // run dcraw with options:
    // -c : write to stdout
    //
    // -4 : 16bit ppm output
    //
    // -f : Interpolate RGB as four colors. This blurs the image a little, but it eliminates false 2x2 mesh patterns.
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible
    // -n : Don't clip colors
    // -s : Use secondary pixels (Fuji Super CCD SR only)
    // -q : Use simple bilinear interpolation for quick results. Warning: this option require arguments 
    //      depending dcraw version (look below)
    // -B : Use bilateral filter to smooth noise while preserving edges.
    // -p : Use the input ICC profiles to define the camera's raw colorspace.
    // -o : Use ICC profiles to define the output colorspace.
    // -h : Output a half-size color image. Twice as fast as -q 0.

    *m_process << DcrawBinary::instance()->path();
    *m_process << "-c";

    if (m_rawDecodingSettings.sixteenBitsImage)
        *m_process << "-4";

    if (m_rawDecodingSettings.halfSizeColorImage)
        *m_process << "-h";

    if (m_rawDecodingSettings.cameraColorBalance)
        *m_process << "-w";

    if (m_rawDecodingSettings.automaticColorBalance)
        *m_process << "-a";

    if (m_rawDecodingSettings.RGBInterpolate4Colors)
        *m_process << "-f";

    if (m_rawDecodingSettings.SuperCCDsecondarySensor)
        *m_process << "-s";

    *m_process << "-H";
    *m_process << QString::number(m_rawDecodingSettings.unclipColors);

    *m_process << "-b";
    *m_process << QString::number(m_rawDecodingSettings.brightness);

    *m_process << "-q";
    *m_process << QString::number(m_rawDecodingSettings.RAWQuality);

    if (m_rawDecodingSettings.enableNoiseReduction)
    {
        *m_process << "-B";
        *m_process << QString::number(m_rawDecodingSettings.NRSigmaDomain);
        *m_process << QString::number(m_rawDecodingSettings.NRSigmaRange);
    }

    *m_process << "-o";
    *m_process << QString::number( m_rawDecodingSettings.outputColorSpace );

    // -----------------------------------------------------------------
    
    *m_process << QFile::encodeName( m_filePath );

#ifdef ENABLE_DEBUG_MESSAGES
    kdDebug() << "Running dcraw command " << m_process->args() << endl;
#endif

    // actually start the process
    if ( !m_process->start(KProcess::NotifyOnExit, KProcess::Communication(KProcess::Stdout | KProcess::Stderr)) )
    {
        kdError() << "Failed to start dcraw" << endl;
        delete m_process;
        m_process    = 0;
        m_running    = false;
        m_normalExit = false;
        return;
    }
}

void RAWLoader::slotContinueQuery()
{
    // this is called from the timer

    if (m_observer)
    {
        if (!m_observer->continueQuery(m_image))
        {
            m_process->kill();
            m_process->wait();
            m_normalExit = false;
        }
    }
}

void RAWLoader::slotProcessExited(KProcess *)
{
    // set variables, clean up, wake up loader thread

    QMutexLocker lock(&m_mutex);
    m_running = false;
    m_normalExit = m_process->normalExit();
    delete m_process;
    m_process = 0;
    delete m_queryTimer;
    m_queryTimer = 0;
    m_condVar.wakeAll();
}

void RAWLoader::slotReceivedStdout(KProcess *, char *buffer, int buflen)
{
    if (!m_data)
    {
        // first data packet:
        // Parse PPM header to find out size and allocate buffer

        // PPM header is "P6 <width> <height> <maximum rgb value "
        // where the blanks are newline characters

        QString magic = QString::fromAscii(buffer, 2);
        if (magic != "P6") 
        {
            kdError() << "Cannot parse header from dcraw: Magic is " << magic << endl;
            m_process->kill();
            return;
        }

        // Find the third newline that marks the header end in a dcraw generated ppm.
        int i       = 0;
        int counter = 0;

        while (i < buflen) 
        {
            if (counter == 3) break;
            if (buffer[i] == '\n') 
            {
                counter++;
            }
            ++i;
        }

        QStringList splitlist = QStringList::split("\n", QString::fromAscii(buffer, i));
        //kdDebug() << "Header: " << QString::fromAscii(buffer, i) << endl;
        QStringList sizes = QStringList::split(" ", splitlist[1]);
        if (splitlist.size() < 3 || sizes.size() < 2)
        {
            kdError() << "Cannot parse header from dcraw: Could not split" << endl;
            m_process->kill();
            return;
        }

        m_width  = sizes[0].toInt();
        m_height = sizes[1].toInt();
        m_rgbmax = splitlist[2].toInt();

#ifdef ENABLE_DEBUG_MESSAGES
        kdDebug() << "Parsed PPM header: width " << m_width << " height " 
                  << m_height << " rgbmax " << m_rgbmax << endl;
#endif

        // cut header from data for memcpy below
        buffer += i;
        buflen -= i;

        // allocate buffer
        m_data    = new uchar[m_width * m_height * (m_rawDecodingSettings.sixteenBitsImage ? 6 : 3)];
        m_dataPos = 0;
    }

    // copy data to buffer
    memcpy(m_data + m_dataPos, buffer, buflen);
    m_dataPos += buflen;
}

void RAWLoader::slotReceivedStderr(KProcess *, char *buffer, int buflen)
{
    QCString message(buffer, buflen);
    kdDebug() << "Dcraw StdErr: " << message << endl;
}

bool RAWLoader::save(const QString&, DImgLoaderObserver *)
{
    // RAW files are always Read only.
    return false;
}

bool RAWLoader::sixteenBit() const
{
    return m_sixteenBit;
}

}  // NameSpace Digikam

#include "rawloader.moc"

