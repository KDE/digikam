/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to DropBox web-service.
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

#include "dbplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dbwindow.h"

namespace DigikamGenericDropBoxPlugin
{

DBPlugin::DBPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

DBPlugin::~DBPlugin()
{
    delete m_toolDlg;
}

QString DBPlugin::name() const
{
    return i18n("DropBox");
}

QString DBPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DBPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-dropbox"));
}

QString DBPlugin::description() const
{
    return i18n("A tool to export to DropBox web-service");
}

QString DBPlugin::details() const
{
    return i18n("<p>This tool permit to export items to DropBox web-service.</p>"
                "<p>See DropBox web site for details: <a href='https://www.dropbox.com/'>https://www.dropbox.com/</a></p>");
}

QList<DPluginAuthor> DBPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Pankaj Kumar"),
                             QString::fromUtf8("me at panks dot me"),
                             QString::fromUtf8("(C) 2013"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2018-2019"))
            ;
}

void DBPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Dropbox..."));
    ac->setObjectName(QLatin1String("export_dropbox"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_D);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotDropBox()));

    addAction(ac);
}

void DBPlugin::slotDropBox()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new DBWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericDropBoxPlugin
