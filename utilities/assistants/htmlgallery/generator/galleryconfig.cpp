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

    KConfigSkeleton::ItemString* const itemTheme
        = new KConfigSkeleton::ItemString(currentGroup(), QLatin1String("theme"), mTheme);

    addItem(itemTheme, QLatin1String("theme"));

    KConfigSkeleton::ItemBool* const itemUseOriginalImageAsFullImage
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("useOriginalImageAsFullImage"),
                                        mUseOriginalImageAsFullImage, false);

    addItem(itemUseOriginalImageAsFullImage, QLatin1String("useOriginalImageAsFullImage"));

    KConfigSkeleton::ItemBool* const itemFullResize
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("fullResize"), mFullResize, true);

    addItem(itemFullResize, QLatin1String("fullResize"));

    KConfigSkeleton::ItemInt* const itemFullSize
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("fullSize"), mFullSize, 1024);

    addItem(itemFullSize, QLatin1String("fullSize"));

    QList<KConfigSkeleton::ItemEnum::Choice> valuesfullFormat;
    {
        KConfigSkeleton::ItemEnum::Choice choice1;
        choice1.name = QLatin1String("JPEG");
        valuesfullFormat.append(choice1);
    }

    KConfigSkeleton::ItemEnum::Choice choice2;
    choice2.name = QLatin1String("PNG");
    valuesfullFormat.append(choice2);

    KConfigSkeleton::ItemEnum* const itemFullFormat
        = new KConfigSkeleton::ItemEnum(currentGroup(), QLatin1String("fullFormat"),
                                        mFullFormat, valuesfullFormat, EnumFullFormat::JPEG);

    addItem(itemFullFormat, QLatin1String("fullFormat"));

    KConfigSkeleton::ItemInt* const itemFullQuality
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("fullQuality"), mFullQuality, 80);

    addItem(itemFullQuality, QLatin1String("fullQuality"));

    KConfigSkeleton::ItemBool* const itemCopyOriginalImage
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("copyOriginalImage"),
                                        mCopyOriginalImage, false);

    addItem(itemCopyOriginalImage, QLatin1String("copyOriginalImage"));

    KConfigSkeleton::ItemInt* const itemThumbnailSize
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("thumbnailSize"), mThumbnailSize, 120);

    addItem(itemThumbnailSize, QLatin1String("thumbnailSize"));

    QList<KConfigSkeleton::ItemEnum::Choice> valuesthumbnailFormat;
    {
        KConfigSkeleton::ItemEnum::Choice choice3;
        choice3.name = QLatin1String("JPEG");
        valuesthumbnailFormat.append(choice3);
    }

    KConfigSkeleton::ItemEnum::Choice choice4;
    choice4.name = QLatin1String("PNG");
    valuesthumbnailFormat.append(choice4);

    KConfigSkeleton::ItemEnum* const itemThumbnailFormat
        = new KConfigSkeleton::ItemEnum(currentGroup(), QLatin1String("thumbnailFormat"),
                                        mThumbnailFormat, valuesthumbnailFormat, EnumThumbnailFormat::JPEG);

    addItem(itemThumbnailFormat, QLatin1String("thumbnailFormat"));

    KConfigSkeleton::ItemInt* const itemThumbnailQuality
        = new KConfigSkeleton::ItemInt(currentGroup(), QLatin1String("thumbnailQuality"),
                                       mThumbnailQuality, 80);

    addItem(itemThumbnailQuality, QLatin1String("thumbnailQuality"));

    KConfigSkeleton::ItemBool* const itemThumbnailSquare
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("thumbnailSquare"),
                                        mThumbnailSquare, true);

    addItem(itemThumbnailSquare, QLatin1String("thumbnailSquare"));

    KConfigSkeleton::ItemUrl* const itemDestUrl
        = new KConfigSkeleton::ItemUrl(currentGroup(), QLatin1String("destUrl"), mDestUrl);

    addItem(itemDestUrl, QLatin1String("destUrl"));

    KConfigSkeleton::ItemBool* const itemOpenInBrowser
        = new KConfigSkeleton::ItemBool(currentGroup(), QLatin1String("openInBrowser"), mOpenInBrowser, true);

    addItem(itemOpenInBrowser, QLatin1String("openInBrowser"));

    KConfigSkeleton::ItemString* const itemImageSelectionTitle
        = new KConfigSkeleton::ItemString(currentGroup(), QLatin1String("imageSelectionTitle"), mImageSelectionTitle);

    addItem(itemImageSelectionTitle, QLatin1String("imageSelectionTitle"));
}

GalleryConfig::~GalleryConfig()
{
}

void GalleryConfig::setTheme(const QString& v)
{
    if (!isImmutable(QLatin1String("theme")))
        mTheme = v;
}

QString GalleryConfig::theme() const
{
    return mTheme;
}

void GalleryConfig::setUseOriginalImageAsFullImage(bool v)
{
    if (!isImmutable(QLatin1String("useOriginalImageAsFullImage")))
        mUseOriginalImageAsFullImage = v;
}

bool GalleryConfig::GalleryConfig::useOriginalImageAsFullImage() const
{
    return mUseOriginalImageAsFullImage;
}

void GalleryConfig::setFullResize(bool v)
{
    if (!isImmutable(QStringLiteral("fullResize")))
        mFullResize = v;
}

bool GalleryConfig::fullResize() const
{
    return mFullResize;
}

void GalleryConfig::setFullSize(int v)
{
    if (!isImmutable(QLatin1String("fullSize")))
        mFullSize = v;
}

int GalleryConfig::fullSize() const
{
    return mFullSize;
}

void GalleryConfig::setFullFormat(int v)
{
    if (!isImmutable(QLatin1String("fullFormat")))
        mFullFormat = v;
}

int GalleryConfig::fullFormat() const
{
    return mFullFormat;
}

void GalleryConfig::setFullQuality(int v)
{
    if (!isImmutable(QLatin1String("fullQuality")))
        mFullQuality = v;
}

int GalleryConfig::fullQuality() const
{
    return mFullQuality;
}

void GalleryConfig::setCopyOriginalImage(bool v)
{
    if (!isImmutable(QLatin1String("copyOriginalImage")))
        mCopyOriginalImage = v;
}

bool GalleryConfig::copyOriginalImage() const
{
    return mCopyOriginalImage;
}

void GalleryConfig::setThumbnailSize(int v)
{
    if (!isImmutable(QLatin1String("thumbnailSize")))
        mThumbnailSize = v;
}

int GalleryConfig::thumbnailSize() const
{
    return mThumbnailSize;
}

void GalleryConfig::setThumbnailFormat(int v)
{
    if (!isImmutable(QLatin1String("thumbnailFormat")))
        mThumbnailFormat = v;
}

int GalleryConfig::thumbnailFormat() const
{
    return mThumbnailFormat;
}

void GalleryConfig::setThumbnailQuality(int v)
{
    if (!isImmutable(QLatin1String("thumbnailQuality")))
        mThumbnailQuality = v;
}

int GalleryConfig::thumbnailQuality() const
{
    return mThumbnailQuality;
}

void GalleryConfig::setThumbnailSquare(bool v)
{
    if (!isImmutable(QLatin1String("thumbnailSquare")))
        mThumbnailSquare = v;
}

bool GalleryConfig::thumbnailSquare() const
{
    return mThumbnailSquare;
}

void GalleryConfig::setDestUrl(const QUrl& v)
{
    if (!isImmutable(QLatin1String("destUrl")))
        mDestUrl = v;
}

QUrl GalleryConfig::destUrl() const
{
    return mDestUrl;
}

void GalleryConfig::setOpenInBrowser(bool v)
{
    if (!isImmutable(QLatin1String("openInBrowser")))
        mOpenInBrowser = v;
}

bool GalleryConfig::openInBrowser() const
{
    return mOpenInBrowser;
}

void GalleryConfig::setImageSelectionTitle(const QString& v)
{
    if (!isImmutable(QLatin1String("imageSelectionTitle")))
        mImageSelectionTitle = v;
}

QString GalleryConfig::imageSelectionTitle() const
{
    return mImageSelectionTitle;
}

} // namespace Digikam
