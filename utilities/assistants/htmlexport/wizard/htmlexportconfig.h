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

#ifndef HTML_EXPORT_CONFIG_H
#define HTML_EXPORT_CONFIG_H

// Qt includes

#include <qglobal.h>
#include <QCoreApplication>
#include <QDebug>

// KDE includes

#include <kconfigskeleton.h>

namespace Digikam
{

class Config : public KConfigSkeleton
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

    explicit Config();
    virtual ~Config();

    void setTheme(const QString& v)
    {
        if (!isImmutable(QLatin1String("theme")))
            mTheme = v;
    }

    QString theme() const
    {
        return mTheme;
    }

    void setUseOriginalImageAsFullImage(bool v)
    {
        if (!isImmutable(QLatin1String("useOriginalImageAsFullImage")))
            mUseOriginalImageAsFullImage = v;
    }

    bool useOriginalImageAsFullImage() const
    {
        return mUseOriginalImageAsFullImage;
    }

    void setFullResize(bool v)
    {
        if (!isImmutable(QStringLiteral("fullResize")))
            mFullResize = v;
    }

    bool fullResize() const
    {
        return mFullResize;
    }

    void setFullSize( int v )
    {
        if (!isImmutable(QLatin1String("fullSize")))
            mFullSize = v;
    }

    int fullSize() const
    {
        return mFullSize;
    }

    void setFullFormat( int v )
    {
        if (!isImmutable(QLatin1String("fullFormat")))
            mFullFormat = v;
    }

    int fullFormat() const
    {
        return mFullFormat;
    }

    void setFullQuality( int v )
    {
        if (!isImmutable(QLatin1String("fullQuality")))
            mFullQuality = v;
    }

    int fullQuality() const
    {
        return mFullQuality;
    }

    void setCopyOriginalImage( bool v )
    {
        if (!isImmutable(QLatin1String("copyOriginalImage")))
            mCopyOriginalImage = v;
    }

    bool copyOriginalImage() const
    {
        return mCopyOriginalImage;
    }

    void setThumbnailSize( int v )
    {
        if (!isImmutable(QLatin1String("thumbnailSize")))
            mThumbnailSize = v;
    }

    int thumbnailSize() const
    {
        return mThumbnailSize;
    }

    void setThumbnailFormat( int v )
    {
        if (!isImmutable(QLatin1String("thumbnailFormat")))
            mThumbnailFormat = v;
    }

    int thumbnailFormat() const
    {
        return mThumbnailFormat;
    }

    void setThumbnailQuality( int v )
    {
        if (!isImmutable(QLatin1String("thumbnailQuality")))
            mThumbnailQuality = v;
    }

    int thumbnailQuality() const
    {
        return mThumbnailQuality;
    }

    void setThumbnailSquare( bool v )
    {
        if (!isImmutable(QLatin1String("thumbnailSquare")))
            mThumbnailSquare = v;
    }

    bool thumbnailSquare() const
    {
        return mThumbnailSquare;
    }

    void setDestUrl( const QUrl & v )
    {
        if (!isImmutable(QLatin1String("destUrl")))
            mDestUrl = v;
    }

    QUrl destUrl() const
    {
        return mDestUrl;
    }

    void setOpenInBrowser( bool v )
    {
        if (!isImmutable(QLatin1String("openInBrowser")))
            mOpenInBrowser = v;
    }

    bool openInBrowser() const
    {
        return mOpenInBrowser;
    }

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
};

} // namespace Digikam

#endif // HTML_EXPORT_CONFIG_H
