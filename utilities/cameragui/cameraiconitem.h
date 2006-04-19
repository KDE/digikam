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

// Qt includes.

#include <qstring.h>
#include <qpixmap.h>

// Local includes.

#include "iconitem.h"

namespace Digikam
{

class GPItemInfo;

class CameraIconViewItem : public IconItem
{
public:

    CameraIconViewItem(IconGroupItem* parent, const GPItemInfo& itemInfo,
                       const QPixmap& pixmap, 
                       const QString& downloadName=QString::null);
    ~CameraIconViewItem();

    void    setPixmap(const QPixmap& pixmap);
    
    void    setDownloadName(const QString& downloadName);
    QString getDownloadName() const;
    void    setDownloaded();

    GPItemInfo* itemInfo() const
        { return m_itemInfo; }

    // reimplemented from IconItem
    virtual QRect clickToOpenRect();
    
protected:

    virtual void paintItem();
    
private:

    void calcRect();

private:

    GPItemInfo        *m_itemInfo;
    QString            m_downloadName;
    QPixmap            m_pixmap;

    QRect              m_pixRect;
    QRect              m_textRect;
    QRect              m_extraRect;
    
    static QPixmap    *m_newEmblem;
    static const char *new_xpm[];

    friend class CameraUI;
    friend class CameraIconView;
    
};

}  // namespace Digikam

#endif /* CAMERAICONITEM_H */
