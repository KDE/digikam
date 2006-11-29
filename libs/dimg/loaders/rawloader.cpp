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
#include <qmutex.h>
#include <qwaitcondition.h>

// KDE includes.

#include <kprocess.h>
#include <kstandarddirs.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "dcrawbinary.h"
#include "rawloader.h"
#include "rawloader.moc"

namespace Digikam
{

class RAWLoaderPriv
{
public:

    RAWLoaderPriv()
    {
        observer   = 0;
        process    = 0;
        queryTimer = 0;
        running    = false;
        normalExit = false;
        data       = 0;
        dataPos    = 0;
        width      = 0;
        height     = 0;
        rgbmax     = 0;
    }
    
    bool                running;
    bool                normalExit;

    uchar              *data;
    
    int                 dataPos;
    int                 width;
    int                 height;
    int                 rgbmax;

    QString             filePath;

    QMutex              mutex;
    
    QWaitCondition      condVar;
    
    QTimer             *queryTimer;

    KProcess           *process;
    
    DImgLoaderObserver *observer;

    RawDecodingSettings rawDecodingSettings;
};

RAWLoader::RAWLoader(DImg* image, RawDecodingSettings rawDecodingSettings)
         : DImgLoader(image)
{
    d = new RAWLoaderPriv;
    d->rawDecodingSettings = rawDecodingSettings;
}

RAWLoader::~RAWLoader()
{
    delete d;
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
    d->observer   = observer;
    d->filePath   = filePath;
    d->running    = true;
    d->normalExit = false;

    // trigger startProcess and loop to wait dcraw decoding
    QApplication::postEvent(this, new QCustomEvent(QEvent::User));


    // The time from starting dcraw to when it first outputs something takes
    // much longer than the time while it outputs the data and the time while
    // we process the data.
    // We do not have progress information for this, but it is much more promising to the user
    // if there is progress which does not stay at a fixed value.
    // So we make up some progress (0% - 90%), using the file size as an indicator how long it might take.
    QTime dcrawStartTime = QTime::currentTime();
    int fileSize = QFileInfo(d->filePath).size();
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
    while (d->running && !d->observer->isShuttingDown())
    {
        if (d->dataPos == 0)
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
        else if (d->dataPos > checkpoint)
        {
            // While receiving data, progress from 90% to 95%
            int size = d->width * d->height * (d->rawDecodingSettings.sixteenBitsImage ? 6 : 3);
            checkpoint += granularity(observer, size, 0.05);
            if (observer)
                observer->progressInfo(m_image, 0.9 + 0.05*(((float)d->dataPos)/((float)size)) );
        }

        QMutexLocker lock(&d->mutex);
        d->condVar.wait(&d->mutex, 10);
        //DDebug() << "Waiting for dcraw, is running " << process.isRunning() << endl;
    }

    if (!d->normalExit)
    {
        delete [] d->data;
        d->data = 0;
        return false;
    }

    // -------------------------------------------------------------------
    // Get image data

    if (d->rawDecodingSettings.sixteenBitsImage)       // 16 bits image
    {
        uchar *image = new uchar[d->width*d->height*8];

        unsigned short *dst = (unsigned short *)image;
        uchar          *src = d->data;
        float fac           = 65535.0 / d->rgbmax;
        checkpoint          = 0;

        for (int h = 0; h < d->height; h++)
        {

            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, d->height, 0.1);
                if (!observer->continueQuery(m_image))
                {
                    delete [] d->data;
                    d->data = 0;
                    return false;
                }
                observer->progressInfo(m_image, 0.95 + 0.05*(((float)h)/((float)d->height)) );
            }

            for (int w = 0; w < d->width; w++)
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
        uchar *image = new uchar[d->width*d->height*4];
        uchar *dst   = image;
        uchar *src   = d->data;
        checkpoint   = 0;

        for (int h = 0; h < d->height; h++)
        {

            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, d->height, 0.1);
                if (!observer->continueQuery(m_image))
                {
                    delete [] d->data;
                    d->data = 0;
                    return false;
                }
                observer->progressInfo(m_image, 0.95 + 0.05*(((float)h)/((float)d->height)) );
            }

            for (int w = 0; w < d->width; w++)
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

    delete [] d->data;
    d->data = 0;

    //----------------------------------------------------------

    imageWidth()  = d->width;
    imageHeight() = d->height;
    imageSetAttribute("format", "RAW");

    return true;
}

void RAWLoader::customEvent(QCustomEvent *)
{
    // KProcess (because of QSocketNotifier) is not reentrant.
    // We must only use it from the main thread.
    startProcess();

    // set up timer to call continueQuery at regular intervals
    if (d->running)
    {
        d->queryTimer = new QTimer;
        connect(d->queryTimer, SIGNAL(timeout()),
                this, SLOT(slotContinueQuery()));
        d->queryTimer->start(30);
    }
}

void RAWLoader::startProcess()
{
    if (d->observer)
    {
        if (!d->observer->continueQuery(m_image))
        {
            d->running    = false;
            d->normalExit = false;
            return;
        }
    }

    // create KProcess and build argument list

    d->process = new KProcess;

    connect(d->process, SIGNAL(processExited(KProcess *)),
            this, SLOT(slotProcessExited(KProcess *)));
             
    connect(d->process, SIGNAL(receivedStdout(KProcess *, char *, int)),
            this, SLOT(slotReceivedStdout(KProcess *, char *, int)));
             
    connect(d->process, SIGNAL(receivedStderr(KProcess *, char *, int)),
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

    *d->process << DcrawBinary::instance()->path();
    *d->process << "-c";

    if (d->rawDecodingSettings.sixteenBitsImage)
        *d->process << "-4";

    if (d->rawDecodingSettings.halfSizeColorImage)
        *d->process << "-h";

    if (d->rawDecodingSettings.cameraColorBalance)
        *d->process << "-w";

    if (d->rawDecodingSettings.automaticColorBalance)
        *d->process << "-a";

    if (d->rawDecodingSettings.RGBInterpolate4Colors)
        *d->process << "-f";

    if (d->rawDecodingSettings.SuperCCDsecondarySensor)
        *d->process << "-s";

    *d->process << "-H";
    *d->process << QString::number(d->rawDecodingSettings.unclipColors);

    *d->process << "-b";
    *d->process << QString::number(d->rawDecodingSettings.brightness);

    *d->process << "-q";
    *d->process << QString::number(d->rawDecodingSettings.RAWQuality);

    if (d->rawDecodingSettings.enableNoiseReduction)
    {
        *d->process << "-B";
        *d->process << QString::number(d->rawDecodingSettings.NRSigmaDomain);
        *d->process << QString::number(d->rawDecodingSettings.NRSigmaRange);
    }

    *d->process << "-o";
    *d->process << QString::number( d->rawDecodingSettings.outputColorSpace );

    // -----------------------------------------------------------------
    
    *d->process << QFile::encodeName( d->filePath );

#ifdef ENABLE_DEBUG_MESSAGES
    DDebug() << "Running RAW decoding command " << d->process->args() << endl;
#endif

    // actually start the process
    if ( !d->process->start(KProcess::NotifyOnExit, KProcess::Communication(KProcess::Stdout | KProcess::Stderr)) )
    {
        DError() << "Failed to start RAW decoding" << endl;
        delete d->process;
        d->process    = 0;
        d->running    = false;
        d->normalExit = false;
        return;
    }
}

void RAWLoader::slotContinueQuery()
{
    // this is called from the timer

    if (d->observer)
    {
        if (!d->observer->continueQuery(m_image))
        {
            d->process->kill();
            d->process->wait();
            d->normalExit = false;
        }
    }
}

void RAWLoader::slotProcessExited(KProcess *)
{
    // set variables, clean up, wake up loader thread

    QMutexLocker lock(&d->mutex);
    d->running    = false;
    d->normalExit = d->process->normalExit() && d->process->exitStatus() == 0; 
    delete d->process;
    d->process    = 0;
    delete d->queryTimer;
    d->queryTimer = 0;
    d->condVar.wakeAll();
}

void RAWLoader::slotReceivedStdout(KProcess *, char *buffer, int buflen)
{
    if (!d->data)
    {
        // first data packet:
        // Parse PPM header to find out size and allocate buffer

        // PPM header is "P6 <width> <height> <maximum rgb value "
        // where the blanks are newline characters

        QString magic = QString::fromAscii(buffer, 2);
        if (magic != "P6") 
        {
            DError() << "Cannot parse header from RAW decoding: Magic is " << magic << endl;
            d->process->kill();
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
        //DDebug() << "Header: " << QString::fromAscii(buffer, i) << endl;
        QStringList sizes = QStringList::split(" ", splitlist[1]);
        if (splitlist.size() < 3 || sizes.size() < 2)
        {
            DError() << "Cannot parse header from RAW decoding: Could not split" << endl;
            d->process->kill();
            return;
        }

        d->width  = sizes[0].toInt();
        d->height = sizes[1].toInt();
        d->rgbmax = splitlist[2].toInt();

#ifdef ENABLE_DEBUG_MESSAGES
        DDebug() << "Parsed PPM header: width " << d->width << " height " 
                  << d->height << " rgbmax " << d->rgbmax << endl;
#endif

        // cut header from data for memcpy below
        buffer += i;
        buflen -= i;

        // allocate buffer
        d->data    = new uchar[d->width * d->height * (d->rawDecodingSettings.sixteenBitsImage ? 6 : 3)];
        d->dataPos = 0;
    }

    // copy data to buffer
    memcpy(d->data + d->dataPos, buffer, buflen);
    d->dataPos += buflen;
}

void RAWLoader::slotReceivedStderr(KProcess *, char *buffer, int buflen)
{
    QCString message(buffer, buflen);
    DDebug() << "RAW decoding StdErr: " << message << endl;
}

bool RAWLoader::save(const QString&, DImgLoaderObserver *)
{
    // RAW files are always Read only.
    return false;
}

bool RAWLoader::sixteenBit() const
{
    return d->rawDecodingSettings.sixteenBitsImage;
}

}  // NameSpace Digikam


