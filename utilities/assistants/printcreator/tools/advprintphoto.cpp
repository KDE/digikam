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
    m_filename             = QUrl();

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
    m_filename      = photo.m_filename;
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
        photo = PreviewLoadThread::loadHighQualitySynchronously(m_filename.toLocalFile())
                .copyQImage();
    }

    if (photo.isNull())
    {
        photo.load(m_filename.toLocalFile());
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
    if (m_filename.url().isEmpty())
    {
        if (m_meta.load(m_filename.url()))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot load metadata from file " << m_filename;
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
