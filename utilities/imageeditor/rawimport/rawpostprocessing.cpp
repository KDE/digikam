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
#include "imagelevels.h"
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
    rawPostProcessing();
}

void RawPostProcessing::rawPostProcessing()
{
    if (!m_orgImage.bits() || !m_orgImage.width() || !m_orgImage.height())
    {
       DWarning() << ("RawPostProcessing::rawPostProcessing: no image m_orgImage.bits() available!")
                  << endl;
       return;
    }

    if (!m_customRawSettings.postProcessingSettingsIsDirty())
    {
        m_destImage = m_orgImage;
        return;
    }

    postProgress(15);

    if (m_customRawSettings.exposureComp != 0.0 || m_customRawSettings.saturation != 1.0)
    {
        WhiteBalance wb(m_orgImage.sixteenBit());
        wb.whiteBalance(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit(),
                        0.0,                                // black
                        m_customRawSettings.exposureComp,   // exposure
                        6500.0,                             // temperature (neutral)
                        1.0,                                // green
                        0.5,                                // dark
                        1.0,                                // gamma
                        m_customRawSettings.saturation);    // saturation
    }
    postProgress(30);

    if (m_customRawSettings.lightness != 0.0 || m_customRawSettings.contrast != 1.0 || m_customRawSettings.gamma != 1.0)
    {
        BCGModifier bcg;
        bcg.setBrightness(m_customRawSettings.lightness);
        bcg.setContrast(m_customRawSettings.contrast);
        bcg.setGamma(m_customRawSettings.gamma);
        bcg.applyBCG(m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit());
    }
    postProgress(45);

    if (!m_customRawSettings.curveAdjust.isEmpty())
    {
        DImg tmp(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit());
        ImageCurves curves(m_orgImage.sixteenBit());
        curves.setCurvePoints(ImageHistogram::ValueChannel, m_customRawSettings.curveAdjust);
        curves.curvesCalculateCurve(ImageHistogram::ValueChannel);
        curves.curvesLutSetup(ImageHistogram::AlphaChannel);
        curves.curvesLutProcess(m_orgImage.bits(), tmp.bits(), m_orgImage.width(), m_orgImage.height());
        memcpy(m_orgImage.bits(), tmp.bits(), tmp.numBytes());
    }
    postProgress(60);

    if (!m_customRawSettings.levelsAdjust.isEmpty())
    {
        DImg tmp(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit());
        ImageLevels levels(m_orgImage.sixteenBit());
        int j=0;
        for (int i = 0 ; i < 4; i++)
        {
            levels.setLevelLowInputValue(i, m_customRawSettings.levelsAdjust[j++]);
            levels.setLevelHighInputValue(i, m_customRawSettings.levelsAdjust[j++]);
            levels.setLevelLowOutputValue(i, m_customRawSettings.levelsAdjust[j++]);
            levels.setLevelHighOutputValue(i, m_customRawSettings.levelsAdjust[j++]);
        }

        levels.levelsLutSetup(ImageHistogram::AlphaChannel);
        levels.levelsLutProcess(m_orgImage.bits(), tmp.bits(), m_orgImage.width(), m_orgImage.height());
        memcpy(m_orgImage.bits(), tmp.bits(), tmp.numBytes());
    }
    postProgress(75);

    m_destImage = m_orgImage;

    postProgress(100);
}

}  // NameSpace Digikam
