/* ============================================================
* File  : imageeffect_hotpixels.cpp
* Author: Unai Garro <ugarro at users dot sourceforge dot net>
* Date  : 2005-07-05
* Description :  a ListView to display black frames
*
* Copyright 2005 by Unai Garro
*
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
#ifndef BLACKFRAMELISTVIEW_H
#define BLACKFRAMELISTVIEW_H

#include <qimage.h>
#include <qsize.h>
#include <qpoint.h>
#include <qvaluelist.h>

#include <klistview.h>
#include <kurl.h>
#include "blackframeparser.h"

class BlackFrameListView;

class BlackFrameListViewItem:public QObject, KListViewItem
{
Q_OBJECT

public:
	BlackFrameListViewItem(BlackFrameListView*parent,KURL url);
	~BlackFrameListViewItem(){}
	virtual QString text(int column)const;
	virtual void paintCell(QPainter* p,const QColorGroup& cg,int column,int width,int align);
	virtual int width (const QFontMetrics& fm,const QListView* lv,int c)const;
private:
	//Data contained within each listview item
	QImage mThumb;
	QImage mImage;
	QSize  mImageSize;
	QValueList <HotPixel> mHotPixels;
	BlackFrameParser mParser;
	
	//Private methods
	QPixmap thumb(QSize size);
	
private slots:
	void slotParsed(QValueList<HotPixel>);
	
	
};

class BlackFrameListView:public KListView
{
Q_OBJECT
public:
	BlackFrameListView(QWidget* parent=0);
	~BlackFrameListView(){}
};

#endif
