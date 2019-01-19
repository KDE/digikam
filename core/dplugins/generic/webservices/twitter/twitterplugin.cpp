/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Twitter web-service.
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

#include "twitterplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "twitterwindow.h"

namespace Digikam
{

TwitterPlugin::TwitterPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

TwitterPlugin::~TwitterPlugin()
{
    delete m_toolDlg;
}

QString TwitterPlugin::name() const
{
    return i18n("Twitter");
}

QString TwitterPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TwitterPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-twitter"));
}

QString TwitterPlugin::description() const
{
    return i18n("A tool to export to Twitter web-service");
}

QString TwitterPlugin::details() const
{
    return i18n("<p>This tool permit to export items to Twitter web-service.</p>"
                "<p>See Twitter web site for details: <a href='https://twitter.com/'>https://twitter.com/</a></p>");
}

QList<DPluginAuthor> TwitterPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Tarek Talaat"),
                             QString::fromUtf8("tarektalaat93 at gmail dot com"),
                             QString::fromUtf8("(C) 2018"))
            ;
}

void TwitterPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Twitter..."));
    ac->setObjectName(QLatin1String("export-twitter"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotTwitter()));

    addAction(ac);
}

void TwitterPlugin::slotTwitter()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new TwWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace Digikam
