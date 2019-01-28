/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Rajce web-service.
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

#include "rajceplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "rajcewindow.h"

namespace DigikamGenericRajcePlugin
{

RajcePlugin::RajcePlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

RajcePlugin::~RajcePlugin()
{
    delete m_toolDlg;
}

QString RajcePlugin::name() const
{
    return i18n("Rajce");
}

QString RajcePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RajcePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-rajce"));
}

QString RajcePlugin::description() const
{
    return i18n("A tool to export to Rajce web-service");
}

QString RajcePlugin::details() const
{
    return i18n("<p>This tool permit to export items to Rajce web-service.</p>"
                "<p>See Rajce web site for details: <a href='https://www.rajce.idnes.cz/'>https://www.rajce.idnes.cz/</a></p>");
}

QList<DPluginAuthor> RajcePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Lukas Krejci"),
                             QString::fromUtf8("metlosh at gmail dot com"),
                             QString::fromUtf8("(C) 2011-2013"))
            ;
}

void RajcePlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Rajce..."));
    ac->setObjectName(QLatin1String("export_rajce"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::ALT + Qt::SHIFT + Qt::CTRL + Qt::Key_J);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRajce()));

    addAction(ac);
}

void RajcePlugin::slotRajce()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new RajceWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericRajcePlugin
