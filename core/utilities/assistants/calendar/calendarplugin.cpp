/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to create calendar.
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

#include "calendarplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "calwizard.h"

namespace Digikam
{

CalendarPlugin::CalendarPlugin(QObject* const parent)
    : DPlugin(parent)
{
}

CalendarPlugin::~CalendarPlugin()
{
}

QString CalendarPlugin::name() const
{
    return i18n("Create Calendar");
}

QString CalendarPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon CalendarPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("view-calendar"));
}

QString CalendarPlugin::description() const
{
    return i18n("A tool to create calendar from images");
}

QString CalendarPlugin::details() const
{
    return i18n("<p>This tool permit to compose items and create a calendar with you prefered photos.</p>"
                "<p>Different calendar layout are avaialble and you can integrate a list of dates from ICS format to highlight holidays time.</p>"
                "<p>Finaly, the tool will propose to export the result to your printer or in a PDF file.</p>");
}

QList<DPluginAuthor> CalendarPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2004-2019"),
                             i18n("Developer and Maintainer"))
            << DPluginAuthor(QLatin1String("Renchi Raju"),
                             QLatin1String("renchi dot raju at gmail dot com"),
                             QLatin1String("(C) 2003-2005"),
                             i18n("Former Author"))
            << DPluginAuthor(QLatin1String("Orgad Shaneh"),
                             QLatin1String("orgads at gmail dot com"),
                             QLatin1String("(C) 2007-2008"))
            << DPluginAuthor(QLatin1String("Tom Albers"),
                             QLatin1String("tomalbers at kde dot nl"),
                             QLatin1String("(C) 2006"))
            ;
}

void CalendarPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create Calendar..."));
    ac->setObjectName(QLatin1String("calendar"));
    ac->setActionCategory(DPluginAction::GenericTool);
    
    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotCalendar()));

    addAction(ac);
}

void CalendarPlugin::slotCalendar()
{
    QPointer<CalWizard> wzrd = new CalWizard(0, infoIface(sender()));
    wzrd->setPlugin(this);
    wzrd->exec();
    delete wzrd;
}

} // namespace Digikam
