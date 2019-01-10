/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to edit items geolocation.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "geolocationeditplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "geolocationedit.h"

namespace Digikam
{

GeolocationEditPlugin::GeolocationEditPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

GeolocationEditPlugin::~GeolocationEditPlugin()
{
}

QString GeolocationEditPlugin::name() const
{
    return i18n("Geolocation Edit");
}

QString GeolocationEditPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon GeolocationEditPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("globe"));
}

QString GeolocationEditPlugin::description() const
{
    return i18n("A tool to edit items geolocation");
}

QString GeolocationEditPlugin::details() const
{
    return i18n("<p>This tool permit to changes geolocation information from items.</p>"
                "<p>This tool can edit GPS data, manualy or over a map. Reverse geo-coding is also available through web services.</p>"
                "<p>This tool as also an export function to KML to store map traces in Google format.</p>"
                "<p>Finaly, this tool is able to read a GPS trace from a device to synchronize geo-location of items if you camera do not have an embeded GPS device.</p>");
}

QList<DPluginAuthor> GeolocationEditPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2006-2019"))
            << DPluginAuthor(QLatin1String("Michael G. Hansen"),
                             QLatin1String("mike at mghansen dot de"),
                             QLatin1String("(C) 2008-2012"))
            << DPluginAuthor(QLatin1String("Gabriel Voicu"),
                             QLatin1String("ping dot gabi at gmail dot com"),
                             QLatin1String("(C) 2010-2012"))
            << DPluginAuthor(QLatin1String("Justus Schwartz"),
                             QLatin1String("justus at gmx dot li"),
                             QLatin1String("(C) 2014"))
            ;
}

void GeolocationEditPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Edit Geolocation..."));
    ac->setObjectName(QLatin1String("geolocation_edit"));
    ac->setActionCategory(DPluginAction::GenericMetadata);
    ac->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotEditGeolocation()));

    addAction(ac);
}

void GeolocationEditPlugin::slotEditGeolocation()
{
    QPointer<GeolocationEdit> dialog = new GeolocationEdit(0, infoIface(sender()));
    dialog->setPlugin(this);
    dialog->exec();
    delete dialog;
}

} // namespace Digikam
