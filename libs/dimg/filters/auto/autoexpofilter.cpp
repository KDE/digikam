/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-15-02
 * Description : auto exposure image filter.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"

namespace Digikam
{

AutoExpoFilter::AutoExpoFilter(DImg* orgImage, DImg* refImage, QObject* parent)
                : WBFilter(orgImage, parent)
{
    m_refImage = refImage->copy();
    initFilter();
}

AutoExpoFilter::~AutoExpoFilter()
{
}

void AutoExpoFilter::filterImage()
{
    if (m_orgImage.sixteenBit() != m_refImage.sixteenBit())
    {
        kDebug() << "Ref. image and Org. has different bits depth"; 
        return;
    }

    autoExposureAdjustement(&m_refImage, m_settings.black, m_settings.exposition);
    WBFilter::filterImage();
}

}  // namespace Digikam
