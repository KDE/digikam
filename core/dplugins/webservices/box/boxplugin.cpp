/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Box web-service.
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

#include "boxplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "boxwindow.h"

namespace Digikam
{

BoxPlugin::BoxPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

BoxPlugin::~BoxPlugin()
{
    delete m_toolDlg;
}

QString BoxPlugin::name() const
{
    return i18n("Box");
}

QString BoxPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BoxPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-box"));
}

QString BoxPlugin::description() const
{
    return i18n("A tool to export to Box web-service");
}

QString BoxPlugin::details() const
{
    return i18n("<p>This tool permit to export items to Box web-service.</p>"
                "<p>See Box web site for details: <a href='https://box.com/'>https://box.com/</a></p>");
}

QList<DPluginAuthor> BoxPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Tarek Talaat"),
                             QLatin1String("tarektalaat93 at gmail dot com"),
                             QLatin1String("(C) 2018"))
            ;
}

void BoxPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Box..."));
    ac->setObjectName(QLatin1String("export_box"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::ALT + Qt::SHIFT + Qt::CTRL + Qt::Key_B);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBox()));

    addAction(ac);
}

void BoxPlugin::slotBox()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new BOXWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace Digikam
