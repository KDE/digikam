/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-13-08
 * Description : Raw post processing corrections.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rawpostprocessing.h"

// Local includes

#include "digikam_debug.h"
#include "imagehistogram.h"
#include "imagelevels.h"
#include "wbfilter.h"
#include "curvesfilter.h"
#include "bcgfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

RawPostProcessing::RawPostProcessing(DImg* const orgImage, QObject* const parent, const DRawDecoding& settings)
    : DImgThreadedFilter(parent)
{
    setFilterName("RawPostProcessing");
    setOriginalImage(orgImage->copy());
    m_customRawSettings = settings;
    initFilter();
}

RawPostProcessing::RawPostProcessing(DImgThreadedFilter* const parentFilter,
                                     const DImg& orgImage, const DImg& destImage,
                                     int progressBegin, int progressEnd, const DRawDecoding& settings)
    : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                         parentFilter->filterName() + ": RawPostProcessing")
{
    m_customRawSettings = settings;
    filterImage();
}

RawPostProcessing::~RawPostProcessing()
{
    cancelFilter();
}

void RawPostProcessing::filterImage()
{
    rawPostProcessing();
}

void RawPostProcessing::rawPostProcessing()
{
    if (!m_orgImage.bits() || !m_orgImage.width() || !m_orgImage.height())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << ("RawPostProcessing::rawPostProcessing: no image m_orgImage.bits() available!");
        return;
    }

    if (!m_customRawSettings.postProcessingSettingsIsDirty())
    {
        m_destImage = m_orgImage;
        return;
    }

    postProgress(20);

    if (m_customRawSettings.exposureComp != 0.0 || m_customRawSettings.saturation != 1.0)
    {
        WBContainer settings;
        settings.temperature  = 6500.0;
        settings.dark         = 0.5;
        settings.black        = 0.0;
        settings.exposition   = m_customRawSettings.exposureComp;
        settings.gamma        = 1.0;
        settings.saturation   = m_customRawSettings.saturation;
        settings.green        = 1.0;
        WBFilter wb(&m_orgImage, 0L, settings);
        wb.startFilterDirectly();
        m_orgImage.putImageData(wb.getTargetImage().bits());
    }

    postProgress(40);

    if (m_customRawSettings.lightness != 0.0 || m_customRawSettings.contrast != 1.0 || m_customRawSettings.gamma != 1.0)
    {
        BCGContainer settings;
        settings.brightness = m_customRawSettings.lightness;
        settings.contrast   = m_customRawSettings.contrast;
        settings.gamma      = m_customRawSettings.gamma;
        BCGFilter bcg(&m_orgImage, 0L, settings);
        bcg.startFilterDirectly();
        m_orgImage.putImageData(bcg.getTargetImage().bits());
    }

    postProgress(60);

    if (!m_customRawSettings.curveAdjust.isEmpty())
    {
        CurvesContainer prm(ImageCurves::CURVE_SMOOTH, m_orgImage.sixteenBit());
        prm.values[LuminosityChannel] = m_customRawSettings.curveAdjust;
        CurvesFilter curves(&m_orgImage, 0L, prm);
        curves.startFilterDirectly();
        m_orgImage.putImageData(curves.getTargetImage().bits());
    }

    postProgress(80);

    m_destImage = m_orgImage;

    postProgress(100);
}

} // namespace Digikam
