/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

    KConfigSkeleton::ItemString* const itemtheme
        = new KConfigSkeleton::ItemString(currentGroup(), QLatin1String("theme"), m_theme);

    addItem(itemtheme, QLatin1String("theme"));

    // -------------------

    KConfigSkeleton::ItemBool* const itemuseOriginalImageAsFullImage
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("useOriginalImageAsFullImage"),
                                        m_useOriginalImageAsFullImage, false);

    addItem(itemuseOriginalImageAsFullImage, QLatin1String("useOriginalImageAsFullImage"));

    // -------------------

    KConfigSkeleton::ItemBool* const itemfullResize
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("fullResize"), m_fullResize, true);

    addItem(itemfullResize, QLatin1String("fullResize"));

    KConfigSkeleton::ItemInt* const itemfullSize
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("fullSize"), m_fullSize, 1024);

    addItem(itemfullSize, QLatin1String("fullSize"));

    // -------------------

    QList<KConfigSkeleton::ItemEnum::Choice> valuesfullFormat;

    KConfigSkeleton::ItemEnum::Choice choice1;
    choice1.name = QLatin1String("JPEG");
    valuesfullFormat.append(choice1);

    KConfigSkeleton::ItemEnum::Choice choice2;
    choice2.name = QLatin1String("PNG");
    valuesfullFormat.append(choice2);

    KConfigSkeleton::ItemEnum* const itemfullFormat
        = new KConfigSkeleton::ItemEnum(currentGroup(), QLatin1String("fullFormat"),
                                        m_fullFormat, valuesfullFormat, EnumFullFormat::JPEG);

    addItem(itemfullFormat, QLatin1String("fullFormat"));

    // -------------------

    KConfigSkeleton::ItemInt* const itemfullQuality
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("fullQuality"), m_fullQuality, 80);

    addItem(itemfullQuality, QLatin1String("fullQuality"));

    KConfigSkeleton::ItemBool* const itemcopyOriginalImage
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("copyOriginalImage"),
                                        m_copyOriginalImage, false);

    addItem(itemcopyOriginalImage, QLatin1String("copyOriginalImage"));

    // -------------------

    KConfigSkeleton::ItemInt* const itemthumbnailSize
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("thumbnailSize"), m_thumbnailSize, 120);

    addItem(itemthumbnailSize, QLatin1String("thumbnailSize"));

    // -------------------

    QList<KConfigSkeleton::ItemEnum::Choice> valuesthumbnailFormat;

    KConfigSkeleton::ItemEnum::Choice choice3;
    choice3.name = QLatin1String("JPEG");
    valuesthumbnailFormat.append(choice3);

    KConfigSkeleton::ItemEnum::Choice choice4;
    choice4.name = QLatin1String("PNG");
    valuesthumbnailFormat.append(choice4);

    KConfigSkeleton::ItemEnum* const itemthumbnailFormat
        = new KConfigSkeleton::ItemEnum(currentGroup(), QLatin1String("thumbnailFormat"),
                                        m_thumbnailFormat, valuesthumbnailFormat, EnumThumbnailFormat::JPEG);

    addItem(itemthumbnailFormat, QLatin1String("thumbnailFormat"));

    // -------------------

    KConfigSkeleton::ItemInt* const itemthumbnailQuality
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("thumbnailQuality"),
                                       m_thumbnailQuality, 80);

    addItem(itemthumbnailQuality, QLatin1String("thumbnailQuality"));

    // -------------------

    KConfigSkeleton::ItemBool* const itemthumbnailSquare
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("thumbnailSquare"),
                                        m_thumbnailSquare, true);

    addItem(itemthumbnailSquare, QLatin1String("thumbnailSquare"));

    // -------------------

    KConfigSkeleton::ItemUrl* const itemdestUrl
        = new KConfigSkeleton::ItemUrl(currentGroup(), QLatin1String("destUrl"), m_destUrl);

    addItem(itemdestUrl, QLatin1String("destUrl"));

    // -------------------

    QList<KConfigSkeleton::ItemEnum::Choice> valueswebBrowser;

    KConfigSkeleton::ItemEnum::Choice choice5;
    choice5.name = QLatin1String("NONE");
    valueswebBrowser.append(choice5);

    KConfigSkeleton::ItemEnum::Choice choice6;
    choice6.name = QLatin1String("INTERNAL");
    valueswebBrowser.append(choice6);

    KConfigSkeleton::ItemEnum::Choice choice7;
    choice7.name = QLatin1String("DESKTOP");
    valueswebBrowser.append(choice7);

    KConfigSkeleton::ItemEnum* const itemwebBrowser
        = new KConfigSkeleton::ItemEnum(currentGroup(), QLatin1String("openInBrowser"),
                                        m_openInBrowser, valueswebBrowser,
                                        EnumWebBrowser::INTERNAL);

    addItem(itemwebBrowser, QLatin1String("openInBrowser"));

    // -------------------

    KConfigSkeleton::ItemString* const itemimageSelectionTitle
        = new KConfigSkeleton::ItemString(currentGroup(), QLatin1String("imageSelectionTitle"), m_imageSelectionTitle);

    addItem(itemimageSelectionTitle, QLatin1String("imageSelectionTitle"));
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

void GalleryConfig::setOpenInBrowser(int v)
{
    if (!isImmutable(QLatin1String("openInBrowser")))
        m_openInBrowser = v;
}

int GalleryConfig::openInBrowser() const
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
