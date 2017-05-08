/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "galleryconfig.h"

// Qt includes

#include <QCoreApplication>
#include <QDebug>

namespace Digikam
{

GalleryConfig::GalleryConfig()
    : KConfigSkeleton(QLatin1String("digikamrc"))
{
    setCurrentGroup(QLatin1String("general"));

    KConfigSkeleton::ItemString* const item_theme
        = new KConfigSkeleton::ItemString(currentGroup(), QLatin1String("theme"), m_theme);

    addItem(item_theme, QLatin1String("theme"));

    KConfigSkeleton::ItemBool* const item_useOriginalImageAsFullImage
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("useOriginalImageAsFullImage"),
                                        m_useOriginalImageAsFullImage, false);

    addItem(item_useOriginalImageAsFullImage, QLatin1String("useOriginalImageAsFullImage"));

    KConfigSkeleton::ItemBool* const item_fullResize
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("fullResize"), m_fullResize, true);

    addItem(item_fullResize, QLatin1String("fullResize"));

    KConfigSkeleton::ItemInt* const item_fullSize
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("fullSize"), m_fullSize, 1024);

    addItem(item_fullSize, QLatin1String("fullSize"));

    QList<KConfigSkeleton::ItemEnum::Choice> valuesfullFormat;
    {
        KConfigSkeleton::ItemEnum::Choice choice1;
        choice1.name = QLatin1String("JPEG");
        valuesfullFormat.append(choice1);
    }

    KConfigSkeleton::ItemEnum::Choice choice2;
    choice2.name = QLatin1String("PNG");
    valuesfullFormat.append(choice2);

    KConfigSkeleton::ItemEnum* const item_fullFormat
        = new KConfigSkeleton::ItemEnum(currentGroup(), QLatin1String("fullFormat"),
                                        m_fullFormat, valuesfullFormat, EnumFullFormat::JPEG);

    addItem(item_fullFormat, QLatin1String("fullFormat"));

    KConfigSkeleton::ItemInt* const item_fullQuality
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("fullQuality"), m_fullQuality, 80);

    addItem(item_fullQuality, QLatin1String("fullQuality"));

    KConfigSkeleton::ItemBool* const item_copyOriginalImage
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("copyOriginalImage"),
                                        m_copyOriginalImage, false);

    addItem(item_copyOriginalImage, QLatin1String("copyOriginalImage"));

    KConfigSkeleton::ItemInt* const item_thumbnailSize
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("thumbnailSize"), m_thumbnailSize, 120);

    addItem(item_thumbnailSize, QLatin1String("thumbnailSize"));

    QList<KConfigSkeleton::ItemEnum::Choice> valuesthumbnailFormat;
    {
        KConfigSkeleton::ItemEnum::Choice choice3;
        choice3.name = QLatin1String("JPEG");
        valuesthumbnailFormat.append(choice3);
    }

    KConfigSkeleton::ItemEnum::Choice choice4;
    choice4.name = QLatin1String("PNG");
    valuesthumbnailFormat.append(choice4);

    KConfigSkeleton::ItemEnum* const item_thumbnailFormat
        = new KConfigSkeleton::ItemEnum(currentGroup(), QLatin1String("thumbnailFormat"),
                                        m_thumbnailFormat, valuesthumbnailFormat, EnumThumbnailFormat::JPEG);

    addItem(item_thumbnailFormat, QLatin1String("thumbnailFormat"));

    KConfigSkeleton::ItemInt* const item_thumbnailQuality
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("thumbnailQuality"),
                                       m_thumbnailQuality, 80);

    addItem(item_thumbnailQuality, QLatin1String("thumbnailQuality"));

    KConfigSkeleton::ItemBool* const item_thumbnailSquare
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("thumbnailSquare"),
                                        m_thumbnailSquare, true);

    addItem(item_thumbnailSquare, QLatin1String("thumbnailSquare"));

    KConfigSkeleton::ItemUrl* const item_destUrl
        = new KConfigSkeleton::ItemUrl(currentGroup(), QLatin1String("destUrl"), m_destUrl);

    addItem(item_destUrl, QLatin1String("destUrl"));

    KConfigSkeleton::ItemBool* const item_openInBrowser
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("openInBrowser"), m_openInBrowser, true);

    addItem(item_openInBrowser, QLatin1String("openInBrowser"));

    KConfigSkeleton::ItemString* const item_imageSelectionTitle
        = new KConfigSkeleton::ItemString(currentGroup(), QLatin1String("imageSelectionTitle"), m_imageSelectionTitle);

    addItem(item_imageSelectionTitle, QLatin1String("imageSelectionTitle"));
}

GalleryConfig::~GalleryConfig()
{
}

void GalleryConfig::setTheme(const QString& v)
{
    if (!isImmutable(QLatin1String("theme")))
        m_theme = v;
}

QString GalleryConfig::theme() const
{
    return m_theme;
}

void GalleryConfig::setUseOriginalImageAsFullImage(bool v)
{
    if (!isImmutable(QLatin1String("useOriginalImageAsFullImage")))
        m_useOriginalImageAsFullImage = v;
}

bool GalleryConfig::GalleryConfig::useOriginalImageAsFullImage() const
{
    return m_useOriginalImageAsFullImage;
}

void GalleryConfig::setFullResize(bool v)
{
    if (!isImmutable(QStringLiteral("fullResize")))
        m_fullResize = v;
}

bool GalleryConfig::fullResize() const
{
    return m_fullResize;
}

void GalleryConfig::setFullSize(int v)
{
    if (!isImmutable(QLatin1String("fullSize")))
        m_fullSize = v;
}

int GalleryConfig::fullSize() const
{
    return m_fullSize;
}

void GalleryConfig::setFullFormat(int v)
{
    if (!isImmutable(QLatin1String("fullFormat")))
        m_fullFormat = v;
}

int GalleryConfig::fullFormat() const
{
    return m_fullFormat;
}

void GalleryConfig::setFullQuality(int v)
{
    if (!isImmutable(QLatin1String("fullQuality")))
        m_fullQuality = v;
}

int GalleryConfig::fullQuality() const
{
    return m_fullQuality;
}

void GalleryConfig::setCopyOriginalImage(bool v)
{
    if (!isImmutable(QLatin1String("copyOriginalImage")))
        m_copyOriginalImage = v;
}

bool GalleryConfig::copyOriginalImage() const
{
    return m_copyOriginalImage;
}

void GalleryConfig::setThumbnailSize(int v)
{
    if (!isImmutable(QLatin1String("thumbnailSize")))
        m_thumbnailSize = v;
}

int GalleryConfig::thumbnailSize() const
{
    return m_thumbnailSize;
}

void GalleryConfig::setThumbnailFormat(int v)
{
    if (!isImmutable(QLatin1String("thumbnailFormat")))
        m_thumbnailFormat = v;
}

int GalleryConfig::thumbnailFormat() const
{
    return m_thumbnailFormat;
}

void GalleryConfig::setThumbnailQuality(int v)
{
    if (!isImmutable(QLatin1String("thumbnailQuality")))
        m_thumbnailQuality = v;
}

int GalleryConfig::thumbnailQuality() const
{
    return m_thumbnailQuality;
}

void GalleryConfig::setThumbnailSquare(bool v)
{
    if (!isImmutable(QLatin1String("thumbnailSquare")))
        m_thumbnailSquare = v;
}

bool GalleryConfig::thumbnailSquare() const
{
    return m_thumbnailSquare;
}

void GalleryConfig::setDestUrl(const QUrl& v)
{
    if (!isImmutable(QLatin1String("destUrl")))
        m_destUrl = v;
}

QUrl GalleryConfig::destUrl() const
{
    return m_destUrl;
}

void GalleryConfig::setOpenInBrowser(bool v)
{
    if (!isImmutable(QLatin1String("openInBrowser")))
        m_openInBrowser = v;
}

bool GalleryConfig::openInBrowser() const
{
    return m_openInBrowser;
}

void GalleryConfig::setImageSelectionTitle(const QString& v)
{
    if (!isImmutable(QLatin1String("imageSelectionTitle")))
        m_imageSelectionTitle = v;
}

QString GalleryConfig::imageSelectionTitle() const
{
    return m_imageSelectionTitle;
}

} // namespace Digikam
