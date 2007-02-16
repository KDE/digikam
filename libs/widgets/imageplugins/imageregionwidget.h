/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-08-17
 * Description : a widget to draw an image clip region.
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

#ifndef IMAGEREGIONWIDGET_H
#define IMAGEREGIONWIDGET_H

// Qt includes.

#include <qscrollview.h>
#include <qrect.h>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageRegionWidgetPriv;

class DIGIKAM_EXPORT ImageRegionWidget : public QScrollView
{
Q_OBJECT

public:
    
    enum SeparateViewMode 
    {
        SeparateViewHorizontal=0,
        SeparateViewVertical,
        SeparateViewNone,
        SeparateViewDuplicateVert,
        SeparateViewDuplicateHorz
    };
    
public:
    
    ImageRegionWidget(int wp, int hp, QWidget *parent=0, bool scrollBar=true);
    ~ImageRegionWidget();
    
    void   setContentsPosition(int x, int y, bool targetDone);    
    void   setCenterContentsPosition(void);

    /** To get image region including original or/and target area depending of separate view mode */
    QRect  getImageRegion(void);
    
    /** To get target image region area to render */
    QRect  getImageRegionToRender(void);
    
    /** To get target image region image to use for render operations */
    DImg   getImageRegionImage(void);

    void   updatePreviewImage(DImg *img);
    void   updateOriginalImage(void);   

    void   backupPixmapRegion(void);
    void   restorePixmapRegion(void);
    
    void   setHighLightPoints(QPointArray pointsList);
             
signals:
    
    void contentsMovedEvent( bool target );
    
protected:
    
    void viewportResizeEvent(QResizeEvent*);
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void contentsMouseMoveEvent(QMouseEvent*);
    void contentsWheelEvent(QWheelEvent *e){ e->accept(); };

private:
    
    void  drawContents(QPainter *p, int x, int y, int w, int h);
    void  updatePixmap(DImg& img);
    QRect getLocalTargetImageRegion(void);
    QRect getLocalImageRegionToRender(void);

public slots:

    void slotSeparateViewToggled(int mode);
    void slotZoomFactorChanged(double);

private slots:
    
    void slotTimerResizeEvent();

private:
    
    ImageRegionWidgetPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEREGIONWIDGET_H */
