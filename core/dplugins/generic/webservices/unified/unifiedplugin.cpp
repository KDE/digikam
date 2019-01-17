/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export items to web-services (unified version).
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

#include "unifiedplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "wswizard.h"

namespace Digikam
{

UnifiedPlugin::UnifiedPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

UnifiedPlugin::~UnifiedPlugin()
{
    delete m_toolDlg;
}

QString UnifiedPlugin::name() const
{
    return i18n("Unified");
}

QString UnifiedPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon UnifiedPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("internet-web-browser"));
}

QString UnifiedPlugin::description() const
{
    return i18n("A tool to export items to web-services");
}

QString UnifiedPlugin::details() const
{
    return i18n("<p>This tool permit to export items to many web-services.</p>"
                "<p>This is a unified tool, grouping many tools in one.</p>");
}

QList<DPluginAuthor> UnifiedPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2018-2019"))
            << DPluginAuthor(QLatin1String("Maik Qualmann"),
                             QLatin1String("metzpinguin at gmail dot com"),
                             QLatin1String("(C) 2018-2019"))
            << DPluginAuthor(QLatin1String("Thanh Trung Dinh"),
                             QLatin1String("dinhthanhtrung1996 at gmail dot com"),
                             QLatin1String("(C) 2018"))
            ;
}

void UnifiedPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to Webservices..."));
    ac->setObjectName(QLatin1String("export_unified"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotUnified()));

    addAction(ac);
}

void UnifiedPlugin::slotUnified()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new WSWizard(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace Digikam
