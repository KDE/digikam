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

extern "C"
{
#include <time.h>
}

// Local includes.

#include "thumbitem.h"


class QPainter;
class QPixmap;
class QImage;
class QColorGroup;

class KFileItem;
class KFileMetaInfo;

class AlbumIconView;


class AlbumIconItem : public ThumbItem 
{

    friend class AlbumIconView;

public:

    AlbumIconItem(AlbumIconView* parent, const QString& text,
                  const KFileItem* fileItem);
    ~AlbumIconItem();

    const KFileItem* fileItem() {
        return fileItem_;
    }

    void setPixmap(const QPixmap& thumb, const KFileMetaInfo* metaInfo);
    void setMetaInfo(const KFileMetaInfo* metaInfo);
    QRect thumbnailRect() const;

    virtual int compare(ThumbItem *item);
    
protected:

    virtual void calcRect();
    virtual void paintItem(QPainter* p, const QColorGroup& cg);

private:

    const KFileItem     *fileItem_;
    const KFileMetaInfo *metaInfo_;
    AlbumIconView       *view_;
    QPixmap              thumbnail_;
    time_t               time_;
};


#endif  // ALBUMICONITEM_H
