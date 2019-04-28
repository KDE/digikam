/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Pinterest web-service.
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

#include "pplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "pwindow.h"

namespace DigikamGenericPinterestPlugin
{

PPlugin::PPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

PPlugin::~PPlugin()
{
    delete m_toolDlg;
}

QString PPlugin::name() const
{
    return i18n("Pinterest");
}

QString PPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-pinterest"));
}

QString PPlugin::description() const
{
    return i18n("A tool to export to Pinterest web-service");
}

QString PPlugin::details() const
{
    return i18n("<p>This tool permit to export items to Pinterest web-service.</p>"
                "<p>See Pinterest web site for details: <a href='https://www.pinterest.com/'>https://www.pinterest.com/</a></p>");
}

QList<DPluginAuthor> PPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Tarek Talaat"),
                             QString::fromUtf8("tarektalaat93 at gmail dot com"),
                             QString::fromUtf8("(C) 2018"))
            ;
}

void PPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Pinterest..."));
    ac->setObjectName(QLatin1String("export_pinterest"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_I);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPinterest()));

    addAction(ac);
}

void PPlugin::slotPinterest()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new PWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericPinterestPlugin
