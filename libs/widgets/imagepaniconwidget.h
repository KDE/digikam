/* ============================================================
 * File  : imagepaniconwidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-22
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

#ifndef IMAGEPANICONWIDGET_H
#define IMAGEPANICONWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qrect.h>

class QPixmap;

namespace Digikam
{

class ImageIface;

class ImagePanIconWidget : public QWidget
{
Q_OBJECT

public:

    ImagePanIconWidget(int width, int height, QWidget *parent=0);
    ~ImagePanIconWidget();

    void setRegionSelection(QRect regionSelection);
        
signals:

    void signalSelectionMoved( QRect rect, bool targetDone );

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

    QRect       m_rect;       
    QRect       m_regionSelection;         // Original size image selection.
    QRect       m_localRegionSelection;    // Thumbnail size selection.
    
    QPixmap*    m_pixmap;
    
    // Recalculate the target selection position and emit 'signalSelectionMoved'.
    
    void ImagePanIconWidget::regionSelectionChanged( bool targetDone );

};

}  // NameSpace Digikam

#endif /* IMAGEPANICONWIDGET_H */
