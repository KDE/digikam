/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-15-02
 * Description : auto exposure image filter.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "autoexpofilter.h"

// Local includes

#include "dimg.h"
#include "digikam_debug.h"

namespace Digikam
{

AutoExpoFilter::AutoExpoFilter(QObject* const parent)
    : WBFilter(parent)
{
    initFilter();
}

AutoExpoFilter::AutoExpoFilter(DImg* const orgImage, const DImg* const refImage, QObject* const parent)
    : WBFilter(orgImage, parent),
      m_refImage(*refImage)
{
    initFilter();
}

AutoExpoFilter::~AutoExpoFilter()
{
}

void AutoExpoFilter::filterImage()
{
    if (m_refImage.isNull())
    {
        m_refImage = m_orgImage;
    }

    if (m_orgImage.sixteenBit() != m_refImage.sixteenBit())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Ref. image and Org. have different bits depth";
        return;
    }

    autoExposureAdjustement(&m_refImage, m_settings.black, m_settings.expositionMain);
    WBFilter::filterImage();
}

FilterAction AutoExpoFilter::filterAction()
{
    return DefaultFilterAction<AutoExpoFilter>();
}

void AutoExpoFilter::readParameters(const FilterAction& action)
{
    Q_UNUSED(action);
}

}  // namespace Digikam
