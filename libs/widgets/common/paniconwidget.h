/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-08-22
 * Description : a generic widget to display a panel to choose
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

#ifndef PANICONWIDGET_H
#define PANICONWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qrect.h>
#include <qimage.h>

namespace Digikam
{

class PanIconWidgetPriv;

class PanIconWidget : public QWidget
{
Q_OBJECT

public:

    PanIconWidget(QWidget *parent=0, WFlags flags=Qt::WDestructiveClose);
    ~PanIconWidget();

    void setImage(int previewWidth, int previewHeight, const QImage& image);

    void  setRegionSelection(QRect regionSelection);
    QRect getRegionSelection(void);
    void  setCenterSelection(void);

    void  setCursorToLocalRegionSelectionCenter(void);
    void  setMouseFocus(void);
       
signals:

    // Used with ImagePreview widget. 
    // Emit when selection have been moved with mouse. 'targetDone' booleen 
    // value is used for indicate if the mouse have been released.
    void signalSelectionMoved( QRect rect, bool targetDone );     
    
    void signalSelectionTakeFocus(void);

    void signalHiden(void);

public slots:

    void slotZoomFactorChanged( double );
            
protected:

    void hideEvent(QHideEvent *);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void timerEvent(QTimerEvent *);
        
private:
    
    // Recalculate the target selection position and emit 'signalSelectionMoved'.
    
    void regionSelectionMoved( bool targetDone );
    void updatePixmap();

private:

    PanIconWidgetPriv* d;        
};

}  // NameSpace Digikam

#endif /* PANICONWIDGET_H */
