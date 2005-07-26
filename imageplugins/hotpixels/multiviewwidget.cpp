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

// Qt includes.

#include <qpainter.h>
#include <qpixmap.h>

// KDE includes.

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h> 

// Digikam includes.

#include <imageiface.h>

// Local includes.

#include "multiviewwidget.h"

#include <iostream>

MultiViewWidget::MultiViewWidget(int wp, int hp, QWidget *parent)
		: QFrame(parent)
{


	//Initialize variables
	m_zoom=0; //Autofit
	m_xpos=0;
	m_ypos=0;
	m_thumb=0;
	m_pix=0;
	
	//Create external widgets
	m_hScrollBar=new QScrollBar(Qt::Horizontal,this);
	m_vScrollBar=new QScrollBar(Qt::Vertical,this);
	
	
	//Load the image
	setImage();
	
	//Setup layout
	setFixedSize(wp,hp); //Important: set teh desired size before layouting, to get a correct thumb. The thumb won't be resized after the first layouting.
	setLayout();
	
	
	//Connect signals & slots
	connect(m_hScrollBar,SIGNAL(valueChanged(int)),this,SLOT(slotHScrollBarChanged(int)));
	connect(m_vScrollBar,SIGNAL(valueChanged(int)),this,SLOT(slotVScrollBarChanged(int)));
	
	this->setMouseTracking(true);
}

MultiViewWidget::~MultiViewWidget()
{
	if(m_pix) delete m_pix;
	if (m_thumb) delete m_thumb;
}

void MultiViewWidget::setImage(void)
{

	//Obtain the image from digikam
	Digikam::ImageIface iface(0, 0);
	int w = iface.originalWidth();
	int h = iface.originalHeight();
	uint *data = iface.getOriginalData();
	
	//Copy it to a pixmap
	m_img.create( w, h, 32 );
	memcpy(m_img.bits(), data, m_img.numBytes());
	m_pix = new QPixmap(w, h);
	m_pix->convertFromImage(m_img);
	
	
	delete [] data;
}

void MultiViewWidget::setCenterClipPosition(void)
{
    //center(contentsWidth()/2, contentsHeight()/2);    
    emit contentsMovedEvent();
}

void MultiViewWidget::setClipPosition(int x, int y, bool targetDone)
{
    //setContentsPos(x, y);    
    
    if( targetDone )
       emit contentsMovedEvent();
}

QRect MultiViewWidget::getImageRegion(void)
{
//    return( QRect::QRect(horizontalScrollBar()->value(), verticalScrollBar()->value(), 
//                         visibleWidth(), visibleHeight()) );
}

const QImage* MultiViewWidget::image(void)
{
	return (&m_img);
}

void MultiViewWidget::paintEvent(QPaintEvent* e)
{	
	QPainter p(this);

	//Repaint the image area except the place where the thumbnail is
	if(m_pix) 
	{
		QRegion reg=e->region();
		if (m_thumb) reg=reg.subtract(m_thumbRect);
		
		p.setClipRegion(reg);
		p.drawPixmap(0, 0, *m_pix, m_xpos,m_ypos, width(), height());
	}
	
	//Now redraw the thumbnail with the tracker
	if(m_thumb)
	{
		//Draw the thumb to clear the existing tracker
		int tx,ty,tw,th;
		tx=m_thumbRect.x(); ty=m_thumbRect.y(); tw=m_thumbRect.width(); th=m_thumbRect.height();
		p.setClipRegion(m_thumbRect);
		p.drawPixmap(tx,ty,*m_thumb,0,0,tw,th);
		p.setPen(QPen(Qt::white));
		p.drawRect (tx,ty,tw,th);
		
		//Now locate and draw the tracker
		float xRatio,yRatio;
		xRatio=(float)m_thumb->width()/(float)m_pix->width();
		yRatio=(float)m_thumb->height()/(float)m_pix->height();
		p.setPen(QPen(Qt::red));
		p.drawRect(tx+m_xpos*xRatio,ty+m_ypos*yRatio,width()*xRatio,height()*yRatio);
	}
	
	p.end();
}

void MultiViewWidget::contentsMousePressEvent ( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
       {
       m_xpos = e->x();
       m_ypos = e->y();
       setCursor ( KCursor::sizeAllCursor() );
       }
}

void MultiViewWidget::contentsMouseReleaseEvent ( QMouseEvent *  )
{
    setCursor ( KCursor::arrowCursor() );
    emit contentsMovedEvent();
}

void MultiViewWidget::contentsMouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() == Qt::LeftButton )
       {
       uint newxpos = e->x();
       uint newypos = e->y();
      
//       scrollBy (-(newxpos - m_xpos), -(newypos - m_ypos));
     
       m_xpos = newxpos - (newxpos-m_xpos);
       m_ypos = newypos - (newypos-m_ypos);
       return;
       }
        
    setCursor( KCursor::handCursor() );
}

void MultiViewWidget::resizeEvent (QResizeEvent *e)
{
	setLayout();
}
void MultiViewWidget::setLayout()
{
	m_hScrollBar->resize(width()-m_vScrollBar->sizeHint().width(),m_hScrollBar->sizeHint().height());
	m_vScrollBar->resize(m_vScrollBar->sizeHint().width(),height()-m_hScrollBar->sizeHint().height());
	m_hScrollBar->move(QPoint(0,height()-m_hScrollBar->height()));
	m_vScrollBar->move(QPoint(width()-m_vScrollBar->width(),0));
	

	//Create the thumb if it wasn't already created
	if (!m_thumb)
	{
		
		//Calculate an appropriate thumb size. We'll use
		//a width()/5*height()/5 thumb box at maximum
		int maxBoxX, maxBoxY;
		int thumbW, thumbH;
		maxBoxX=width()/5; maxBoxY=height()/5;
		
		if ((float)m_pix->width()/(float)maxBoxX >=
			(float)m_pix->height()/(float)maxBoxY)
		{
			//x needs more scaling down to fit in the box. Lets fit x
			thumbW=maxBoxX;
			thumbH=(int)(m_pix->height()*((float)maxBoxX/(float)m_pix->width()));
			
		}
		else
		{
			//y needs more scaling down to fit in the box. Lets fit y
			thumbH=maxBoxY;
			thumbW=(int)(m_pix->width()*((float)maxBoxY/(float)m_pix->height()));
		}
		
		//Create the thumb
		m_thumb = new QPixmap(thumbW,thumbH);
		QPainter pthumb(m_thumb);
		pthumb.drawPixmap(QRect(0,0,m_thumb->width(),m_thumb->height()),*m_pix);
		pthumb.end();
		
		//Set the thumb position on screen
		m_thumbRect=QRect(20,20,m_thumb->width(), m_thumb->height());
	}
	

	
	m_hScrollBar->setLineStep(m_pix->width()/100);
	m_vScrollBar->setLineStep(m_pix->height()/100);
	m_hScrollBar->setPageStep(m_hScrollBar->lineStep());
	m_vScrollBar->setPageStep(m_vScrollBar->lineStep());
	m_hScrollBar->setMinValue(0);
	m_vScrollBar->setMinValue(0);
	
	//change the maximum value of the scrollbars so it goes up to pixmapwidth-width
	//and pixmapheight-height
	if (m_pix)
	{
		m_hScrollBar->setMaxValue(m_pix->width()-width()-1);
		m_vScrollBar->setMaxValue(m_pix->height()-height()-1);
	}	

}

void MultiViewWidget::slotHScrollBarChanged(int pos)
{

	m_xpos=pos;
	repaint(QRect(0,0,width(),height()),false);
	
}
void MultiViewWidget::slotVScrollBarChanged(int pos)
{
	m_ypos=pos;
	repaint(QRect(0,0,width(),height()),false);
}

QRect MultiViewWidget::thumbRect(void)
{
	return m_thumbRect;
}

QPoint MultiViewWidget::pagePosition()
{
	return QPoint(m_hScrollBar->value(),m_vScrollBar->value());
}

#include "multiviewwidget.moc"
