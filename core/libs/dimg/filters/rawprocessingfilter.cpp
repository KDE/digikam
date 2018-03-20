/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-14
 * Description : Filter to manage and help with raw loading
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "rawprocessingfilter.h"

// DRawDecoder includes

#include "drawdecoder.h"

// Local includes

#include "bcgfilter.h"
#include "curvesfilter.h"
#include "digikam_export.h"
#include "dimgloaderobserver.h"
#include "filteraction.h"
#include "icctransform.h"
#include "wbfilter.h"

namespace Digikam
{

RawProcessingFilter::RawProcessingFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      m_observer(0)
{
}

RawProcessingFilter::RawProcessingFilter(DImg* const orgImage, QObject* const parent,
                                         const DRawDecoding& settings, const QString& name)
    : DImgThreadedFilter(orgImage, parent, name),
      m_observer(0)
{
    setSettings(settings);
    initFilter();
}

RawProcessingFilter::RawProcessingFilter(const DRawDecoding& settings,
                                         DImgThreadedFilter* const master, const DImg& orgImage, const DImg& destImage,
                                         int progressBegin, int progressEnd, const QString& name)
    : DImgThreadedFilter(master, orgImage, destImage, progressBegin, progressEnd, name),
      m_observer(0)
{
    setSettings(settings);
    filterImage();
}

RawProcessingFilter::~RawProcessingFilter()
{
    cancelFilter();
}

void RawProcessingFilter::setSettings(const DRawDecoding& settings)
{
    m_settings = settings;
}

DRawDecoding RawProcessingFilter::settings() const
{
    return m_settings;
}

void RawProcessingFilter::setOutputProfile(const IccProfile& profile)
{
    m_customOutputProfile = profile;
}

void RawProcessingFilter::setObserver(DImgLoaderObserver* const observer, int progressBegin, int progressEnd)
{
    initSlave(0, progressBegin, progressEnd);
    m_observer = observer;
}

FilterAction RawProcessingFilter::filterAction()
{
    DefaultFilterAction<RawProcessingFilter> action;
    m_settings.writeToFilterAction(action);
    return action;
}

void RawProcessingFilter::readParameters(const FilterAction& action)
{
    m_settings = DRawDecoding::fromFilterAction(action);
}

void RawProcessingFilter::postProgress(int)
{
    DImgThreadedFilter::postProgress(20);

    if (m_observer)
    {
        m_observer->progressInfo(&m_destImage, float(modulateProgress(20)) / 100);
    }
}

bool RawProcessingFilter::continueQuery() const
{
    if (m_observer && !m_observer->continueQuery(0))
    {
        return false;
    }

    return runningFlag();
}

void RawProcessingFilter::filterImage()
{
    m_destImage = m_orgImage;

    // emulate LibRaw custom output profile
    if (!m_customOutputProfile.isNull())
    {
        // Note the m_destImage is not yet ready in load()!
        IccTransform trans;
        trans.setIntent(IccTransform::Perceptual);
        trans.setEmbeddedProfile(m_destImage);
        trans.setOutputProfile(m_customOutputProfile);
        trans.apply(m_orgImage, m_observer);
        m_destImage.setIccProfile(m_customOutputProfile);
    }

    postProgress(20);

    if (!m_settings.wb.isDefault())
    {
        WBFilter wb(m_settings.wb, this, m_orgImage, m_destImage, 20, 40);
    }

    postProgress(40);

    if (!m_settings.bcg.isDefault())
    {
        BCGFilter bcg(m_settings.bcg, this, m_orgImage, m_destImage, 40, 70);
    }

    postProgress(70);

    if (!m_settings.curvesAdjust.isEmpty())
    {
        CurvesFilter curves(m_settings.curvesAdjust, this, m_orgImage, m_destImage, 70, 100);
    }

    postProgress(100);
}

} // namespace Digikam
