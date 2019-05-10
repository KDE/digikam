/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-02
 * Description : plugin to export images as wallpaper.
 *
 * Copyright (C) 2019 by Igor Antropov <antropovi at yahoo dot com>
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

#include "wallpaperplugin.h"

// Qt includes

#include <QDBusMessage>
#include <QDBusConnection>
#include <QString>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericWallpaperPlugin
{

WallpaperPlugin::WallpaperPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

WallpaperPlugin::~WallpaperPlugin()
{
}

QString WallpaperPlugin::name() const
{
    return i18n("Export as wallpaper");
}

QString WallpaperPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon WallpaperPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("preferences-desktop-wallpaper"));
}

QString WallpaperPlugin::description() const
{
    return i18n("A tool to set image as wallpaper");
}

QString WallpaperPlugin::details() const
{
    return i18n("<p>This tool changes background wallpaper to selected image for all desktops.</p>"
                "<p>If many images are selected, the first one will be used.</p>"
                "<p>If no image is selected, the first one from current album will be used.</p>");
}

QList<DPluginAuthor> WallpaperPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Igor Antropov"),
                             QString::fromUtf8("antropovi at yahoo dot com"),
                             QString::fromUtf8("(C) 2019"));
}

void WallpaperPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Set as wallpaper"));
    ac->setObjectName(QLatin1String("Wallpaper"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotWallpaper()));

    addAction(ac);
}

void WallpaperPlugin::slotWallpaper()
{
    DInfoInterface* const iface = infoIface(sender());
    QList<QUrl> images          = iface->currentSelectedItems();

    if (images.isEmpty())
    {
        images = iface->currentAlbumItems();
    }

    if (!images.isEmpty())
    {
        QDBusMessage message = QDBusMessage::createMethodCall(
            QLatin1String("org.kde.plasmashell"),
            QLatin1String("/PlasmaShell"),
            QLatin1String("org.kde.PlasmaShell"),
            QLatin1String("evaluateScript"));

        message << QString::fromUtf8(
            "var allDesktops = desktops();"
            "for (i=0;i<allDesktops.length;i++) {"
                "d = allDesktops[i];"
                "d.wallpaperPlugin = \"org.kde.image\";"
                "d.currentConfigGroup = "
                    "Array(\"Wallpaper\", \"org.kde.image\", \"General\");"
                "d.writeConfig(\"Image\", \"%1\")}").arg(images[0].toString());

        QDBusMessage reply = QDBusConnection::sessionBus().call(message);

        if (reply.type() == QDBusMessage::ErrorMessage)
        {
            QMessageBox::critical(nullptr,
                                  i18nc("@title:window",
                                        "Error while set image as wallpaper"),
                                  reply.errorMessage());
        }
    }
}

} // namespace DigikamGenericWallpaperPlugin
