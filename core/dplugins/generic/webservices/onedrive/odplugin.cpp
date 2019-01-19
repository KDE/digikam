/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to OneDrive web-service.
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

#include "odplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "odwindow.h"

namespace Digikam
{

ODPlugin::ODPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

ODPlugin::~ODPlugin()
{
    delete m_toolDlg;
}

QString ODPlugin::name() const
{
    return i18n("OneDrive");
}

QString ODPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ODPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-onedrive"));
}

QString ODPlugin::description() const
{
    return i18n("A tool to export to OneDrive web-service");
}

QString ODPlugin::details() const
{
    return i18n("<p>This tool permit to export items to OneDrive web-service.</p>"
                "<p>See Box web site for details: <a href='https://onedrive.live.com/'>https://onedrive.live.com/</a></p>");
}

QList<DPluginAuthor> ODPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Tarek Talaat"),
                             QString::fromUtf8("tarektalaat93 at gmail dot com"),
                             QString::fromUtf8("(C) 2018"))
            ;
}

void ODPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Onedrive..."));
    ac->setObjectName(QLatin1String("export_onedrive"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::ALT + Qt::SHIFT + Qt::CTRL + Qt::Key_O);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotOneDrive()));

    addAction(ac);
}

void ODPlugin::slotOneDrive()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new ODWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace Digikam
