/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-04-25
 * Description : implementation to render album icon item.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMICONITEM_H
#define ALBUMICONITEM_H

// Qt includes.

#include <QString>
#include <QDateTime>
#include <QRect>

// Local includes.

#include "iconitem.h"

class QPainter;

namespace Digikam
{

class ImageInfo;
class AlbumIconItemPriv;

class AlbumIconItem : public IconItem
{

public:

    AlbumIconItem(IconGroupItem* parent, const ImageInfo &info);
    ~AlbumIconItem();

    ImageInfo imageInfo() const;
    QString   filePath() const;

    QRect thumbnailRect() const;

    bool isDirty();

    static void dateToString(const QDateTime& datetime, QString& str);
    static QString squeezedText(QPainter* p, int width, const QString& text);

    virtual int compare(IconItem *item);
    virtual QRect clickToOpenRect();

protected:

    virtual void paintItem(QPainter *p);

private:

    AlbumIconItemPriv* const d;
};

}  // namespace Digikam

#endif  // ALBUMICONITEM_H
