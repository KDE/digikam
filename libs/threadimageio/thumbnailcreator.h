/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef DIGIKAMTHUMBNAILCREATOR_H
#define DIGIKAMTHUMBNAILCREATOR_H

// Qt includes

#include <QString>
#include <QPixmap>
#include <QImage>

// KDE includes

#include <kfileitem.h>

// LibKDcraw includes.

#include <libkdcraw/rawdecodingsettings.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImgLoaderObserver;
class ThumbnailCreatorPriv;

class DIGIKAM_EXPORT ThumbnailCreator
{
public:
    ThumbnailCreator();
    ThumbnailCreator(int thumbnailSize);
    ~ThumbnailCreator();

    void setThumbnailSize(int thumbnailSize);
    void setLoadingProperties(DImgLoaderObserver *observer, KDcrawIface::RawDecodingSettings settings);

    QImage load(const QString &filePath, bool exifRotate);

    QString errorString() const;

protected:

    QImage loadWithDImg(const QString &path);
    QImage loadImagePreview(const QString& path);
    void exifRotate(const QString& filePath, QImage& thumb, bool fromEmbeddedPreview);

    // implementations in thumbnailbasic.cpp
    QImage loadPNG(const QString& path);
    void initThumbnailDirs();
    QString thumbnailUri(const QString &filePath);
    QString thumbnailPath(const QString &uri);

private:

    ThumbnailCreatorPriv *d;
};


}

#endif

