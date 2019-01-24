/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export and import items with SmugMug web-service.
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

#include "smugplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "smugwindow.h"

namespace GenericDigikamSmugPlugin
{

SmugPlugin::SmugPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

SmugPlugin::~SmugPlugin()
{
    delete m_toolDlgExport;
    delete m_toolDlgImport;
}

QString SmugPlugin::name() const
{
    return i18n("SmugMug");
}

QString SmugPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SmugPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-smugmug"));
}

QString SmugPlugin::description() const
{
    return i18n("A tool to export and import items with SmugMug web-service");
}

QString SmugPlugin::details() const
{
    return i18n("<p>This tool permit to export and import items with SmugMug web-service.</p>"
                "<p>See SmugMug web site for details: <a href='https://www.smugmug.com'>https://www.smugmug.com</a></p>");
}

QList<DPluginAuthor> SmugPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Luka Renko"),
                             QString::fromUtf8("lure at kubuntu dot org"),
                             QString::fromUtf8("(C) 2008-2009"))
            << DPluginAuthor(QString::fromUtf8("Vardhman Jain"),
                             QString::fromUtf8("vardhman at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2008"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2017-2019"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2019"))
            ;
}

void SmugPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &SmugMug..."));
    ac->setObjectName(QLatin1String("export_smugmug"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_S);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotSmugMugExport()));

    addAction(ac);

    DPluginAction* const ac2 = new DPluginAction(parent);
    ac2->setIcon(icon());
    ac2->setText(i18nc("@action", "Import from &SmugMug..."));
    ac2->setObjectName(QLatin1String("import_smugmug"));
    ac2->setActionCategory(DPluginAction::GenericImport);
    ac2->setShortcut(Qt::ALT + Qt::SHIFT + Qt::CTRL + Qt::Key_S);

    connect(ac2, SIGNAL(triggered(bool)),
            this, SLOT(slotSmugMugImport()));

    addAction(ac2);

}

void SmugPlugin::slotSmugMugExport()
{
    if (!reactivateToolDialog(m_toolDlgExport))
    {
        delete m_toolDlgExport;
        m_toolDlgExport = new SmugWindow(infoIface(sender()), 0);
        m_toolDlgExport->setPlugin(this);
        m_toolDlgExport->show();
    }
}

void SmugPlugin::slotSmugMugImport()
{
    if (!reactivateToolDialog(m_toolDlgImport))
    {
        delete m_toolDlgImport;
        m_toolDlgImport = new SmugWindow(infoIface(sender()), 0, true);
        m_toolDlgImport->setPlugin(this);
        m_toolDlgImport->show();
    }
}

} // namespace GenericDigikamSmugPlugin
