/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-17
 * Description : a widget to draw an image clip region.
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

#ifndef IMAGEREGIONWIDGET_H
#define IMAGEREGIONWIDGET_H

// Qt includes.

#include <qscrollview.h>
#include <qrect.h>
#include <qimage.h>
#include <qpointarray.h>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

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
    QRect  getImageRegion(void);
    QRect  getImageRegionToRender(void);
    DImg   getImageRegionImage(void);
    void   updatePreviewImage(DImg *img);
    void   updateOriginalImage(void);   

    void   backupPixmapRegion(void);
    void   restorePixmapRegion(void);
    
    void   setHighLightPoints(QPointArray pointsList);
             
    // FIXME remove these methods when all image plugins will be ported to DIMG.
    QImage getImageRegionData(void);
    void   updatePreviewImage(QImage *img);
             
signals:
    
    void contentsMovedEvent( bool target );
    
protected:
    
    void viewportResizeEvent( QResizeEvent *e );
    void contentsMousePressEvent ( QMouseEvent * e );
    void contentsMouseReleaseEvent ( QMouseEvent * e );
    void contentsMouseMoveEvent( QMouseEvent * e );
    void contentsWheelEvent( QWheelEvent * e ){ e->accept(); };

private:
    
    bool         m_movingInProgress;

    int          m_separateView;
    int          m_xpos;
    int          m_ypos;

    QPixmap     *m_pix;                // Entire content widget pixmap.
    QPixmap     *m_pixRegion;          // Pixmap of current region to render.
    
    QPointArray  m_hightlightPoints;    
    
    DImg        *m_img;                // Entire content image.
    
private:
    
    void  drawContents(QPainter *p, int x, int y, int w, int h);
    void  updatePixmap(DImg *img);
    QRect getTargetImageRegion(void);

    // FIXME remove this method when all image plugins will be ported to DIMG.
    void  updatePixmap(QImage *img);
    
public slots:

    void slotSeparateViewToggled(int mode);

private slots:
    
    void slotTimerResizeEvent();

};

}  // NameSpace Digikam

#endif /* IMAGEREGIONWIDGET_H */
