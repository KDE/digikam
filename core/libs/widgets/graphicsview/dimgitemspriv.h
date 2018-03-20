/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View item for DImg
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIMGITEMSPRIV_H
#define DIMGITEMSPRIV_H

// Qt includes

#include <QPixmapCache>
#include <QQueue>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "dimgpreviewitem.h"
#include "imagezoomsettings.h"
#include "previewsettings.h"

namespace Digikam
{

class CachedPixmapKey
{
public:

    QRect             region;
    QPixmapCache::Key key;
};

// -------------------------------------------------------------------------------

class CachedPixmaps
{
public:

    explicit CachedPixmaps(int maxCount = 2);
    ~CachedPixmaps();

    void setMaxCount(int);
    void clear();
    bool find(const QRect& region, QPixmap* const pix, QRect* const source);
    void insert(const QRect& region, const QPixmap& pixmap);

protected:

    int                     maxCount;
    QQueue<CachedPixmapKey> keys;
};

// -------------------------------------------------------------------------------

class DIGIKAM_EXPORT GraphicsDImgItem::GraphicsDImgItemPrivate
{
public:

    GraphicsDImgItemPrivate()
    {
    }

    void init(GraphicsDImgItem* const q);

public:

    DImg                  image;
    ImageZoomSettings     zoomSettings;
    mutable CachedPixmaps cachedPixmaps;
};

// -------------------------------------------------------------------------------

class PreviewLoadThread;
class DImgPreviewItem;

class DIGIKAM_EXPORT DImgPreviewItem::DImgPreviewItemPrivate : public GraphicsDImgItem::GraphicsDImgItemPrivate
{
public:

    DImgPreviewItemPrivate();
    void init(DImgPreviewItem* const q);

public:

    DImgPreviewItem::State state;
    bool                   exifRotate;
    int                    previewSize;

    QString                path;
    PreviewSettings        previewSettings;
    PreviewLoadThread*     previewThread;
    PreviewLoadThread*     preloadThread;
    QStringList            pathsToPreload;
};

} // namespace Digikam

#endif // DIMGITEMSPRIV_H
