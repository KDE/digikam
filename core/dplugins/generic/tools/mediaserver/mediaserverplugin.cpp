/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to share items with DLNA server.
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

#include "mediaserverplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmediaserverdlg.h"

namespace Digikam
{

MediaServerPlugin::MediaServerPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

MediaServerPlugin::~MediaServerPlugin()
{
}

QString MediaServerPlugin::name() const
{
    return i18n("DLNA Export");
}

QString MediaServerPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon MediaServerPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("arrow-right-double"));
}

QString MediaServerPlugin::description() const
{
    return i18n("A tool to export items to a DLNA compatible device");
}

QString MediaServerPlugin::details() const
{
    return i18n("<p>This tool permit to share items on the local network through a DLNA server.</p>"
                "<p>Items to share can be selected one by one or by group through a selection of albums.</p>"
                "<p>Many kind of electronic devices can support DLNA, as tablets, cellulars, TV, etc.</p>");
}

QList<DPluginAuthor> MediaServerPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2012-2019"),
                             i18n("Developer and Maintainer"))
            << DPluginAuthor(QLatin1String("Ahmed Fathi"),
                             QLatin1String("ahmed dot fathi dot abdelmageed at gmail dot com"),
                             QLatin1String("(C) 2015"))
            << DPluginAuthor(QLatin1String("Smit Mehta"),
                             QLatin1String("smit dot meh at gmail dot com"),
                             QLatin1String("(C) 2012-2013"))
            << DPluginAuthor(QLatin1String("Marcel Wiesweg"),
                             QLatin1String("marcel dot wiesweg at gmx dot de"),
                             QLatin1String("(C) 2012-2013"))
            ;
}

void MediaServerPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Share with DLNA..."));
    ac->setObjectName(QLatin1String("mediaserver"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotMediaServer()));

    addAction(ac);
}

void MediaServerPlugin::slotMediaServer()
{
    QPointer<DMediaServerDlg> w = new DMediaServerDlg(this, infoIface(sender()));
    w->setPlugin(this);
    w->exec();
    delete w;
}

} // namespace Digikam
