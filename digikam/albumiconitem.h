/* ============================================================
 * File  : albumiconitem.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-25
 * Description : 
 * 
 * Copyright 2003-2004 by Renchi Raju and Gilles Caulier
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

#include <qstring.h>
#include <qrect.h>
#include <qpixmap.h>

// Local includes.

#include "iconitem.h"

class QPainter;
class QPixmap;
class QImage;
class QColorGroup;

namespace Digikam
{

class ImageInfo;
class AlbumIconView;

class AlbumIconItem : public IconItem
{

    friend class AlbumIconView;

public:

    AlbumIconItem(IconGroupItem* parent, ImageInfo* info);
    ~AlbumIconItem();

    ImageInfo* imageInfo() const;

    QRect thumbnailRect() const;

    virtual int compare(IconItem *item);
    virtual QRect clickToOpenRect();
    
protected:

    virtual void paintItem();

private:

    ImageInfo           *info_;
    AlbumIconView       *view_;
    bool                 dirty_;
    QRect                tightPixmapRect_;
};

}  // namespace Digikam

#endif  // ALBUMICONITEM_H
