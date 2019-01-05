/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to jAlbum gallery generator
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

#include "jalbumplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "jalbumwizard.h"

namespace Digikam
{

JAlbumPlugin::JAlbumPlugin(QObject* const parent)
    : DPlugin(parent)
{
}

JAlbumPlugin::~JAlbumPlugin()
{
}

QString JAlbumPlugin::name() const
{
    return i18n("jAlbum Export");
}

QString JAlbumPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon JAlbumPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("text-html"));
}

QString JAlbumPlugin::description() const
{
    return i18n("A tool to export images to jAlbum gallery generator");
}

QString JAlbumPlugin::details() const
{
    return i18n("<p>This tool permit to export items to jAlbum html gallery generator.</p>"
                "<p>Items to process can be selected one by one or by group through a selection of albums.</p>"
                "<p>jAlbum is themable with different templates and layout. See the jAlbum web-site for details: https://jalbum.net/.</p>");
}

QList<DPluginAuthor> JAlbumPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Andrew Goodbody"),
                             QLatin1String("ajg zero two at elfringham dot co dot uk"),
                             QLatin1String("(c) 2013-2019"),
                             i18n("Author and Maintainer"))
            ;
}

void JAlbumPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create jAlbum Gallery..."));
    ac->setObjectName(QLatin1String("jalbum"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotJAlbum()));

    addAction(ac);
}

void JAlbumPlugin::slotJAlbum()
{
    QPointer<JAlbumWizard> wzrd = new JAlbumWizard(0, infoIface(sender()));
    wzrd->setPlugin(this);
    wzrd->exec();
    delete wzrd;
}

} // namespace Digikam
