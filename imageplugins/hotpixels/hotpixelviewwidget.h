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
 
 
#ifndef HOTPIXELVIEWWIDGET_H
#define HOTPIXELVIEWWIDGET_H

#include <qvaluelist.h>
#include "hotpixel.h"
#include "multiviewwidget.h"

class HotPixelViewWidget : public MultiViewWidget
{
Q_OBJECT
public:
	HotPixelViewWidget(int w, int h, QWidget* parent=0);
	~HotPixelViewWidget();
	void addHotPixel(const HotPixel &hotPixel);
	const QValueList <HotPixel>& hotPixelList(void);

protected:
	virtual void paintEvent(QPaintEvent* e);

private:
	QValueList <HotPixel> m_hotPixelList;
	
};

#endif // HOTPIXELVIEWWIDGET_H
