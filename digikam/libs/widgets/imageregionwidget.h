/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-17
 * Description : a widget to draw a image clip region.
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

#ifndef IMAGEREGIONWIDGET_H
#define IMAGEREGIONWIDGET_H

// Qt includes.

#include <qscrollview.h>
#include <qimage.h>
#include <qrect.h>

class QPixmap;

namespace Digikam
{

class ImageRegionWidget : public QScrollView
{
Q_OBJECT

public:
    
    ImageRegionWidget(int wp, int hp, QWidget *parent=0, bool scrollBar=true);
    ~ImageRegionWidget();
    
    void   setClipPosition(int x, int y, bool targetDone);    
    void   setCenterClipPosition(void);
    QRect  getImageRegion(void);
    QImage getImageRegionData(void);

    void   updatePreviewImage(QImage *img);
    void   updateOriginalImage(void);   
         
signals:
    
    void contentsMovedEvent( void );

protected:
    
    void contentsMousePressEvent ( QMouseEvent * e );
    void contentsMouseReleaseEvent ( QMouseEvent * e );
    void contentsMouseMoveEvent( QMouseEvent * e );
    void viewportResizeEvent(QResizeEvent *e);

private:
    
    QPixmap   *m_pix;
    QImage     m_img;
   
    int        m_xpos;
    int        m_ypos;

private:
    
    void drawContents(QPainter *p, int x, int y, int w, int h);
    void updatePixmap(QImage *img);
};

}  // NameSpace Digikam

#endif /* IMAGEREGIONWIDGET_H */
