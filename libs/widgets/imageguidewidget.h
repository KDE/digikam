/* ============================================================
 * File  : imageguidewidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-20
 * Description : 
 * 
 * Copyright 2004-2005 Gilles Caulier
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
#include <qpoint.h>
#include <qcolor.h>
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{
class ImageIface;

class DIGIKAM_EXPORT ImageGuideWidget : public QWidget
{
Q_OBJECT

public:

    enum GuideToolMode 
    {
    HVGuideMode=0,
    PickColorMode
    };

public:

    ImageGuideWidget(int w, int h, QWidget *parent=0, bool spotVisible=true, int guideMode=HVGuideMode);
    ~ImageGuideWidget();
        
    Digikam::ImageIface* imageIface();
    
    QPoint getSpotPosition(void);
    QColor getSpotColor(void);
    void   setSpotVisible(bool v);
    void   resetSpotPosition(void);

signals:

    void spotPositionChanged( const QColor &color, bool release, const QPoint &position ); 
    
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
    
    int         m_guideMode;

    QRect       m_rect;       
    
    bool        m_focus;
    bool        m_freeze;
    bool        m_spotVisible;
    bool        m_mouseLeftButtonTracking;
    
    QPixmap    *m_pixmap;
};

}  // NameSpace Digikam

#endif /* IMAGEGUIDEWIDGET_H */
