/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2006-05-16
 * @brief  A plugin to synchronize pictures with a GPS device.
 *
 * @author Copyright (C) 2006-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010,2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PLUGIN_GEOLOCATOR_H
#define PLUGIN_GEOLOCATOR_H

// Qt includes

#include <QVariant>

// Libkipi includes

#include <KIPI/Plugin>

class QAction;

namespace KIPI
{
    class Interface;
}



namespace Digikam
{

class Plugin_Geolocator : public Plugin
{
    Q_OBJECT

public:

    Plugin_Geolocator(QObject* const parent, const QVariantList& args);
    ~Plugin_Geolocator();

    void setup(QWidget* const);

protected Q_SLOTS:

    void slotGeolocator();

private:

    void setupActions();
    bool checkSidecarSettings();

private:

    QAction*   m_action_geolocation;
    Interface* m_interface;
};

} // namespace Digikam

#endif // PLUGIN_GEOLOCATOR_H
