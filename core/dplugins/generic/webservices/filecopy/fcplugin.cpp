/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : a plugin to export items to a local storage.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2019      by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "fcplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "fcexportwindow.h"

namespace DigikamGenericFileCopyPlugin
{

FCPlugin::FCPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

FCPlugin::~FCPlugin()
{
    delete m_toolDlgExport;
}

QString FCPlugin::name() const
{
    return i18n("FileCopy");
}

QString FCPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FCPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("drive-removable-media"));
}

QString FCPlugin::description() const
{
    return i18n("A tool to export items to a local storage");
}

QString FCPlugin::details() const
{
    return i18n("<p>This tool permit to export items to a local storage.</p>");
}

QList<DPluginAuthor> FCPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Johannes Wienke"),
                             QString::fromUtf8("languitar at semipol dot de"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2017-2019"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2019"))
            ;
}

void FCPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to local storage..."));
    ac->setObjectName(QLatin1String("export_filecopy"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_L);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFileCopyExport()));

    addAction(ac);
}

void FCPlugin::slotFileCopyExport()
{
    if (!reactivateToolDialog(m_toolDlgExport))
    {
        delete m_toolDlgExport;
        m_toolDlgExport = new FCExportWindow(infoIface(sender()), 0);
        m_toolDlgExport->setPlugin(this);
        m_toolDlgExport->show();
    }
}

} // namespace DigikamGenericFileCopyPlugin
