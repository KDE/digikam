/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a tab to display GPS info
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

#ifndef IMAGEPROPERTIESGPSTAB_H
#define IMAGEPROPERTIESGPSTAB_H

// Qt includes.

#include <QDateTime>
#include <QString>
#include <QWidget>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "dmetadata.h"
#include "worldmapwidget.h"
#include "digikam_export.h"

namespace Digikam
{

class ImagePropertiesGPSTabPriv;

class DIGIKAM_EXPORT ImagePropertiesGPSTab : public QWidget
{
    Q_OBJECT

public:

    enum WebGPSLocator
    {
        MapQuest = 0,
        GoogleMaps,
        MsnMaps,
        MultiMap,
        OpenStreetMap
    };

public:

    ImagePropertiesGPSTab(QWidget* parent);
    ~ImagePropertiesGPSTab();

    void setGPSInfo();
    void setGPSInfoList(const GPSInfoList& list);
    void setCurrentURL(const KUrl& url=KUrl());

    void setMetadata(const DMetadata& meta, const KUrl& url);

    int  getWebGPSLocator();
    void setWebGPSLocator(int locator);

private Q_SLOTS:

    void slotGPSDetails();

private:

    ImagePropertiesGPSTabPriv* const d;
};

}  // namespace Digikam

#endif /* IMAGEPROPERTIESGPSTAB_H */
