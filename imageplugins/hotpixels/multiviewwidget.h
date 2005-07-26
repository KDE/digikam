/* ============================================================
 * Author: Unai Garro <ugarro at users dot sourceforge dot net>
 * Date  : 2005-03-27
 * Description : a widget that allows zooming and navigating
 *		 through a large picture 
 * 
 * Copyright 2005 by Unai Garro
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

#ifndef MULTIVIEWWIDGET_H
#define MULTIVIEWWIDGET_H

// Qt includes.

#include <qscrollview.h>
#include <qimage.h>
#include <qrect.h>

class QPixmap;

class MultiViewWidget : public QFrame
{
Q_OBJECT

public:
    
    MultiViewWidget(int w, int h, QWidget *parent=0);
    ~MultiViewWidget();
    
    void          setClipPosition(int x, int y, bool targetDone);    
    void          setCenterClipPosition(void);
    QRect         getImageRegion(void);
    const QImage* image(void);
    QRect         thumbRect();
    QPoint	  pagePosition();
    
signals:
    
    void contentsMovedEvent( void );

protected:
    
    void contentsMousePressEvent ( QMouseEvent * e );
    void contentsMouseReleaseEvent ( QMouseEvent * e );
    void contentsMouseMoveEvent( QMouseEvent * e );
protected:
    
    QPixmap   *m_pix;
    QPixmap   *m_thumb;
    QImage     m_img;
    
   
    int m_xpos;
    int m_ypos;
    int m_zoom;

    virtual void paintEvent(QPaintEvent* e);
    virtual void resizeEvent (QResizeEvent *e);
    void setImage(void);
private: 
	QScrollBar *m_hScrollBar;
	QScrollBar *m_vScrollBar;
	QRect m_thumbRect;
	void setLayout(void);
	void setScrollbars(void);
private slots:
	void slotHScrollBarChanged(int pos);
	void slotVScrollBarChanged(int pos);

};

#endif /* MULTIVIEWWIDGET_H */
