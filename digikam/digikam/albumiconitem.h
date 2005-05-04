//////////////////////////////////////////////////////////////////////////////
//
//    ALBUMICONITEM.H
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

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

class ImageInfo;
class KFileMetaInfo;

class AlbumIconView;


class AlbumIconItem : public IconItem
{

    friend class AlbumIconView;

public:

    AlbumIconItem(IconGroupItem* parent, ImageInfo* info);
    ~AlbumIconItem();

    ImageInfo* imageInfo() const;

    void setMetaInfo(const KFileMetaInfo* metaInfo);
    QRect thumbnailRect() const;

    virtual int compare(IconItem *item);
    
protected:

    virtual void paintItem();

private:

    ImageInfo           *info_;
    const KFileMetaInfo *metaInfo_;
    AlbumIconView       *view_;
    bool                 dirty_;
};


#endif  // ALBUMICONITEM_H
