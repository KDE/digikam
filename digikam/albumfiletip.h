/* ============================================================
 * Date  : 2004-08-19
 * Description : 
 *
 * Adapted from kfiletip (konqueror - konq_iconviewwidget.h)
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2000, 2001, 2002 David Faure <david@mandrakesoft.com>  
 * Copyright (C) 2004 by Renchi Raju<renchi@pooh.tam.uiuc.edu>
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

#ifndef ALBUMFILETIP_H
#define ALBUMFILETIP_H

// Qt includes.

#include <qframe.h>
#include <qstring.h>
#include <qpixmap.h>

class QLabel;
class QDateTime;
class AlbumIconView;
class AlbumIconItem;

class AlbumFileTip : public QFrame
{
public:

    AlbumFileTip(AlbumIconView* view);
    ~AlbumFileTip();

    void setIconItem(AlbumIconItem* iconItem);

protected:

    bool event(QEvent *e);
    void resizeEvent(QResizeEvent* e);
    void drawContents(QPainter *p);

private:

    void    reposition();
    void    renderArrows();
    void    updateText();
    QString breakString(const QString& str);
    
    AlbumIconView* m_view;
    AlbumIconItem* m_iconItem;
    QLabel*        m_label;
    int            m_corner;
    QPixmap        m_corners[4];
};

#endif /* ALBUMFILETIP_H */
