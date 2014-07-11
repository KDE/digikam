/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-03
 * Description : Greycstoration interface.
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

// NOTE: Veaceslav cherry pick
/** Don't use CImg interface (keyboard/mouse interaction) */
// #define cimg_display 0
// /** Only print debug information on the console */
// #define cimg_debug 1

#include "greycstorationfilter.h"

// C++ includes

#include <cassert>

// KDE includes

#include <kdebug.h>

// includes for GreycStoration
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QTimer>
#include <QCoreApplication>
#include <QTime>
#include "dynamicthread.h"

// #define cimg_plugin "greycstoration.h"

/** Uncomment this line if you use future GreycStoration implementation with GFact parameter
 */
// #define GREYSTORATION_USING_GFACT 1 // NOTE: Veaceslav cherry pick

// CImg includes

#include "CImg.h"
#include "gmic.h"
#include "gmicinterface.h"


// NOTE: Veaceslav cherry pick
// extern "C"
// {
// #include <unistd.h>
// }

using namespace cimg_library;

namespace Digikam
{

class GreycstorationFilter::Private
{

public:

    Private() :
//         gfact(1.0),
//         computationThreads(2),
        mode(GreycstorationFilter::Restore)
//         threadManager(new CImg<>::GreycstorationThreadManager)
    {
    }

    ~Private()
    {
//         delete threadManager;
    }

public:

//     float                                gfact;
    int mode;

//     int                                  computationThreads;  // Number of threads used by CImg during computation.
//     int                                  mode;                // The interface running mode.

    QSize                                newSize;
    QImage                               inPaintingMask;      // Mask for inpainting.

    GreycstorationContainer              settings;            // Current Greycstoraion algorithm settings.

    CImg<>                               img;                 // Main image.
    CImg<uchar>                          mask;                // The mask used with inpaint or resize mode
    QThread*                             thread;
    GMicInterface*                       gmicInterface;
    QTimer*                               timer;


//     CImg<>::GreycstorationThreadManager* threadManager;
};

GreycstorationFilter::GreycstorationFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    setOriginalImage(DImg());
    setSettings(GreycstorationContainer());
    setMode(Restore);
    setInPaintingMask(QImage());
    initGmicInterface();
}

GreycstorationFilter::GreycstorationFilter(DImg* const orgImage,
                                           const GreycstorationContainer& settings,
                                           int mode,
                                           int newWidth, int newHeight,
                                           const QImage& inPaintingMask,
                                           QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    setOriginalImage(orgImage->copyImageData());
    setSettings(settings);
    setMode(mode, newWidth, newHeight);
    setInPaintingMask(inPaintingMask);
    setup();
    initGmicInterface();
}

GreycstorationFilter::~GreycstorationFilter()
{
    cancelFilter();
    delete d;
}

void GreycstorationFilter::setSettings(const GreycstorationContainer& settings)
{
    d->settings = settings;
}

void GreycstorationFilter::setMode(int mode, int newWidth, int newHeight)
{
    d->mode    = mode;
    d->newSize = QSize(newWidth, newHeight);
}

void GreycstorationFilter::setInPaintingMask(const QImage& inPaintingMask)
{
    d->inPaintingMask = inPaintingMask;
}

// NOTE: Veaceslav cherrypick
// void GreycstorationFilter::computeChildrenThreads()
// {
//     const int numProcs    = qMax(QThread::idealThreadCount(), 1);
//     const int maxThreads  = 16;
//     d->computationThreads = qMin(maxThreads, 2 + ((numProcs - 1) * 2));
//     kDebug() << "GreycstorationFilter::Computation threads: " << d->computationThreads;
// }

void GreycstorationFilter::setup()
{
//     computeChildrenThreads();
//
//     if (m_orgImage.sixteenBit())   // 16 bits image.
//     {
//         d->gfact = 1.0 / 256.0;
//     }

    if (d->mode == Resize || d->mode == SimpleResize)
    {
        m_destImage = DImg(d->newSize.width(), d->newSize.height(),
                           m_orgImage.sixteenBit(), m_orgImage.hasAlpha());

        kDebug() << "GreycstorationFilter::Resize: new size: ("
                 << d->newSize.width() << ", " << d->newSize.height() << ")";
    }
    else
    {
        m_destImage = DImg(m_orgImage.width(), m_orgImage.height(),
                           m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
    }

    initFilter();
}

QString GreycstorationFilter::cimgVersionString()
{
    return QString::number(gmic_version);
}

// We need to re-implement this method from DImgThreadedFilter class because
// target image size can be different from original if d->mode = Resize.

void GreycstorationFilter::initFilter()
{
    // (left out here: creation of m_destImage)

    if (m_master)
    {
        startFilterDirectly();
    }
}

void GreycstorationFilter::startFilterDirectly()
{
    if (m_orgImage.width() && m_orgImage.height())
    {
        emit started();

        m_wasCancelled = false;

            QDateTime now = QDateTime::currentDateTime();
            filterImage();
            kDebug() << m_name << ":: excecution time : " << now.msecsTo(QDateTime::currentDateTime()) << " ms";
//         }
//         catch (std::bad_alloc& ex)
//         {
//             //TODO: User notification
//             kError() << "Caught out-of-memory exception! Aborting operation" << ex.what();
//             emit finished(false);
//             return;
//         }

//         emit finished(!m_wasCancelled);
    }
    else  // No image data
    {
//         emit finished(false);
        kDebug() << m_name << "::No valid image data !!! ...";
    }
}

void GreycstorationFilter::initGmicInterface()
{
    d->thread = new QThread();
    d->gmicInterface = new GMicInterface();
    d->gmicInterface->moveToThread(d->thread);
    connect(d->thread, SIGNAL(finished()),
            d->gmicInterface, SLOT(deleteLater()));
    connect(this, SIGNAL(signalStartWork()),
            d->gmicInterface, SLOT(runGmic()),Qt::QueuedConnection);
    connect(d->gmicInterface, SIGNAL(signalResultReady(bool)),
            this, SLOT(setImageAfterProcessing(bool)));

    d->thread->start();
    d->timer = new QTimer();
    connect(d->timer, SIGNAL(timeout()), this, SLOT(iterationLoop()));
}

void GreycstorationFilter::setImageAfterProcessing(bool result)
{

    d->timer->stop();

    if(result)
    {
        register int x, y;
        d->img = d->gmicInterface->getImg();


            kDebug() << "Finalization..." << d->img.width() << " " << d->img.height();

        uchar* const newData = m_destImage.bits();
        int newWidth         = m_destImage.width();
        int newHeight        = m_destImage.height();

        if (!m_orgImage.sixteenBit())           // 8 bits image.
        {
            uchar* ptr = newData;

            for (y = 0; y < newHeight; ++y)
            {
                for (x = 0; x < newWidth; ++x)
                {
                    // Overwrite RGB values to destination.
                    ptr[0] = static_cast<uchar>(d->img(x, y, 0));        // Blue
                    ptr[1] = static_cast<uchar>(d->img(x, y, 1));        // Green
                    ptr[2] = static_cast<uchar>(d->img(x, y, 2));        // Red
                    ptr[3] = static_cast<uchar>(d->img(x, y, 3));        // Alpha
                    ptr    += 4;
                }
            }
        }
        else                                     // 16 bits image.
        {
            unsigned short* ptr = reinterpret_cast<unsigned short*>(newData);

            for (y = 0; y < newHeight; ++y)
            {
                for (x = 0; x < newWidth; ++x)
                {
                    // Overwrite RGB values to destination.
                    ptr[0] = static_cast<unsigned short>(d->img(x, y, 0));        // Blue
                    ptr[1] = static_cast<unsigned short>(d->img(x, y, 1));        // Green
                    ptr[2] = static_cast<unsigned short>(d->img(x, y, 2));        // Red
                    ptr[3] = static_cast<unsigned short>(d->img(x, y, 3));        // Alpha
                    ptr    += 4;
                }
            }
        }
    }
    emit finished(result);
}

void GreycstorationFilter::cancelFilter()
{
    if(d->thread->isRunning())
    {
        d->gmicInterface->cancel();
    }
    d->timer->stop();
    d->thread->exit();
    kDebug() << "Thread exited";
    /**
     * Wait 100 ms for thread to exit, if no, terminate him
     */


    /** NOTE: Ugly solution, but gmic do not have a good support for cancel,
     * and we are forced to kill the thread which run it.
     */
//     if(!d->thread->isFinished())
//     {
//         kDebug() << "Killing Thread: Note this one throws exception";
//         d->thread->terminate();
//     }

    while(!d->thread->isFinished())
    {
        QTime dieTime = QTime::currentTime().addMSecs(100);
        while( QTime::currentTime() < dieTime )
        {
            QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
        }
    }
    // And now when stop main loop and clean up all
    DImgThreadedFilter::cancelFilter();
}

void GreycstorationFilter::filterImage()
{
    register int x, y;

    kDebug() << "Initialization...";

    uchar* const data = m_orgImage.bits();
    int width         = m_orgImage.width();
    int height        = m_orgImage.height();

    // convert DImg (interleaved RGBA) to CImg (planar RGBA)
    if (!m_orgImage.sixteenBit())           // 8 bits image.
    {
        // NOTE: Veaceslav cherrypick
//         d->img = CImg<unsigned char>(data, 4, width, height, 1, false).
//                  get_permute_axes("yzvx");
        d->img     = CImg<uchar>(width, height, 1, 4);
        uchar* ptr = data;

        // FIXME: Test this one
        for (y = 0; y < height; ++y)
        {
            for (x = 0; x < width; ++x)
            {
                d->img(x, y, 0) = ptr[0];        // Blue.
                d->img(x, y, 1) = ptr[1];        // Green.
                d->img(x, y, 2) = ptr[2];        // Red.
                d->img(x, y, 3) = ptr[3];        // Alpha.
                ptr += 4;
            }
        }
    }
    else                                    // 16 bits image.
    {
//         d->img = CImg<unsigned short>(reinterpret_cast<unsigned short*>(data), 4, width, height, 1, false).
//                  get_permute_axes("yzvx");

        d->img              = CImg<unsigned short>(width, height, 1, 4);
        unsigned short* ptr = (unsigned short*)data;

        for (y = 0; y < height; ++y)
        {
            for (x = 0; x < width; ++x)
            {
                d->img(x, y, 0) = ptr[0];        // Blue.
                d->img(x, y, 1) = ptr[1];        // Green.
                d->img(x, y, 2) = ptr[2];        // Red.
                d->img(x, y, 3) = ptr[3];        // Alpha.
                ptr += 4;
            }
        }
    }

    kDebug() << "Process Computation...";

//     try
//     {
        switch (d->mode)
        {
            case Restore:
                restoration();
                break;

            case InPainting:
                inpainting();
                break;

            case Resize:
                resize();
                break;

            case SimpleResize:
                simpleResize();
                break;
        }

        // harvest
//         d->threadManager->finish();
//     }
//     catch (...)        // Everything went wrong.
//     {
//         kDebug() << "Error during Greycstoration filter computation!";
//
//         return;
//     }

//     if (!runningFlag())
//     {
//         return;
//     }

    // Copy CImg onto destination.


}

void GreycstorationFilter::restoration()
{
// NOTE: Veaceslav cherry pick
//     for (uint iter = 0 ; runningFlag() && (iter < d->settings.nbIter) ; ++iter)
//     {
//         // This function will start a thread running one iteration of the GREYCstoration filter.
//         // It returns immediately, so you can do what you want after (update a progress bar for
//         // instance).
//         d->threadManager->start(d->img, d->settings.amplitude,
//                                 d->settings.sharpness,
//                                 d->settings.anisotropy,
//                                 d->settings.alpha,
//                                 d->settings.sigma,
// #ifdef GREYSTORATION_USING_GFACT
//                                 d->gfact,
// #endif
//                                 d->settings.dl,
//                                 d->settings.da,
//                                 d->settings.gaussPrec,
//                                 d->settings.interp,
//                                 d->settings.fastApprox,
//                                 d->settings.tile,
//                                 d->settings.btile,
//                                 d->computationThreads);
//
//         iterationLoop(iter);
//     }

//     int tile;
//     int btile;

//     try{
//         gmic_list<> image_list;
//         gmic_list<char> image_name;
//         image_list.assign(d->img);

        QString command;

        command.append(QString("-repeat %1 ").arg(d->settings.nbIter));       // Iterations
        command.append(QString("-smooth "));
        command.append(QString("%1,").arg(d->settings.amplitude));            // Amplitude
        command.append(QString("%1,").arg(d->settings.sharpness));            // Sharpness
        command.append(QString("%1,").arg(d->settings.anisotropy));           // Anisotropy
        command.append(QString("%1,").arg(d->settings.alpha));                // Gradient Smoothness
        command.append(QString("%1,").arg(d->settings.sigma));                // Tensor Smoothness
        command.append(QString("%1,").arg(d->settings.dl));                   // Spatial Precision
        command.append(QString("%1,").arg(d->settings.da));                   // Angular Precision
        command.append(QString("%1,").arg(d->settings.gaussPrec));            // Value Precision
        command.append(QString("%1,").arg(d->settings.interp));               // Interpolation
        command.append(QString("%1 ").arg(d->settings.fastApprox));           // Fast Approximation
        command.append(QString("-done"));

        kDebug() << command;

        d->gmicInterface->addImg(d->img);
        d->gmicInterface->setParallelCommand(command);
//         d->gmicInterface->runGmic();
//         kDebug() << " G Mic finished";
        emit signalStartWork();
        d->timer->start(1000);
//         gmic(command.toAscii().data(), image_list, image_name);

//         kDebug() << "Gmic finished";

//         d->img = image_list[0];
//      }
//     catch (gmic_exception& e)
//     {
//         kDebug() << "Error encountered when calling G'MIC: " << e.what();
//     }

}

void GreycstorationFilter::inpainting()
{
    if (!d->inPaintingMask.isNull())
    {
        // Copy the inpainting image data into a CImg type image with three channels and no alpha.

        register int x, y;

        d->mask    = CImg<uchar>(d->inPaintingMask.width(), d->inPaintingMask.height(), 1, 3);
        uchar* ptr = d->inPaintingMask.bits();

        for (y = 0; y < d->inPaintingMask.height(); ++y)
        {
            for (x = 0; x < d->inPaintingMask.width(); ++x)
            {
                d->mask(x, y, 0) = ptr[2];        // blue.
                d->mask(x, y, 1) = ptr[1];        // green.
                d->mask(x, y, 2) = ptr[0];        // red.
                ptr += 4;
            }
        }
    }
    else
    {
        kDebug() << "Inpainting image: mask is null!";
        stop();
        return;
    }

    QString command = "-print -inpaint[0] [1],0,3 ";

        kDebug() << "Inpaint command +++++++++++++++++++++++++++++++" << command;

        d->gmicInterface->addImg(d->img, d->mask);
        d->gmicInterface->setCommand(command);
        emit signalStartWork();
        d->timer->start(1000);

//         d->timer->start(1000);
// Veaceslav cherry pick
//     for (uint iter = 0 ; runningFlag() && (iter < d->settings.nbIter) ; ++iter)
//     {
//         // This function will start a thread running one iteration of the GREYCstoration filter.
//         // It returns immediately, so you can do what you want after (update a progress bar for
//         // instance).
//         d->threadManager->start(d->img, &d->mask,
//                                 d->settings.amplitude,
//                                 d->settings.sharpness,
//                                 d->settings.anisotropy,
//                                 d->settings.alpha,
//                                 d->settings.sigma,
// #ifdef GREYSTORATION_USING_GFACT
//                                 d->gfact,
// #endif
//                                 d->settings.dl,
//                                 d->settings.da,
//                                 d->settings.gaussPrec,
//                                 d->settings.interp,
//                                 d->settings.fastApprox,
//                                 d->settings.tile,
//                                 d->settings.btile,
//                                 d->computationThreads);
//
//         iterationLoop(iter);
//     }
// TODO: Implement this with g'mic
}

void GreycstorationFilter::resize()
{
//     const bool anchor       = true;   // Anchor original pixels.
//     const unsigned int init = 5;      // Initial estimate (1=block, 3=linear, 5=bicubic).
//     int w                   = m_destImage.width();
//     int h                   = m_destImage.height();

//     d->mask.assign(d->img.dimx(), d->img.dimy(), 1, 1, 255);
//
//     if (!anchor)
//     {
//         d->mask.resize(w, h, 1, 1, 1);
//     }
//     else
//     {
//         d->mask = !d->mask.resize(w, h, 1, 1, 4);
//     }
//
//     d->img.resize(w, h, 1, -100, init);

//     for (uint iter = 0 ; runningFlag() && (iter < d->settings.nbIter) ; ++iter)
//     {
//         // This function will start a thread running one iteration of the GREYCstoration filter.
//         // It returns immediately, so you can do what you want after (update a progress bar for
//         // instance).
//         d->threadManager->start(d->img, &d->mask,
//                                 d->settings.amplitude,
//                                 d->settings.sharpness,
//                                 d->settings.anisotropy,
//                                 d->settings.alpha,
//                                 d->settings.sigma,
// #ifdef GREYSTORATION_USING_GFACT
//                                 d->gfact,
// #endif
//                                 d->settings.dl,
//                                 d->settings.da,
//                                 d->settings.gaussPrec,
//                                 d->settings.interp,
//                                 d->settings.fastApprox,
//                                 d->settings.tile,
//                                 d->settings.btile,
//                                 d->computationThreads);
//
//         iterationLoop(iter);
//     }
    // TODO: implement the above with g'mic
}

void GreycstorationFilter::simpleResize()
{
    //FIXME: code doesnt work
    const unsigned int method = 3;      // Initial estimate (0, none, 1=block, 3=linear, 4=grid, 5=bicubic).
    int w                     = m_destImage.width();
    int h                     = m_destImage.height();

    while (d->img.width() > 2 * w && d->img.height() > 2 * h)
    {
        d->img.resize_halfXY();
    }

    d->img.resize(w, h, -100, -100, method);
}

void GreycstorationFilter::iterationLoop()
{
    uint p   = 0;
    if(d->thread-isRunning())
    {
        float iter = d->gmicInterface->getProgress();
        kDebug() << "Progress " << iter;
        p = (uint)(iter * 100);
        postProgress(p);
    }
}
//
//     while (d->threadManager->isRunning())
//     {
//         if (!runningFlag())
//         {
//             d->threadManager->stop();
//             d->threadManager->wait();
//         }
//         else
//         {
//             float progress = d->threadManager->waitABit(50);
//
//             // Update the progress bar in dialog. We simply compute the global
//             // progression index (including all iterations).
//             p = (uint)((iter * 100 + progress) / d->settings.nbIter);
//
//             if (p > mp)
//             {
//                 postProgress(p);
//                 mp = p;
//             }
//         }
//     }
// }

FilterAction GreycstorationFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter("alpha",        d->settings.alpha);
    action.addParameter("amplitude",    d->settings.amplitude);
    action.addParameter("anisotropy",   d->settings.anisotropy);
    action.addParameter("btile",        d->settings.btile);
    action.addParameter("da",           d->settings.da);
    action.addParameter("dl",           d->settings.dl);
    action.addParameter("fastApprox",   d->settings.fastApprox);
    action.addParameter("gaussPrec",    d->settings.gaussPrec);
    action.addParameter("interp",       d->settings.interp);
    action.addParameter("nbIter",       d->settings.nbIter);
    action.addParameter("sharpness",    d->settings.sharpness);
    action.addParameter("sigma",        d->settings.sigma);
    action.addParameter("tile",         d->settings.tile);

    return action;
}

void GreycstorationFilter::readParameters(const FilterAction& action)
{
    d->settings.alpha       = action.parameter("alpha").toFloat();
    d->settings.amplitude   = action.parameter("amplitude").toFloat();
    d->settings.anisotropy  = action.parameter("anisotropy").toFloat();
    d->settings.btile       = action.parameter("btile").toInt();
    d->settings.da          = action.parameter("da").toFloat();
    d->settings.dl          = action.parameter("dl").toFloat();
    d->settings.fastApprox  = action.parameter("fastApprox").toBool();
    d->settings.gaussPrec   = action.parameter("gaussPrec").toFloat();
    d->settings.interp      = action.parameter("interp").toFloat();
    d->settings.nbIter      = action.parameter("nbIter").toUInt();
    d->settings.sharpness   = action.parameter("sharpness").toFloat();
    d->settings.sigma       = action.parameter("sigma").toFloat();
    d->settings.tile        = action.parameter("tile").toInt();
}

}  // namespace Digikam
