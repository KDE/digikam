/* ============================================================
 * File  : cameraiconitem.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-23
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef CAMERAICONITEM_H
#define CAMERAICONITEM_H

#include "thumbitem.h"

class QString;
class QPixmap;
class QPainter;
class QColorGroup;

class KFileItem;
class ThumbView;

class CameraIconItem : public ThumbItem
{
    friend class CameraIconView;
    
public:

    CameraIconItem(ThumbView* parent,
                   KFileItem* fileItem,
                   const QPixmap& pixmap);
    ~CameraIconItem();

    KFileItem* fileItem() const;
    
    void setPixmap(const QPixmap& pix);

protected:

    virtual void paintItem(QPainter *p, const QColorGroup& cg);
    
private:

    void loadBasePix();

    KFileItem* m_fileItem;
    int m_pixWidth, m_pixHeight;

    static QPixmap* baseRegPix;
    static QPixmap* baseSelPix;
};

#endif /* CAMERAICONITEM_H */
