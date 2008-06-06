/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-06-06
 * Description : a widget to display GPS info on a world map
 * 
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WORLDMAPWIDGETDB_H
#define WORLDMAPWIDGETDB_H

// Local includes

#include "digikam_export.h"
#include "imageinfo.h"
#include "worldmapwidget.h"

namespace Digikam
{

class WorldMapWidgetDBPriv;

class DIGIKAM_EXPORT WorldMapWidgetDB : public WorldMapWidget
{
Q_OBJECT

public:

    WorldMapWidgetDB(int w, int h, QWidget *parent);
    virtual ~WorldMapWidgetDB();

    void setGPSPositions(const ImageInfoList& list);

private:

    WorldMapWidgetDBPriv *d;
};

}  // namespace Digikam

#endif /* WORLDMAPWIDGETDB_H */
