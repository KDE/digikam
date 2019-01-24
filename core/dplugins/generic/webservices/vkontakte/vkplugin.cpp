/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to VKontakte web-service.
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

#include "vkplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vkwindow.h"

namespace GenericDigikamVKontaktePlugin
{

VKontaktePlugin::VKontaktePlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

VKontaktePlugin::~VKontaktePlugin()
{
    delete m_toolDlg;
}

QString VKontaktePlugin::name() const
{
    return i18n("VKontakte");
}

QString VKontaktePlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon VKontaktePlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("preferences-web-browser-shortcuts"));
}

QString VKontaktePlugin::description() const
{
    return i18n("A tool to export to VKontakte web-service");
}

QString VKontaktePlugin::details() const
{
    return i18n("<p>This tool permit to export items to VKontakte web-service.</p>"
                "<p>See VKontakte web site for details: <a href='https://vk.com/'>https://vk.com/</a></p>");
}

QList<DPluginAuthor> VKontaktePlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Alexander Potashev"),
                             QString::fromUtf8("aspotashev at gmail dot com"),
                             QString::fromUtf8("(C) 2011-2015"))
            ;
}

void VKontaktePlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &VKontakte..."));
    ac->setObjectName(QLatin1String("export_vkontakte"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotVKontakte()));

    addAction(ac);
}

void VKontaktePlugin::slotVKontakte()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new VKWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace GenericDigikamVKontaktePlugin
