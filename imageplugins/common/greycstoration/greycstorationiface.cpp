/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-12-03
 * Description : Greycstoration interface.
 * 
 * Copyright 2007 by Gilles Caulier
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

 // C Ansi includes

extern "C"
{
#include <unistd.h>
}
  
// C++ includes. 
 
#include <cstdio>
#include <cmath>
#include <cstring>

// Qt includes.

#include <qimage.h>
#include <qfile.h>

// KDE includes.

#include <kstandarddirs.h>

// Local includes.
 
#define cimg_plugin "greycstoration.h"
#if cimg_OS!=2
#include <pthread.h>
#endif
#include "CImg.h"
using namespace cimg_library;

#include "greycstorationsettings.h"
#include "greycstorationiface.h"

namespace DigikamImagePlugins
{

class GreycstorationIfacePriv
{
    
public:

    GreycstorationIfacePriv()
    {
        restore = true;
        inpaint = false;
        resize = false;
    }

    bool                   restore;
    bool                   inpaint;
    bool                   resize;

    // Inpainting temp mask file path.
    QString                tmpMaskFile;
    
    // Inpainting temp mask data.
    QImage                 inPaintingMask;

    GreycstorationSettings settings;

    CImg<>                 img;
};

GreycstorationIface::GreycstorationIface(Digikam::DImg *orgImage,
                                         GreycstorationSettings settings, 
                                         bool restoreMode, bool inpaintMode, bool resizeMode, 
                                         int newWidth, int newHeight,
                                         QImage *inPaintingMask, QObject *parent)
                   : Digikam::DImgThreadedFilter(orgImage, parent)
{ 
    d = new GreycstorationIfacePriv;
    d->settings = settings;
    d->restore  = restoreMode;
    d->inpaint  = inpaintMode;
    d->resize   = resizeMode;
    
    if (d->resize)
    {
        m_destImage = Digikam::DImg(newWidth, newHeight, m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
        DDebug() << "GreycstorationIface::m_resize is on, new size: (" 
                 << newWidth << ", " << newHeight << ")" << endl;
    }
    else 
    {
        m_destImage = Digikam::DImg(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
    }
    
    d->tmpMaskFile = QString();
    
    if (d->inpaint && inPaintingMask)
    {
       KStandardDirs dir;
       d->tmpMaskFile = dir.saveLocation("tmp");
       d->tmpMaskFile.append(QString::number(getpid()));
       d->tmpMaskFile.append(".png");
       d->inPaintingMask = inPaintingMask->copy();
       d->inPaintingMask.save(d->tmpMaskFile, "PNG");
       DDebug() << "GreycstorationIface::InPainting Mask : " << d->tmpMaskFile << endl;
    }
    
    initFilter();       
}

GreycstorationIface::~GreycstorationIface()
{ 
    if (d->tmpMaskFile != QString())
    {
       // Remove temporary inpainting mask.
       QFile maskFile(d->tmpMaskFile);
       maskFile.remove();
    }

    delete d;
}

// We need to re-implemente this method from DImgThreadedFilter class because
// target image size can be different from original if m_resize is enable.

void GreycstorationIface::initFilter()
{
    if (m_orgImage.width() && m_orgImage.height())
    {
       if (m_parent)
          start();             // m_parent is valide, start thread ==> run()
       else
          startComputation();  // no parent : no using thread.
    }
    else  // No image data 
    {
       if (m_parent)           // If parent then send event about a problem.
       {
          postProgress(0, false, false);
          DDebug() << m_name << "::No valid image data !!! ..." << endl;
       }
    }
}

void GreycstorationIface::cleanupFilter()
{
    if (d->img.greycstoration_is_running())
        d->img.greycstoration_stop();
}

void GreycstorationIface::filterImage()
{
    DDebug() << "GreycstorationIface::Initialization..." << endl;
                   
    // Copy the src data into a CImg type image with three channels and no alpha. 
    // This means that a CImg is always RGBA.

    uchar* imageData = m_orgImage.bits();
    int imageWidth   = m_orgImage.width();
    int imageHeight  = m_orgImage.height();
    
    d->img = CImg<>(imageWidth, imageHeight, 1, 3);
    register int x, y;

    if (!m_orgImage.sixteenBit())           // 8 bits image.
    {
        uchar *ptr = imageData;
        
        for (y = 0; y < imageHeight; y++)
        {
            for (x = 0; x < imageWidth; x++) 
            {
                d->img(x, y, 0) = ptr[0];        // blue.
                d->img(x, y, 1) = ptr[1];        // green.
                d->img(x, y, 2) = ptr[2];        // red.
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
                d->img(x, y, 0) = ptr[0];        // blue.
                d->img(x, y, 1) = ptr[1];        // green.
                d->img(x, y, 2) = ptr[2];        // red.
                ptr += 4;
            }
        }
    }
    
    DDebug() << "GreycstorationIface::Process Computation..." << endl;

    //FIXME : add inpainting and resize mode.              

    restoration(); 
    
    // Copy CImg onto destination.
    
    DDebug() << "GreycstorationIface::Finalization..." << endl;

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
                ptr[0] = (uchar) d->img(x, y, 0);        // Blue
                ptr[1] = (uchar) d->img(x, y, 1);        // Green
                ptr[2] = (uchar) d->img(x, y, 2);        // Red
                ptr += 4;
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
                ptr[0] = (unsigned short) d->img(x, y, 0);        // Blue
                ptr[1] = (unsigned short) d->img(x, y, 1);        // Green
                ptr[2] = (unsigned short) d->img(x, y, 2);        // Red
                ptr += 4;
            }
       }
    }
}

void GreycstorationIface::restoration()
{
    for (unsigned int iter=0 ; iter < d->settings.nbIter ; iter++) 
    {
        // This function will start a thread running one iteration of the GREYCstoration filter.
        // It returns immediately, so you can do what you want after (update a progress bar for instance).
        d->img.greycstoration_run(d->settings.amplitude, 
                                  d->settings.sharpness, 
                                  d->settings.anisotropy, 
                                  d->settings.alpha, 
                                  d->settings.sigma, 
                                  d->settings.dl, 
                                  d->settings.da, 
                                  d->settings.gaussPrec, 
                                  d->settings.interp, 
                                  d->settings.fastApprox, 
                                  d->settings.tile, 
                                  d->settings.btile);
    
        // Here, we print the overall progress percentage.
        do 
        {
            // pr_iteration is the progress percentage for the current iteration
            const float pr_iteration = d->img.greycstoration_progress();
        
            // This simply computes the global progression indice (including all iterations)
            const unsigned int pr_global = (unsigned int)((iter*100 + pr_iteration)/d->settings.nbIter);
        
            postProgress( pr_global );   
      
            // Wait a little bit
//            cimg::wait(100);
        } 
        while (d->img.greycstoration_is_running());
    }
}

void GreycstorationIface::inpainting()
{
}

void GreycstorationIface::resize()
{
}

}  // NameSpace DigikamImagePlugins
