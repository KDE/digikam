/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-03
 * Description : Greycstoration interface.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


/** Don't use CImg interface (keyboard/mouse interaction) */
#define cimg_display 0
/** Only print debug information on the console */
#define cimg_debug 1

#include "greycstorationfilter.h"

// C++ includes

#include <cassert>

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

// Local includes

#include "digikam_debug.h"
#include "dynamicthread.h"

#define cimg_plugin "cimg/greycstoration.h"

/** Uncomment this line if you use future GreycStoration implementation with GFact parameter
 */
#define GREYSTORATION_USING_GFACT 1

// Pragma directives to reduce warnings from CImg header files.
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wcast-align"
#pragma clang diagnostic ignored "-Wshift-negative-value"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif

// CImg includes

#include "cimg/CImg.h"

// Restore warnings
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic pop
#endif

extern "C"
{
#include <unistd.h>
}

using namespace cimg_library;

namespace Digikam
{

class GreycstorationFilter::Private
{

public:

    Private() :
        gfact(1.0),
        computationThreads(2),
        mode(GreycstorationFilter::Restore),
        threadManager(new CImg<>::GreycstorationThreadManager)
    {
    }

    ~Private()
    {
        delete threadManager;
    }

public:

    float                                gfact;

    int                                  computationThreads;  // Number of threads used by CImg during computation.
    int                                  mode;                // The interface running mode.

    QSize                                newSize;
    QImage                               inPaintingMask;      // Mask for inpainting.

    GreycstorationContainer              settings;            // Current Greycstoraion algorithm settings.

    CImg<>                               img;                 // Main image.
    CImg<uchar>                          mask;                // The mask used with inpaint or resize mode

    CImg<>::GreycstorationThreadManager* threadManager;
};

GreycstorationFilter::GreycstorationFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    setOriginalImage(DImg());
    setSettings(GreycstorationContainer());
    setMode(Restore);
    setInPaintingMask(QImage());
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

void GreycstorationFilter::computeChildrenThreads()
{
    const int numProcs    = qMax(QThread::idealThreadCount(), 1);
    const int maxThreads  = 16;
    d->computationThreads = qMin(maxThreads, 2 + ((numProcs - 1) * 2));
    qCDebug(DIGIKAM_DIMG_LOG) << "GreycstorationFilter::Computation threads: " << d->computationThreads;
}

void GreycstorationFilter::setup()
{
    computeChildrenThreads();

    if (m_orgImage.sixteenBit())   // 16 bits image.
    {
        d->gfact = 1.0 / 256.0;
    }

    if (d->mode == Resize || d->mode == SimpleResize)
    {
        m_destImage = DImg(d->newSize.width(), d->newSize.height(),
                           m_orgImage.sixteenBit(), m_orgImage.hasAlpha());

        qCDebug(DIGIKAM_DIMG_LOG) << "GreycstorationFilter::Resize: new size: ("
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
    return QString::number(cimg_version);
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

void GreycstorationFilter::cancelFilter()
{
    // Because Greycstoration algorithm run in a child thread, we need
    // to stop it before to stop this thread.
    qCDebug(DIGIKAM_DIMG_LOG) << "Stop Greycstoration computation...";
    d->threadManager->stop();

    // And now when stop main loop and clean up all
    DImgThreadedFilter::cancelFilter();
}

void GreycstorationFilter::filterImage()
{
   int x, y;

    qCDebug(DIGIKAM_DIMG_LOG) << "Initialization...";

    uchar* const data = m_orgImage.bits();
    int width         = m_orgImage.width();
    int height        = m_orgImage.height();

    // convert DImg (interleaved RGBA) to CImg (planar RGBA)
    if (!m_orgImage.sixteenBit())           // 8 bits image.
    {
        d->img = CImg<unsigned char>(data, 4, width, height, 1, false).
                 get_permute_axes("yzvx");
    }
    else                                    // 16 bits image.
    {
        d->img = CImg<unsigned short>(reinterpret_cast<unsigned short*>(data), 4, width, height, 1, false).
                 get_permute_axes("yzvx");
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Process Computation...";

    try
    {
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
        d->threadManager->finish();
    }
    catch (...)        // Everything went wrong.
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Error during Greycstoration filter computation!";

        return;
    }

    if (!runningFlag())
    {
        return;
    }

    // Copy CImg onto destination.

    qCDebug(DIGIKAM_DIMG_LOG) << "Finalization...";

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

void GreycstorationFilter::restoration()
{
    for (uint iter = 0 ; runningFlag() && (iter < d->settings.nbIter) ; ++iter)
    {
        // This function will start a thread running one iteration of the GREYCstoration filter.
        // It returns immediately, so you can do what you want after (update a progress bar for
        // instance).
        d->threadManager->start(d->img, d->settings.amplitude,
                                d->settings.sharpness,
                                d->settings.anisotropy,
                                d->settings.alpha,
                                d->settings.sigma,
#ifdef GREYSTORATION_USING_GFACT
                                d->gfact,
#endif
                                d->settings.dl,
                                d->settings.da,
                                d->settings.gaussPrec,
                                d->settings.interp,
                                d->settings.fastApprox,
                                d->settings.tile,
                                d->settings.btile,
                                d->computationThreads);

        iterationLoop(iter);
    }
}

void GreycstorationFilter::inpainting()
{
    if (!d->inPaintingMask.isNull())
    {
        // Copy the inpainting image data into a CImg type image with three channels and no alpha.

       int x, y;

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
        qCDebug(DIGIKAM_DIMG_LOG) << "Inpainting image: mask is null!";
        stop();
        return;
    }

    for (uint iter = 0 ; runningFlag() && (iter < d->settings.nbIter) ; ++iter)
    {
        // This function will start a thread running one iteration of the GREYCstoration filter.
        // It returns immediately, so you can do what you want after (update a progress bar for
        // instance).
        d->threadManager->start(d->img, &d->mask,
                                d->settings.amplitude,
                                d->settings.sharpness,
                                d->settings.anisotropy,
                                d->settings.alpha,
                                d->settings.sigma,
#ifdef GREYSTORATION_USING_GFACT
                                d->gfact,
#endif
                                d->settings.dl,
                                d->settings.da,
                                d->settings.gaussPrec,
                                d->settings.interp,
                                d->settings.fastApprox,
                                d->settings.tile,
                                d->settings.btile,
                                d->computationThreads);

        iterationLoop(iter);
    }
}

void GreycstorationFilter::resize()
{
    const bool anchor       = true;   // Anchor original pixels.
    const unsigned int init = 5;      // Initial estimate (1=block, 3=linear, 5=bicubic).
    int w                   = m_destImage.width();
    int h                   = m_destImage.height();

    d->mask.assign(d->img.dimx(), d->img.dimy(), 1, 1, 255);

    if (!anchor)
    {
        d->mask.resize(w, h, 1, 1, 1);
    }
    else
    {
        d->mask = !d->mask.resize(w, h, 1, 1, 4);
    }

    d->img.resize(w, h, 1, -100, init);

    for (uint iter = 0 ; runningFlag() && (iter < d->settings.nbIter) ; ++iter)
    {
        // This function will start a thread running one iteration of the GREYCstoration filter.
        // It returns immediately, so you can do what you want after (update a progress bar for
        // instance).
        d->threadManager->start(d->img, &d->mask,
                                d->settings.amplitude,
                                d->settings.sharpness,
                                d->settings.anisotropy,
                                d->settings.alpha,
                                d->settings.sigma,
#ifdef GREYSTORATION_USING_GFACT
                                d->gfact,
#endif
                                d->settings.dl,
                                d->settings.da,
                                d->settings.gaussPrec,
                                d->settings.interp,
                                d->settings.fastApprox,
                                d->settings.tile,
                                d->settings.btile,
                                d->computationThreads);

        iterationLoop(iter);
    }
}

void GreycstorationFilter::simpleResize()
{
    const unsigned int method = 3;      // Initial estimate (0, none, 1=block, 3=linear, 4=grid, 5=bicubic).
    int w                     = m_destImage.width();
    int h                     = m_destImage.height();

    while (d->img.dimx() > 2 * w && d->img.dimy() > 2 * h)
    {
        d->img.resize_halfXY();
    }

    d->img.resize(w, h, -100, -100, method);
}

void GreycstorationFilter::iterationLoop(uint iter)
{
    uint mp  = 0;
    uint p   = 0;

    while (d->threadManager->isRunning())
    {
        if (!runningFlag())
        {
            d->threadManager->stop();
            d->threadManager->wait();
        }
        else
        {
            float progress = d->threadManager->waitABit(50);

            // Update the progress bar in dialog. We simply compute the global
            // progression index (including all iterations).
            p = (uint)((iter * 100 + progress) / d->settings.nbIter);

            if (p > mp)
            {
                postProgress(p);
                mp = p;
            }
        }
    }
}

FilterAction GreycstorationFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("alpha"),        d->settings.alpha);
    action.addParameter(QLatin1String("amplitude"),    d->settings.amplitude);
    action.addParameter(QLatin1String("anisotropy"),   d->settings.anisotropy);
    action.addParameter(QLatin1String("btile"),        d->settings.btile);
    action.addParameter(QLatin1String("da"),           d->settings.da);
    action.addParameter(QLatin1String("dl"),           d->settings.dl);
    action.addParameter(QLatin1String("fastApprox"),   d->settings.fastApprox);
    action.addParameter(QLatin1String("gaussPrec"),    d->settings.gaussPrec);
    action.addParameter(QLatin1String("interp"),       d->settings.interp);
    action.addParameter(QLatin1String("nbIter"),       d->settings.nbIter);
    action.addParameter(QLatin1String("sharpness"),    d->settings.sharpness);
    action.addParameter(QLatin1String("sigma"),        d->settings.sigma);
    action.addParameter(QLatin1String("tile"),         d->settings.tile);

    return action;
}

void GreycstorationFilter::readParameters(const FilterAction& action)
{
    d->settings.alpha       = action.parameter(QLatin1String("alpha")).toFloat();
    d->settings.amplitude   = action.parameter(QLatin1String("amplitude")).toFloat();
    d->settings.anisotropy  = action.parameter(QLatin1String("anisotropy")).toFloat();
    d->settings.btile       = action.parameter(QLatin1String("btile")).toInt();
    d->settings.da          = action.parameter(QLatin1String("da")).toFloat();
    d->settings.dl          = action.parameter(QLatin1String("dl")).toFloat();
    d->settings.fastApprox  = action.parameter(QLatin1String("fastApprox")).toBool();
    d->settings.gaussPrec   = action.parameter(QLatin1String("gaussPrec")).toFloat();
    d->settings.interp      = action.parameter(QLatin1String("interp")).toFloat();
    d->settings.nbIter      = action.parameter(QLatin1String("nbIter")).toUInt();
    d->settings.sharpness   = action.parameter(QLatin1String("sharpness")).toFloat();
    d->settings.sigma       = action.parameter(QLatin1String("sigma")).toFloat();
    d->settings.tile        = action.parameter(QLatin1String("tile")).toInt();
}

}  // namespace Digikam
