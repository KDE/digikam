/* ============================================================
 * Author: Unai Garro <ugarro at users dot sourceforge dot net>
 * Date  : 2005-03-27
 * Description : a widget that allows displaying/editing hot
 		 pixels
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
 
#include "hotpixelviewwidget.h"
#include <qpainter.h>
#include <iostream>

HotPixelViewWidget::HotPixelViewWidget(int w, int h, QWidget *parent):MultiViewWidget(w,h,parent)
{
			

}

HotPixelViewWidget::~HotPixelViewWidget()
{

}

 void HotPixelViewWidget::addHotPixel(const HotPixel &hPixel)
{
	m_hotPixelList.append(hPixel);
	QRect r=hPixel.rect;
	//redraw the pixel
	repaint(r);
	
	//and the thumb
	repaint(thumbRect());
	
}

void HotPixelViewWidget::paintEvent(QPaintEvent* e)
{
	
	//Let the MultiViewWidget draw itself first
	MultiViewWidget::paintEvent(e);
	
	//And now draw the hot pixels
	if (m_hotPixelList.empty()) return;
	
	QPainter p(this);
	QValueList <HotPixel>::Iterator it;
	for (it=m_hotPixelList.begin() ; it!=m_hotPixelList.end() ; ++it)
	{
	
		//Draw the fullsized hot pixel
		QPoint pagePos=pagePosition();
		QRect hpRect=(*it).rect;
		
			//locate correctly on the view part
		QRect viewRect=hpRect;
		QRect viewRect1,viewRect2,viewRect3,viewRect4;
		viewRect.moveBy(-pagePos.x(),-pagePos.y());
		
		
			//set a proper size around the pixels
		viewRect1.setRect(viewRect.x()-3,viewRect.y()-3,viewRect.width()+6,viewRect.height()+6);
		viewRect2.setRect(viewRect.x()-4,viewRect.y()-4,viewRect.width()+8,viewRect.height()+8);
		viewRect3.setRect(viewRect.x()-5,viewRect.y()-5,viewRect.width()+10,viewRect.height()+10);
		viewRect4.setRect(viewRect.x()-6,viewRect.y()-6,viewRect.width()+12,viewRect.height()+12);
		
			//Now draw it
		QRegion viewRegion(0,0,width(),height());
		viewRegion.subtract(thumbRect());

		
		p.setClipRegion(viewRegion);
		p.setPen(QPen(Qt::white));
		p.drawEllipse(viewRect1);
		p.setPen(QPen(Qt::black));
		p.drawEllipse(viewRect2);
		p.setPen(QPen(Qt::white));
		p.drawEllipse(viewRect3);
		p.setPen(QPen(Qt::black));
		p.drawEllipse(viewRect4);
		
		//p.setPen(QPen(Qt::green));
		//p.drawRect(viewRect);
		
		//Now draw the hot pixel in the thumb
			//Calculate the coordinates of the hot pixel's
			//center-point in the thumb
		float xRatio, yRatio;
		float hpThumbX,hpThumbY;
		
		xRatio=(float)thumbRect().width()/(float)image()->width();
		yRatio=(float)thumbRect().height()/(float)image()->height();
		QRect rThumb=thumbRect();
		
		hpThumbX=rThumb.x()+(hpRect.x()+hpRect.width()/2)*xRatio;
		hpThumbY=rThumb.y()+(hpRect.y()+hpRect.height()/2)*yRatio;
		
			//And draw it
		p.setClipRegion(rThumb);
		
		p.setPen(QPen(Qt::black));
		p.drawLine(hpThumbX,hpThumbY-1,hpThumbX,hpThumbY+1);
		p.drawLine(hpThumbX-1,hpThumbY,hpThumbX+1,hpThumbY);
		p.setPen(QPen(Qt::white));
		p.drawPoint(hpThumbX-1,hpThumbY-1);
		p.drawPoint(hpThumbX+1,hpThumbY+1);
		p.drawPoint(hpThumbX-1,hpThumbY+1);
		p.drawPoint(hpThumbX+1,hpThumbY-1);

		
	}
	p.end();
}

const QValueList <HotPixel>&  HotPixelViewWidget::hotPixelList(void)
{
	return m_hotPixelList;
}

#include "hotpixelviewwidget.moc"
