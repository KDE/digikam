/* ============================================================
 * File  : cameraiconitem.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-21
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qiconview.h>
#include <qstring.h>

class GPItemInfo;

class CameraIconViewItem : public QIconViewItem
{
public:

    CameraIconViewItem(QIconView* parent, const GPItemInfo& itemInfo,
                       const QString& downloadName=QString::null);
    ~CameraIconViewItem();

    void    setDownloadName(const QString& downloadName);
    QString getDownloadName() const;

    GPItemInfo* itemInfo() const
        { return m_itemInfo; }
    
protected:

    void calcRect(const QString & text_ = QString::null);
    void paintItem(QPainter *, const QColorGroup& cg);
    void paintFocus(QPainter *, const QColorGroup& cg);
    
private:

    GPItemInfo* m_itemInfo;
    QString     m_downloadName;
    QRect       m_itemExtraRect;

    friend class CameraUI;
    friend class CameraIconView;
};

#endif /* CAMERAICONITEM_H */
