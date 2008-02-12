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

#ifndef GPSTAB_H
#define GPSTAB_H

// Qt includes.

#include <QDateTime>
#include <QString>

// Local includes

#include "navigatebartab.h"
#include "digikam_export.h"

namespace Digikam
{

class GPSTabPriv;

class DIGIKAM_EXPORT GPSTab : public NavigateBarTab
{
    Q_OBJECT

public:

    enum WebGPSLocator
    {
        MapQuest = 0,
        GoogleMaps,
        MsnMaps,
        MultiMap
    };

public:

    GPSTab(QWidget* parent, bool navBar=true);
    ~GPSTab();

    void setGPSInfo();
    void setGPSInfo(double lat, double lon, long alt, const QDateTime dt);
    void setCurrentURL(const KUrl& url=KUrl());

    int  getWebGPSLocator();
    void setWebGPSLocator(int locator);

private slots:

    void slotGPSDetails();

private:

    GPSTabPriv *d;
};

}  // namespace Digikam

#endif /* GPSTAB_H */
