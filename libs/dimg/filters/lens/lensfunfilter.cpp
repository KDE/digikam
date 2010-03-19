/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "lensfunfilter.h"

// Qt includes

#include <QByteArray>
#include <QCheckBox>
#include <QString>

// KDE includes

#include <kdebug.h>

// Local includes

#include "lensfuniface.h"

namespace Digikam
{

class LensFunFilterPriv
{
public:

    LensFunFilterPriv()
    {
        iface    = 0;
        modifier = 0;
    }

    LensFunIface* iface;

    lfModifier*   modifier;
};

LensFunFilter::LensFunFilter(DImg* orgImage, QObject* parent, LensFunIface* iface,
                             const LensFunContainer& settings)
             : DImgThreadedFilter(orgImage, parent, "LensCorrection"),
               d(new LensFunFilterPriv)
{
    d->iface = iface;
    d->iface->setSettings(settings);

    initFilter();
}

LensFunFilter::~LensFunFilter()
{
    delete d;
}

void LensFunFilter::filterImage()
{
#if 0
    if (!opts.Crop)
        opts.Crop = lens->CropFactor;
    if (!opts.Focal)
        opts.Focal = lens->MinFocal;
    if (!opts.Aperture)
        opts.Aperture = lens->MinAperture;
#endif

    int modifyFlags = 0;
    if ( d->iface->m_settings.filterDist )
       modifyFlags |= LF_MODIFY_DISTORTION;
    if ( d->iface->m_settings.filterGeom )
       modifyFlags |= LF_MODIFY_GEOMETRY;
    if ( d->iface->m_settings.filterCCA )
       modifyFlags |= LF_MODIFY_TCA;
    if ( d->iface->m_settings.filterVig )
       modifyFlags |= LF_MODIFY_VIGNETTING;
    if ( d->iface->m_settings.filterCCI )
       modifyFlags |= LF_MODIFY_CCI;

    // Init lensfun lib, we are working on the full image.

    lfPixelFormat colorDepth = m_orgImage.bytesDepth() == 4 ? LF_PF_U8 : LF_PF_U16;

    d->modifier = lfModifier::Create(d->iface->m_usedLens,
                                     d->iface->m_cropFactor,
                                     m_orgImage.width(),
                                     m_orgImage.height());

    int modflags = d->modifier->Initialize(d->iface->m_usedLens,
                                           colorDepth,
                                           d->iface->m_focalLength,
                                           d->iface->m_aperture,
                                           d->iface->m_subjectDistance,
                                           d->iface->m_cropFactor,
                                           LF_RECTILINEAR,
                                           modifyFlags,
                                           0/*no inverse*/);

    if (!d->modifier)
    {
        kError() << "ERROR: cannot initialize LensFun Modifier.";
        return;
    }

    // Calc necessary steps for progress bar

    int steps = d->iface->m_settings.filterCCA                                         ? 1 : 0 +
                ( d->iface->m_settings.filterVig  || d->iface->m_settings.filterCCI )  ? 1 : 0 +
                ( d->iface->m_settings.filterDist || d->iface->m_settings.filterGeom ) ? 1 : 0;

    kDebug() << "LensFun Modifier Flags: " << modflags << "  Steps:" << steps;

    if ( steps < 1 )
       return;

    // The real correction to do

    int loop   = 0;
    int lwidth = m_orgImage.width() * 2 * 3;
    float* pos = new float[lwidth];

    // Stage 1: TCA correction

    if ( d->iface->m_settings.filterCCA )
    {
        m_orgImage.prepareSubPixelAccess(); // init lanczos kernel

        for (unsigned int y=0; !m_cancel && (y < m_orgImage.height()); ++y)
        {
            if (d->modifier->ApplySubpixelDistortion(0.0, y, m_orgImage.width(), 1, pos))
            {
                float* src = pos;

                for (unsigned x = 0; !m_cancel && (x < m_destImage.width()); ++x)
                {
                    DColor destPixel(0, 0, 0, 0xFFFF, m_destImage.sixteenBit());

                    destPixel.setRed  (m_orgImage.getSubPixelColorFast(src[0], src[1]).red()   );
                    destPixel.setGreen(m_orgImage.getSubPixelColorFast(src[2], src[3]).green() );
                    destPixel.setBlue (m_orgImage.getSubPixelColorFast(src[4], src[5]).blue()  );

                    m_destImage.setPixelColor(x, y, destPixel);
                    src += 2 * 3;
                }
                ++loop;
            }

            // Update progress bar in dialog.
            int progress = (int)(((double)y * 100.0) / m_orgImage.height());
            if (m_parent && progress%5 == 0)
                postProgress(progress/steps);
        }

        kDebug() << "Applying TCA correction... (loop: " << loop << ")";
    }
    else
    {
        m_destImage.bitBltImage(&m_orgImage, 0, 0);
    }

    // Stage 2: Color Correction: Vignetting and CCI

    uchar* data = m_destImage.bits();

    if ( d->iface->m_settings.filterVig || d->iface->m_settings.filterCCI )
    {
        loop         = 0;
        float offset = 0.0;

        if ( steps == 3 )
            offset = 33.3;
        else if (steps == 2 && d->iface->m_settings.filterCCA)
            offset = 50.0;

        for (unsigned int y=0; !m_cancel && (y < m_destImage.height()); ++y)
        {
            if (d->modifier->ApplyColorModification(data, 0.0, y, m_destImage.width(),
                                                    1, m_destImage.bytesDepth(), 0))
            {
                data += m_destImage.height() * m_destImage.bytesDepth();
                ++loop;
            }

            // Update progress bar in dialog.
            int progress = (int)(((double)y * 100.0) / m_destImage.height());
            if (m_parent && progress%5 == 0)
                postProgress(progress/steps + offset);
        }

        kDebug() << "Applying Color Correction: Vignetting and CCI. (loop: " << loop << ")";
    }

    // Stage 3: Distortion and Geometry

    if ( d->iface->m_settings.filterDist || d->iface->m_settings.filterGeom )
    {
        loop = 0;

        // we need a deep copy first
        DImg tempImage(m_destImage.width(), m_destImage.height(), m_destImage.sixteenBit(), m_destImage.hasAlpha());
        m_destImage.prepareSubPixelAccess(); // init lanczos kernel

        for (unsigned long y=0; !m_cancel && (y < tempImage.height()); ++y)
        {
            if (d->modifier->ApplyGeometryDistortion(0.0, y, tempImage.width(), 1, pos))
            {
                float* src = pos;

                for (unsigned long x = 0; !m_cancel && (x < tempImage.width()); ++x, ++loop)
                {
                    //qDebug (" ZZ %f %f %i %i", src[0], src[1], (int)src[0], (int)src[1]);

                    tempImage.setPixelColor(x, y, m_destImage.getSubPixelColor(src[0], src[1]));
                    src += 2;
                }
            }

            // Update progress bar in dialog.
            int progress = (int)(((double)y * 100.0) / tempImage.height());
            if (m_parent && progress%5 == 0)
                postProgress(progress/steps + 33.3*(steps-1));
        }

        /*qDebug (" for %f %f %i %i", tempImage.height(), tempImage.width(),
                                      tempImage.height(), tempImage.width());*/
        kDebug() << "Applying Distortion and Geometry Correction. (loop: " << loop << ")";

        m_destImage = tempImage;
    }

    // clean up

    delete [] pos;
    d->modifier->Destroy();
}

}  // namespace Digikam
