/* ============================================================
 * File  : imageguidewidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-20
 * Description : 
 * 
 * Copyright 2004 Gilles Caulier
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

#ifndef IMAGEGUIDEWIDGET_H
#define IMAGEGUIDEWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qrect.h>

class QPixmap;

namespace Digikam
{
class ImageIface;

class ImageGuideWidget : public QWidget
{
Q_OBJECT

public:

    ImageGuideWidget(int w, int h, QWidget *parent=0);
    ~ImageGuideWidget();
        
    Digikam::ImageIface* imageIface();

protected:
    
    void paintEvent( QPaintEvent *e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
        
private:

    Digikam::ImageIface *m_iface;
    
    uint       *m_data;
    int         m_w;
    int         m_h;
    
    int         m_xpos;
    int         m_ypos;

    QRect       m_rect;       
    
    bool        m_focus;
    bool        m_freeze;

    QPixmap*    m_pixmap;
};

}  // NameSpace Digikam

#endif /* IMAGEGUIDEWIDGET_H */
