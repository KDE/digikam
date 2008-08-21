/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-20
 * Description : a widget to display GPS info on a world map
 * 
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WORLDMAPWIDGET_H
#define WORLDMAPWIDGET_H

// Qt includes.

#include <qscrollview.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class WorldMapWidgetPriv;

class DIGIKAM_EXPORT WorldMapWidget : public QScrollView
{
Q_OBJECT

public:

    WorldMapWidget(int w, int h, QWidget *parent);
    ~WorldMapWidget();

    void   setGPSPosition(double lat, double lng);
    
    double getLatitude();
    double getLongitude();
    void   setEnabled(bool);

private:

    void drawContents(QPainter *p, int x, int y, int w, int h);
    void contentsMousePressEvent ( QMouseEvent * e );
    void contentsMouseReleaseEvent ( QMouseEvent * e );
    void contentsMouseMoveEvent( QMouseEvent * e );

    QPixmap &worldMapPixmap();

private:

    WorldMapWidgetPriv *d;

};

}  // namespace Digikam

#endif /* WORLDMAPWIDGET_H */
