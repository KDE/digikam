/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-09
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef IMAGESELECTIONWIDGET_H
#define IMAGESELECTIONWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qrect.h>

class QPixmap;

namespace Digikam
{

class ImageIface;

class ImageSelectionWidget : public QWidget
{
Q_OBJECT

public:

    ImageSelectionWidget(int width, int height, QWidget *parent=0);
    ~ImageSelectionWidget();

    void  setCenterSelection(void);
    void  setSelectionX(int x);
    void  setSelectionY(int y);
    void  setSelectionWidth(int w);
    void  setSelectionHeight(int h);
    void  setSelectionOrientation(int orient);
    void  setSelectionAspectRatio(int aspectRatio);
    
    int   getOriginalImageWidth(void);
    int   getOriginalImageHeight(void);
    QRect getRegionSelection(void);
    
    enum RatioAspect           // Contrained Aspect Ratio list.
    {
    RATIO01X01 = 0,            // 1:1
    RATIO02x03,                // 2:3
    RATIO03X04,                // 3:4
    RATIO04X05,                // 4:5
    RATIO05x07,                // 5:7
    RATIO07x10,                // 7:10
    };

    enum Orient  
    {
    Landscape = 0,            
    Paysage,                
    };
            
signals:

    void signalSelectionMoved( QRect rect, bool targetDone );     
    void signalSelectionChanged( QRect rect );   

protected:
    
    void paintEvent( QPaintEvent *e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
        
private:

    ImageIface *m_iface;
    
    uint       *m_data;
    int         m_w;
    int         m_h;
    
    int         m_xpos;
    int         m_ypos;

    bool        m_resizingSelMode;
    
    QRect       m_rect;                    
    QRect       m_regionSelection;         // Original size image selection.
    QRect       m_localRegionSelection;    // Thumbnail size selection.
    
    QPixmap*    m_pixmap;
    
    int         m_currentOrientation;
    int         m_currentAspectRatio;
    
    // Recalculate the target selection position and emit 'signalSelectionMoved'.
    
    void regionSelectionMoved( bool targetDone );
    void applyAspectRatio(bool WOrH);

};

}  // NameSpace Digikam

#endif /* IMAGESELECTIONWIDGET_H */
