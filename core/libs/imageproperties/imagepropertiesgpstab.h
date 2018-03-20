/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-22
 * Description : a tab to display GPS info
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef IMAGE_PROPERTIES_GPS_TAB_H
#define IMAGE_PROPERTIES_GPS_TAB_H

// Qt includes

#include <QWidget>
#include <QUrl>

// Local includes

#include "dmetadata.h"
#include "digikam_export.h"
#include "gpsimageinfosorter.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImagePropertiesGPSTab : public QWidget
{
    Q_OBJECT

public:

    enum WebGPSLocator
    {
        MapQuest = 0,
        GoogleMaps,
        BingMaps,
        OpenStreetMap,
        LocAlizeMaps
    };

public:

    explicit ImagePropertiesGPSTab(QWidget* const parent);
    ~ImagePropertiesGPSTab();

    void clearGPSInfo();
    void setGPSInfoList(const GPSImageInfo::List& list);
    void setCurrentURL(const QUrl& url=QUrl());

    void setMetadata(const DMetadata& meta, const QUrl& url);

    int  getWebGPSLocator() const;
    void setWebGPSLocator(int locator);

    void setActive(const bool state);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

private Q_SLOTS:

    void slotGPSDetails();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGE_PROPERTIES_GPS_TAB_H
