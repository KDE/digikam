/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-08-22
 * Description : a widget to display a panel to choose
 *               a rectangular image area.
 * 
 * Copyright 2004-2007 by Gilles Caulier
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
#include <qpointarray.h>

namespace Digikam
{

class ImagePanIconWidgetPriv;

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

    void slotZoomFactorChanged( double );
    void slotSeparateViewToggled(int t);
            
protected:
    
    void paintEvent( QPaintEvent *e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
        
private:
    
    // Recalculate the target selection position and emit 'signalSelectionMoved'.
    
    void regionSelectionMoved( bool targetDone );
    void updatePixmap( void );

private:

    ImagePanIconWidgetPriv* d;
        
};

}  // NameSpace Digikam

#endif /* IMAGEPANICONWIDGET_H */
