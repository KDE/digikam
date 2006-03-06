/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-20
 * Description : a widget to display GPS info on a world map
 * 
 * Copyright 2006 by Gilles Caulier
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

#include <qwidget.h>
#include <qpixmap.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WorldMapWidget : public QWidget
{
Q_OBJECT

public:

    WorldMapWidget( QWidget *parent, const char *name = 0 );
    ~WorldMapWidget();

    void setGPSPosition(double lat, double lng);

    double getLatitude(void);
    double getLongitude(void);

protected:

    void paintEvent( QPaintEvent* );

private:

    double m_latitude;
    double m_longitude;

    QPixmap m_worldMap;
};

}  // namespace Digikam

#endif /* WORLDMAPWIDGET_H */
