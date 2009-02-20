/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-03
 * Description : Greycstoration interface.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "greycstorationiface.h"

// C++ includes.

#include <cassert>

// KDE includes.

#include <kdebug.h>

// Local includes.

#define cimg_plugin "greycstoration.h"
// Unix-like (Linux, Solaris, BSD, MacOSX, Irix,...).
#if defined(unix)       || defined(__unix)      || defined(__unix__) \
 || defined(linux)      || defined(__linux)     || defined(__linux__) \
 || defined(sun)        || defined(__sun) \
 || defined(BSD)        || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__) || defined __DragonFly__ \
 || defined(__MACOSX__) || defined(__APPLE__) \
 || defined(sgi)        || defined(__sgi) \
 || defined(__CYGWIN__)
#include <pthread.h>
#endif

/** Uncomment this line if you use future GreycStoration implementation with GFact parameter */
#define GREYSTORATION_USING_GFACT 1

// KDE includes.

#include <solid/device.h>

// Local includes.

#include "greycstorationsettings.h"

// CImg includes.

#include "CImg.h"

using namespace cimg_library;

namespace Digikam
{

class GreycstorationIfacePriv : public DSharedData
{

public:

    GreycstorationIfacePriv()
    {
        mode               = GreycstorationIface::Restore;
        gfact              = 1.0;
        computationThreads = 2;
    }

    float                  gfact;

    int                    computationThreads;  // Number of threads used by CImg during computation.
    int                    mode;                // The interface running mode.

    QSize                  newSize;
    QImage                 inPaintingMask;      // Mask for inpainting.

    GreycstorationSettings settings;            // Current Greycstoraion algorithm settings.

    CImg<>                 img;                 // Main image.
    CImg<uchar>            mask;                // The mask used with inpaint or resize mode
};

GreycstorationIface::GreycstorationIface(QObject *parent)
                   : DImgThreadedFilter(parent),
                     m_priv(new GreycstorationIfacePriv)
{
    setOriginalImage(DImg());
    setSettings(GreycstorationSettings());
    setMode(Restore);
    setInPaintingMask(QImage());
}

GreycstorationIface::GreycstorationIface(DImg *orgImage,
                                         const GreycstorationSettings& settings,
                                         int mode,
                                         int newWidth, int newHeight,
                                         const QImage& inPaintingMask,
                                         QObject *parent)
                   : DImgThreadedFilter(parent),
                     m_priv(new GreycstorationIfacePriv)
{
    setOriginalImage(orgImage->copyImageData());
    setSettings(settings);
    setMode(mode, newWidth, newHeight);
    setInPaintingMask(inPaintingMask);
    setup();
}

GreycstorationIface::~GreycstorationIface()
{
}

GreycstorationIface& GreycstorationIface::operator=(const GreycstorationIface& iface)
{
    m_priv = iface.m_priv;
    return *this;
}

void GreycstorationIface::setSettings(const GreycstorationSettings& settings)
{
    m_priv->settings = settings;
}

void GreycstorationIface::setMode(int mode, int newWidth, int newHeight)
{
    m_priv->mode = mode;
    m_priv->newSize = QSize(newWidth, newHeight);
}

void GreycstorationIface::setInPaintingMask(const QImage& inPaintingMask)
{
    m_priv->inPaintingMask = inPaintingMask;
}

void GreycstorationIface::computeChildrenThreads()
{
    // Check number of CPU with Solid interface.

    const int numProcs    = qMax(Solid::Device::listFromType(Solid::DeviceInterface::Processor).count(), 1);
    const int maxThreads  = 16;
    m_priv->computationThreads = qMin(maxThreads, 2 + ((numProcs - 1) * 2));
    kDebug(50003) << "GreycstorationIface::Computation threads: " << m_priv->computationThreads << endl;
}

void GreycstorationIface::setup()
{
    computeChildrenThreads();

    if (m_orgImage.sixteenBit())   // 16 bits image.
        m_priv->gfact = 1.0/256.0;

    if (m_priv->mode == Resize || m_priv->mode == SimpleResize)
    {
        m_destImage = Digikam::DImg(m_priv->newSize.width(), m_priv->newSize.height(),
                                    m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
        kDebug(50003) << "GreycstorationIface::Resize: new size: ("
                      << m_priv->newSize.width() << ", " << m_priv->newSize.height() << ")" << endl;
    }
    else
    {
        m_destImage = Digikam::DImg(m_orgImage.width(), m_orgImage.height(),
                                    m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
    }

    initFilter();
}

QString GreycstorationIface::cimgVersionString()
{
    return QString::number(cimg_version);
}

// We need to re-implement this method from DImgThreadedFilter class because
// target image size can be different from original if m_priv->mode = Resize.

void GreycstorationIface::initFilter()
{
    // (left out here: creation of m_destImage)

    if (m_master)
        startFilterDirectly();
}

void GreycstorationIface::cancelFilter()
{
    // Because Greycstoration algorithm run in a child thread, we need
    // to stop it before to stop this thread.
    if (m_priv->img.greycstoration_is_running())
    {
        // If the user abort, we stop the algorithm.
        kDebug(50003) << "Stop Greycstoration computation..." << endl;
        m_priv->img.greycstoration_stop();
    }

    // And now when stop main loop and clean up all
    Digikam::DImgThreadedFilter::cancelFilter();
}

void GreycstorationIface::filterImage()
{
    register int x, y;

    kDebug(50003) << "GreycstorationIface::Initialization..." << endl;

    // Copy the src image data into a CImg type image with three channels and no alpha.

    uchar* imageData = m_orgImage.bits();
    int imageWidth   = m_orgImage.width();
    int imageHeight  = m_orgImage.height();
    m_priv->img           = CImg<>(imageWidth, imageHeight, 1, 4);

    if (!m_orgImage.sixteenBit())           // 8 bits image.
    {
        uchar *ptr = imageData;

        for (y = 0; y < imageHeight; y++)
        {
            for (x = 0; x < imageWidth; x++)
            {
                m_priv->img(x, y, 0) = ptr[0];        // Blue.
                m_priv->img(x, y, 1) = ptr[1];        // Green.
                m_priv->img(x, y, 2) = ptr[2];        // Red.
                m_priv->img(x, y, 3) = ptr[3];        // Alpha.
                ptr += 4;
            }
        }
    }
    else                                // 16 bits image.
    {
        unsigned short *ptr = (unsigned short *)imageData;

        for (y = 0; y < imageHeight; y++)
        {
            for (x = 0; x < imageWidth; x++)
            {
                m_priv->img(x, y, 0) = ptr[0];        // Blue.
                m_priv->img(x, y, 1) = ptr[1];        // Green.
                m_priv->img(x, y, 2) = ptr[2];        // Red.
                m_priv->img(x, y, 3) = ptr[3];        // Alpha.
                ptr += 4;
            }
        }
    }

    kDebug(50003) << "GreycstorationIface::Process Computation..." << endl;

    try
    {
        switch (m_priv->mode)
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
    }
    catch(...)         // Everything went wrong.
    {
       kDebug(50003) << "GreycstorationIface::Error during Greycstoration filter computation!" << endl;

       if (m_parent)
           emit finished(false);

       return;
    }

    if (m_cancel)
        return;

    // Copy CImg onto destination.

    kDebug(50003) << "GreycstorationIface::Finalization..." << endl;

    uchar* newData = m_destImage.bits();
    int newWidth   = m_destImage.width();
    int newHeight  = m_destImage.height();

    if (!m_orgImage.sixteenBit())           // 8 bits image.
    {
        uchar *ptr = newData;

        for (y = 0; y < newHeight; y++)
        {
            for (x = 0; x < newWidth; x++)
            {
                // Overwrite RGB values to destination.
                ptr[0] = static_cast<uchar>(m_priv->img(x, y, 0));        // Blue
                ptr[1] = static_cast<uchar>(m_priv->img(x, y, 1));        // Green
                ptr[2] = static_cast<uchar>(m_priv->img(x, y, 2));        // Red
                ptr[3] = static_cast<uchar>(m_priv->img(x, y, 3));        // Alpha
                ptr    += 4;
            }
        }
    }
    else                                     // 16 bits image.
    {
        unsigned short *ptr = (unsigned short *)newData;

        for (y = 0; y < newHeight; y++)
        {
            for (x = 0; x < newWidth; x++)
            {
                // Overwrite RGB values to destination.
                ptr[0] = static_cast<unsigned short>(m_priv->img(x, y, 0));        // Blue
                ptr[1] = static_cast<unsigned short>(m_priv->img(x, y, 1));        // Green
                ptr[2] = static_cast<unsigned short>(m_priv->img(x, y, 2));        // Red
                ptr[3] = static_cast<unsigned short>(m_priv->img(x, y, 3));        // Alpha
                ptr    += 4;
            }
        }
    }
}

void GreycstorationIface::restoration()
{
    for (uint iter = 0 ; !m_cancel && (iter < m_priv->settings.nbIter) ; iter++)
    {
        // This function will start a thread running one iteration of the GREYCstoration filter.
        // It returns immediately, so you can do what you want after (update a progress bar for
        // instance).
        m_priv->img.greycstoration_run(m_priv->settings.amplitude,
                                  m_priv->settings.sharpness,
                                  m_priv->settings.anisotropy,
                                  m_priv->settings.alpha,
                                  m_priv->settings.sigma,
#ifdef GREYSTORATION_USING_GFACT
                                  m_priv->gfact,
#endif
                                  m_priv->settings.dl,
                                  m_priv->settings.da,
                                  m_priv->settings.gaussPrec,
                                  m_priv->settings.interp,
                                  m_priv->settings.fastApprox,
                                  m_priv->settings.tile,
                                  m_priv->settings.btile,
                                  m_priv->computationThreads);

        iterationLoop(iter);
    }
}

void GreycstorationIface::inpainting()
{
    if (!m_priv->inPaintingMask.isNull())
    {
        // Copy the inpainting image data into a CImg type image with three channels and no alpha.

        register int x, y;

        m_priv->mask    = CImg<uchar>(m_priv->inPaintingMask.width(), m_priv->inPaintingMask.height(), 1, 3);
        uchar *ptr = m_priv->inPaintingMask.bits();

        for (y = 0; y < m_priv->inPaintingMask.height(); y++)
        {
            for (x = 0; x < m_priv->inPaintingMask.width(); x++)
            {
                m_priv->mask(x, y, 0) = ptr[2];        // blue.
                m_priv->mask(x, y, 1) = ptr[1];        // green.
                m_priv->mask(x, y, 2) = ptr[0];        // red.
                ptr += 4;
            }
        }
    }
    else
    {
        kDebug(50003) << "Inpainting image: mask is null!" << endl;
        m_cancel = true;
        return;
    }

    for (uint iter=0 ; !m_cancel && (iter < m_priv->settings.nbIter) ; iter++)
    {
        // This function will start a thread running one iteration of the GREYCstoration filter.
        // It returns immediately, so you can do what you want after (update a progress bar for
        // instance).
        m_priv->img.greycstoration_run(m_priv->mask,
                                  m_priv->settings.amplitude,
                                  m_priv->settings.sharpness,
                                  m_priv->settings.anisotropy,
                                  m_priv->settings.alpha,
                                  m_priv->settings.sigma,
#ifdef GREYSTORATION_USING_GFACT
                                  m_priv->gfact,
#endif
                                  m_priv->settings.dl,
                                  m_priv->settings.da,
                                  m_priv->settings.gaussPrec,
                                  m_priv->settings.interp,
                                  m_priv->settings.fastApprox,
                                  m_priv->settings.tile,
                                  m_priv->settings.btile,
                                  m_priv->computationThreads);

        iterationLoop(iter);
    }
}

void GreycstorationIface::resize()
{
    const bool anchor       = true;   // Anchor original pixels.
    const unsigned int init = 5;      // Initial estimate (1=block, 3=linear, 5=bicubic).

    int w = m_destImage.width();
    int h = m_destImage.height();

    m_priv->mask.assign(m_priv->img.dimx(), m_priv->img.dimy(), 1, 1, 255);

    if (!anchor)
        m_priv->mask.resize(w, h, 1, 1, 1);
    else
        m_priv->mask = !m_priv->mask.resize(w, h, 1, 1, 4);

    m_priv->img.resize(w, h, 1, -100, init);

    for (uint iter = 0 ; !m_cancel && (iter < m_priv->settings.nbIter) ; iter++)
    {
        // This function will start a thread running one iteration of the GREYCstoration filter.
        // It returns immediately, so you can do what you want after (update a progress bar for
        // instance).
        m_priv->img.greycstoration_run(m_priv->mask,
                                  m_priv->settings.amplitude,
                                  m_priv->settings.sharpness,
                                  m_priv->settings.anisotropy,
                                  m_priv->settings.alpha,
                                  m_priv->settings.sigma,
#ifdef GREYSTORATION_USING_GFACT
                                  m_priv->gfact,
#endif
                                  m_priv->settings.dl,
                                  m_priv->settings.da,
                                  m_priv->settings.gaussPrec,
                                  m_priv->settings.interp,
                                  m_priv->settings.fastApprox,
                                  m_priv->settings.tile,
                                  m_priv->settings.btile,
                                  m_priv->computationThreads);

        iterationLoop(iter);
    }
}

void GreycstorationIface::simpleResize()
{
    const unsigned int method = 3;      // Initial estimate (0, none, 1=block, 3=linear, 4=grid, 5=bicubic).

    int w = m_destImage.width();
    int h = m_destImage.height();

    while (m_priv->img.dimx() > 2*w &&
           m_priv->img.dimy() > 2*h)
    {
        m_priv->img.resize_halfXY();
    }

    m_priv->img.resize(w, h, -100, -100, method);
}

void GreycstorationIface::iterationLoop(uint iter)
{
    uint mp  = 0;
    uint p   = 0;

    do
    {
        usleep(100000);

        if (m_parent && !m_cancel)
        {
            // Update the progress bar in dialog. We simply computes the global
            // progression index (including all iterations).

            p = (uint)((iter*100 + m_priv->img.greycstoration_progress())/m_priv->settings.nbIter);

            if (p > mp)
            {
                postProgress(p);
                mp = p;
            }
        }
    }
    while (m_priv->img.greycstoration_is_running() && !m_cancel);

    // A delay is require here. I suspect a sync problem between threads
    // used by GreycStoration algorithm.
    usleep(100000);
}

}  // namespace Digikam
