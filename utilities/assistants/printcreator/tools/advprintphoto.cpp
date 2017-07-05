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
    : mUnit(0),
      mPrintPosition(0),
      mScaleMode(0),
      mKeepRatio(true),
      mAutoRotate(true),
      mPrintWidth(0.0),
      mPrintHeight(0.0),
      mEnlargeSmallerImages(false)
{
}

AdvPrintAdditionalInfo::AdvPrintAdditionalInfo(const AdvPrintAdditionalInfo& ai)
{
    mUnit                 = ai.mUnit;
    mPrintPosition        = ai.mPrintPosition;
    mScaleMode            = ai.mScaleMode;
    mKeepRatio            = ai.mKeepRatio;
    mAutoRotate           = ai.mAutoRotate;
    mPrintWidth           = ai.mPrintWidth;
    mPrintHeight          = ai.mPrintHeight;
    mEnlargeSmallerImages = ai.mEnlargeSmallerImages;
}

AdvPrintAdditionalInfo::~AdvPrintAdditionalInfo()
{
}

// -----------------------------

AdvPrintCaptionInfo::AdvPrintCaptionInfo()
    : m_caption_type(NoCaptions),
      m_caption_font(QLatin1String("Sans Serif")),
      m_caption_color(Qt::yellow),
      m_caption_size(2),
      m_caption_text(QLatin1String(""))
{
}

AdvPrintCaptionInfo::AdvPrintCaptionInfo(const AdvPrintCaptionInfo& ci)
{
    m_caption_type  = ci.m_caption_type;
    m_caption_font  = ci.m_caption_font;
    m_caption_color = ci.m_caption_color;
    m_caption_size  = ci.m_caption_size;
    m_caption_text  = ci.m_caption_text;
}

AdvPrintCaptionInfo::~AdvPrintCaptionInfo()
{
}

// -----------------------------

AdvPrintPhoto::AdvPrintPhoto(int thumbnailSize, DInfoInterface* const iface)
    : m_pAddInfo(0),
      m_pAdvPrintCaptionInfo(0)
{
    m_size                 = 0;
    m_cropRegion           = QRect(-1, -1, -1, -1);
    m_rotation             = 0;
    m_first                = false;

    m_copies               = 1;
    //TODO mPrintPosition;
    m_url             = QUrl();

    m_iface                = iface;
    m_thumbnail            = 0;
    m_thumbnailSize        = thumbnailSize;
}

//to get old photo info
AdvPrintPhoto::AdvPrintPhoto (const AdvPrintPhoto& photo)
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
    m_thumbnail  = new QPixmap(image.width(), image.height());
    QPainter painter(m_thumbnail);
    painter.drawImage(0, 0, image );
    painter.end();

    delete m_size;
    m_size = new QSize(photo.width(), photo.height());
}

QPixmap& AdvPrintPhoto::thumbnail()
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

QSize& AdvPrintPhoto::size()  // private
{
    if (m_size == 0)
    {
        loadCache();
    }

    return *m_size;
}

DMetadata& AdvPrintPhoto::metaIface()
{
    if (!m_url.url().isEmpty())
    {
        if (m_meta.load(m_url.url()))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot load metadata from file " << m_url;
        }
    }

    return m_meta;
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
                         (int)(m_pAddInfo->mPrintWidth  * unitToInches),
                         (int)(m_pAddInfo->mPrintHeight * unitToInches));

    return m_pAddInfo->mPrintWidth * unitToInches;
}

double AdvPrintPhoto::scaleHeight(double unitToInches)
{
    Q_ASSERT(m_pAddInfo != 0);

    m_cropRegion = QRect(0,
                         0,
                         (int)(m_pAddInfo->mPrintWidth  * unitToInches),
                         (int)(m_pAddInfo->mPrintHeight * unitToInches));

    return m_pAddInfo->mPrintHeight * unitToInches;
}

} // Namespace Digikam
