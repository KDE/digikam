/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-21
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef CAMERAICONITEM_H
#define CAMERAICONITEM_H

#include <thumbitem.h>
#include <qstring.h>

class GPItemInfo;

class CameraIconViewItem : public ThumbItem
{
public:

    CameraIconViewItem(ThumbView* parent, const GPItemInfo& itemInfo,
                       const QPixmap& pixmap, 
                       const QString& downloadName=QString::null);
    ~CameraIconViewItem();

    void    setDownloadName(const QString& downloadName);
    QString getDownloadName() const;
    void    setDownloaded();

    GPItemInfo* itemInfo() const
        { return m_itemInfo; }

protected:

    virtual void calcRect();
    virtual void paintItem(QPainter *, const QColorGroup& cg);
    
private:

    GPItemInfo* m_itemInfo;
    QString     m_downloadName;
    QRect       m_itemExtraRect;

    static QPixmap*    m_newEmblem;
    static const char* new_xpm[];
    
    friend class CameraUI;
    friend class CameraIconView;
};

#endif /* CAMERAICONITEM_H */
