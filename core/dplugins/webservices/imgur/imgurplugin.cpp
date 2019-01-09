/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to ImgUr web-service.
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

#include "imgurplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imgurwindow.h"

namespace Digikam
{

ImgUrPlugin::ImgUrPlugin(QObject* const parent)
    : DPlugin(parent)
{
}

ImgUrPlugin::~ImgUrPlugin()
{
    delete m_toolDlg;
}

QString ImgUrPlugin::name() const
{
    return i18n("ImgUr");
}

QString ImgUrPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ImgUrPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-imgur"));
}

QString ImgUrPlugin::description() const
{
    return i18n("A tool to export to ImgUr web-service");
}

QString ImgUrPlugin::details() const
{
    return i18n("<p>This tool permit to export items to ImgUr web-service.</p>"
                "<p>You can export items as anonymous or with an user account.</p>"
                "<p>See ImgUr web site for details: <a href='https://imgur.com/'>https://imgur.com/</a></p>");
}

QList<DPluginAuthor> ImgUrPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2012-2019"))
            << DPluginAuthor(QLatin1String("Marius Orcsik"),
                             QLatin1String("marius at habarnam dot ro"),
                             QLatin1String("(C) 2012-2013"))
            << DPluginAuthor(QLatin1String("Fabian Vogt"),
                             QLatin1String("fabian at ritter dash vogt dot de"),
                             QLatin1String("(C) 2014"))
            ;
}

void ImgUrPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Imgur..."));
    ac->setObjectName(QLatin1String("export_imgur"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotImgUr()));

    addAction(ac);
}

void ImgUrPlugin::slotImgUr()
{
    if (!reactivateToolDialog())
    {
        delete m_toolDlg;
        m_toolDlg = new ImgurWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace Digikam
