/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export and import items with a remote location.
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

#include "ftplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ftexportwindow.h"
#include "ftimportwindow.h"

namespace Digikam
{

FTPlugin::FTPlugin(QObject* const parent)
    : DPlugin(parent)
{
}

FTPlugin::~FTPlugin()
{
    delete m_toolDlgExport;
    delete m_toolDlgImport;
}

QString FTPlugin::name() const
{
    return i18n("FileTransfer");
}

QString FTPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FTPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("folder-html"));
}

QString FTPlugin::description() const
{
    return i18n("A tool to export and import items with a remote location");
}

QString FTPlugin::details() const
{
    return i18n("<p>This tool permit to export and import items with a remote location.</p>"
                "<p>Many protocols can be used, as FTP, SFTP, SAMBA, etc.</p>");
}

QList<DPluginAuthor> FTPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2010-2019"))
            << DPluginAuthor(QLatin1String("Maik Qualmann"),
                             QLatin1String("metzpinguin at gmail dot com"),
                             QLatin1String("(C) 2017-2019"))
            << DPluginAuthor(QLatin1String("Johannes Wienke"),
                             QLatin1String("languitar at semipol dot de"),
                             QLatin1String("(C) 2009"))
            ;
}

void FTPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to remote storage..."));
    ac->setObjectName(QLatin1String("export_filetransfer"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_K);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFileTransferExport()));

    addAction(ac);

    DPluginAction* const ac2 = new DPluginAction(parent);
    ac2->setIcon(icon());
    ac2->setText(i18nc("@action", "Import from remote storage..."));
    ac2->setObjectName(QLatin1String("import_filetransfer"));
    ac2->setActionCategory(DPluginAction::GenericImport);
    ac2->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_I);

    connect(ac2, SIGNAL(triggered(bool)),
            this, SLOT(slotFileTransferImport()));

    addAction(ac2);

}

void FTPlugin::slotFileTransferExport()
{
    if (!reactivateToolDialog(m_toolDlgExport))
    {
        delete m_toolDlgExport;
        m_toolDlgExport = new FTExportWindow(infoIface(sender()), 0);
        m_toolDlgExport->setPlugin(this);
        m_toolDlgExport->show();
    }
}

void FTPlugin::slotFileTransferImport()
{
    if (!reactivateToolDialog(m_toolDlgImport))
    {
        delete m_toolDlgImport;
        m_toolDlgImport = new FTImportWindow(infoIface(sender()), 0);
        m_toolDlgImport->setPlugin(this);
        m_toolDlgImport->show();
    }
}

} // namespace Digikam
