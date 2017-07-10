/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-12-09
 * Description : a tool to print images
 *
 * Copyright (C) 2002-2003 by Todd Shoemaker <todd at theshoemakers dot net>
 * Copyright (C) 2007-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "advprintphoto.h"

// Qt includes

#include <QPainter>
#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "previewloadthread.h"

namespace Digikam
{

AdvPrintAdditionalInfo::AdvPrintAdditionalInfo()
    : m_unit(0),
      m_printPosition(0),
      m_scaleMode(0),
      m_keepRatio(true),
      m_autoRotate(true),
      m_printWidth(0.0),
      m_printHeight(0.0),
      m_enlargeSmallerImages(false)
{
}

AdvPrintAdditionalInfo::AdvPrintAdditionalInfo(const AdvPrintAdditionalInfo& ai)
{
    m_unit                 = ai.m_unit;
    m_printPosition        = ai.m_printPosition;
    m_scaleMode            = ai.m_scaleMode;
    m_keepRatio            = ai.m_keepRatio;
    m_autoRotate           = ai.m_autoRotate;
    m_printWidth           = ai.m_printWidth;
    m_printHeight          = ai.m_printHeight;
    m_enlargeSmallerImages = ai.m_enlargeSmallerImages;
}

AdvPrintAdditionalInfo::~AdvPrintAdditionalInfo()
{
}

// -----------------------------

AdvPrintCaptionInfo::AdvPrintCaptionInfo()
    : m_captionType(NoCaptions),
      m_captionFont(QLatin1String("Sans Serif")),
      m_captionColor(Qt::yellow),
      m_captionSize(2),
      m_captionText(QLatin1String(""))
{
}

AdvPrintCaptionInfo::AdvPrintCaptionInfo(const AdvPrintCaptionInfo& ci)
{
    m_captionType  = ci.m_captionType;
    m_captionFont  = ci.m_captionFont;
    m_captionColor = ci.m_captionColor;
    m_captionSize  = ci.m_captionSize;
    m_captionText  = ci.m_captionText;
}

AdvPrintCaptionInfo::~AdvPrintCaptionInfo()
{
}

// -----------------------------

AdvPrintPhoto::AdvPrintPhoto(int thumbnailSize, DInfoInterface* const iface)
    : m_pAddInfo(0),
      m_pAdvPrintCaptionInfo(0)
{
    m_size          = 0;
    m_cropRegion    = QRect(-1, -1, -1, -1);
    m_rotation      = 0;
    m_first         = false;
    m_copies        = 1;
    m_url           = QUrl();
    m_iface         = iface;
    m_thumbnail     = 0;
    m_thumbnailSize = thumbnailSize;
}

AdvPrintPhoto::AdvPrintPhoto(const AdvPrintPhoto& photo)
    : m_pAddInfo(0),
      m_pAdvPrintCaptionInfo(0)
{
    m_thumbnailSize = photo.m_thumbnailSize;
    m_cropRegion    = photo.m_cropRegion;
    m_url           = photo.m_url;
    m_first         = photo.m_first;
    m_copies        = photo.m_copies;
    m_rotation      = photo.m_rotation;

    if (photo.m_pAddInfo)
    {
        m_pAddInfo = new AdvPrintAdditionalInfo(*photo.m_pAddInfo);
    }

    if (photo.m_pAdvPrintCaptionInfo)
    {
        m_pAdvPrintCaptionInfo = new AdvPrintCaptionInfo(*photo.m_pAdvPrintCaptionInfo);
    }

    m_size      = 0;
    m_thumbnail = 0;
    m_iface     = photo.m_iface;
}

AdvPrintPhoto::~AdvPrintPhoto()
{
    delete m_thumbnail;
    delete m_size;
    delete m_pAddInfo;
    delete m_pAdvPrintCaptionInfo;
}

void AdvPrintPhoto::loadCache()
{
    // load the thumbnail and size only once.
    delete m_thumbnail;

    QImage photo = loadPhoto();
    QImage image = photo.scaled(m_thumbnailSize, m_thumbnailSize, Qt::KeepAspectRatio);
    m_thumbnail  = new QImage(image.width(), image.height(),
                              QImage::Format_ARGB32_Premultiplied);
    QPainter painter(m_thumbnail);
    painter.drawImage(0, 0, image );
    painter.end();

    delete m_size;
    m_size       = new QSize(photo.width(), photo.height());
}

QImage& AdvPrintPhoto::thumbnail()
{
    if (!m_thumbnail)
    {
        loadCache();
    }

    return *m_thumbnail;
}

QImage AdvPrintPhoto::loadPhoto()
{
    QImage photo;

    if (m_iface)
    {
        photo = PreviewLoadThread::loadHighQualitySynchronously(m_url.toLocalFile())
                .copyQImage();
    }

    if (photo.isNull())
    {
        photo.load(m_url.toLocalFile());
    }

    return photo;
}

QSize& AdvPrintPhoto::size()
{
    if (m_size == 0)
    {
        loadCache();
    }

    return *m_size;
}

int AdvPrintPhoto::width()
{
    return size().width();
}

int AdvPrintPhoto::height()
{
    return size().height();
}

double AdvPrintPhoto::scaleWidth(double unitToInches)
{
    Q_ASSERT(m_pAddInfo != 0);

    m_cropRegion = QRect(0, 0,
                         (int)(m_pAddInfo->m_printWidth  * unitToInches),
                         (int)(m_pAddInfo->m_printHeight * unitToInches));

    return m_pAddInfo->m_printWidth * unitToInches;
}

double AdvPrintPhoto::scaleHeight(double unitToInches)
{
    Q_ASSERT(m_pAddInfo != 0);

    m_cropRegion = QRect(0, 0,
                         (int)(m_pAddInfo->m_printWidth  * unitToInches),
                         (int)(m_pAddInfo->m_printHeight * unitToInches));

    return m_pAddInfo->m_printHeight * unitToInches;
}

} // Namespace Digikam
