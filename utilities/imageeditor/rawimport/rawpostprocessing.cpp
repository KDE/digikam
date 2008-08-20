/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-13-08
 * Description : Raw post processing corrections.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "ddebug.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "bcgmodifier.h"
#include "whitebalance.h"
#include "dimgimagefilters.h"
#include "rawpostprocessing.h"

namespace Digikam
{

RawPostProcessing::RawPostProcessing(DImg *orgImage, QObject *parent, const DRawDecoding& settings)
                 : DImgThreadedFilter(orgImage, parent, "RawPostProcessing")
{
    m_customRawSettings = settings;
    initFilter();
}

RawPostProcessing::RawPostProcessing(DImgThreadedFilter *parentFilter,
                                     const DImg &orgImage, const DImg &destImage,
                                     int progressBegin, int progressEnd, const DRawDecoding& settings)
                : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                                     parentFilter->filterName() + ": RawPostProcessing")
{
    m_customRawSettings = settings;
    filterImage();
}

void RawPostProcessing::filterImage()
{
    rawPostProcessing(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(),
                      m_orgImage.sixteenBit(), m_customRawSettings);
}

void RawPostProcessing::rawPostProcessing(uchar *data, int width, int height, bool sixteenBit, 
                                          const DRawDecoding& settings)
{
    if (!data || !width || !height)
    {
       DWarning() << ("RawPostProcessing::rawPostProcessing: no image data available!")
                   << endl;
       return;
    }

    if (!settings.postProcessingSettingsIsDirty())
    {
       m_destImage = m_orgImage;
       return;
    }

    if (settings.exposureComp != 0.0 || settings.saturation != 1.0)
    {
        WhiteBalance wb(sixteenBit);
        wb.whiteBalance(data, width, height, sixteenBit,
                        0.0,                     // black
                        settings.exposureComp,   // exposure
                        6500.0,                  // temperature (neutral)
                        1.0,                     // green
                        0.5,                     // dark
                        1.0,                     // gamma
                        settings.saturation);    // saturation
    }

    if (settings.lightness != 0.0 || settings.contrast != 1.0 || settings.gamma != 1.0)
    {
        BCGModifier bcg;
        bcg.setBrightness(settings.lightness);
        bcg.setContrast(settings.contrast);
        bcg.setGamma(settings.gamma);
        bcg.applyBCG(data, width, height, sixteenBit);
    }

    if (!settings.curveAdjust.isEmpty())
    {
        DImg tmp(width, height, sixteenBit);
        ImageCurves curves(sixteenBit);
        curves.setCurvePoints(ImageHistogram::ValueChannel, settings.curveAdjust);
        curves.curvesCalculateCurve(ImageHistogram::ValueChannel);
        curves.curvesLutSetup(ImageHistogram::AlphaChannel);
        curves.curvesLutProcess(data, tmp.bits(), width, height);

        memcpy(data, tmp.bits(), tmp.numBytes());
    }

    m_destImage = m_orgImage;
}

}  // NameSpace Digikam
