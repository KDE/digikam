/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-22
 * Description : 
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#ifndef IMAGEPANICONWIDGET_H
#define IMAGEPANICONWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qrect.h>
#include <qsize.h>
#include <qpixmap.h>
#include <qpointarray.h>

namespace Digikam
{

class ImageIface;

class ImagePanIconWidget : public QWidget
{
Q_OBJECT

public:

    ImagePanIconWidget(int width, int height, QWidget *parent=0);
    ~ImagePanIconWidget();

    void  setRegionSelection(QRect regionSelection);
    QRect getRegionSelection(void);
    void  setCenterSelection(void);
        
    void  setHighLightPoints(QPointArray pointsList);
       
signals:

    // Used with ImagePreview widget. 
    // Emit when selection have been moved with mouse. 'targetDone' booleen 
    // value is used for indicate if the mouse have been released.
    void signalSelectionMoved( QRect rect, bool targetDone );     
    
    void signalSelectionTakeFocus(void);

public slots:

    void slotSeparateViewToggled(int t);
            
protected:
    
    void paintEvent( QPaintEvent *e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
        
private:

    int          m_separateView;
    
    int          m_w;
    int          m_h;
    
    int          m_xpos;
    int          m_ypos;

    bool         m_moveSelection;

    uchar *      m_data;
    
    QRect        m_rect;                    
    QRect        m_regionSelection;         // Original size image selection.
    QRect        m_localRegionSelection;    // Thumbnail size selection.
    
    QPixmap     *m_pixmap;
    
    QPointArray  m_hightlightPoints;
    
    ImageIface  *m_iface;
        
private:
    
    // Recalculate the target selection position and emit 'signalSelectionMoved'.
    
    void regionSelectionMoved( bool targetDone );
    void updatePixmap( void );

};

}  // NameSpace Digikam

#endif /* IMAGEPANICONWIDGET_H */
