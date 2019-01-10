/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to YandexFotki web-service.
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

#include "yfplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "yfwindow.h"

namespace Digikam
{

YFPlugin::YFPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

YFPlugin::~YFPlugin()
{
    delete m_toolDlg;
}

QString YFPlugin::name() const
{
    return i18n("YandexFotki");
}

QString YFPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon YFPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("internet-web-browser"));
}

QString YFPlugin::description() const
{
    return i18n("A tool to export to YandexFotki web-service");
}

QString YFPlugin::details() const
{
    return i18n("<p>This tool permit to export items to YandexFotki web-service.</p>"
                "<p>See YandexFotki web site for details: <a href='https://fotki.yandex.ru/'>https://fotki.yandex.ru/</a></p>");
}

QList<DPluginAuthor> YFPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Roman Tsisyk"),
                             QLatin1String("roman at tsisyk dot com"),
                             QLatin1String("(C) 2010"))
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2010-2019"))
            ;
}

void YFPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Yandex.Fotki..."));
    ac->setObjectName(QLatin1String("export_yandexfotki"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::ALT + Qt::SHIFT + Qt::CTRL + Qt::Key_Y);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotYandexFotki()));

    addAction(ac);
}

void YFPlugin::slotYandexFotki()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new YFWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace Digikam
