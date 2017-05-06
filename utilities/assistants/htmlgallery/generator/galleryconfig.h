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

#ifndef GALLERY_CONFIG_H
#define GALLERY_CONFIG_H

// Qt includes

#include <QUrl>
#include <QString>

// KDE includes

#include <kconfigskeleton.h>

namespace Digikam
{

class GalleryConfig : public KConfigSkeleton
{
public:

    class EnumFullFormat
    {
    public:
        enum type
        {
            JPEG,
            PNG,
            COUNT
        };
    };

    class EnumThumbnailFormat
    {
    public:

        enum type
        {
            JPEG,
            PNG,
            COUNT
        };
    };

public:

    explicit GalleryConfig();
    virtual ~GalleryConfig();

    void setTheme(const QString&);
    QString theme() const;

    void setUseOriginalImageAsFullImage(bool);
    bool useOriginalImageAsFullImage() const;

    void setFullResize(bool);
    bool fullResize() const;

    void setFullSize(int);
    int fullSize() const;

    void setFullFormat(int);
    int fullFormat() const;

    void setFullQuality(int);
    int fullQuality() const;

    void setCopyOriginalImage(bool);
    bool copyOriginalImage() const;

    void setThumbnailSize(int);
    int thumbnailSize() const;

    void setThumbnailFormat(int);
    int thumbnailFormat() const;

    void setThumbnailQuality(int);
    int thumbnailQuality() const;

    void setThumbnailSquare(bool);
    bool thumbnailSquare() const;

    void setDestUrl(const QUrl&);
    QUrl destUrl() const;

    void setOpenInBrowser(bool);
    bool openInBrowser() const;

    void setImageSelectionTitle(const QString&);
    QString imageSelectionTitle() const;

protected:

    QString mTheme;
    bool    mUseOriginalImageAsFullImage;
    bool    mFullResize;
    int     mFullSize;
    int     mFullFormat;
    int     mFullQuality;
    bool    mCopyOriginalImage;
    int     mThumbnailSize;
    int     mThumbnailFormat;
    int     mThumbnailQuality;
    bool    mThumbnailSquare;
    QUrl    mDestUrl;
    bool    mOpenInBrowser;
    QString mImageSelectionTitle; // Gallery title to use for GalleryInfo::ImageGetOption::IMAGES selection.
};

} // namespace Digikam

#endif // GALLERY_CONFIG_H
