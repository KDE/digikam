/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to blend bracketed images.
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

#include "expoblendingplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "expoblendingmanager.h"

namespace GenericExpoBlendingPlugin
{

ExpoBlendingPlugin::ExpoBlendingPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

ExpoBlendingPlugin::~ExpoBlendingPlugin()
{
    if (ExpoBlendingManager::isCreated())
    {
        delete ExpoBlendingManager::internalPtr;
    }
}

QString ExpoBlendingPlugin::name() const
{
    return i18n("Exposure Blending");
}

QString ExpoBlendingPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ExpoBlendingPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("expoblending"));
}

QString ExpoBlendingPlugin::description() const
{
    return i18n("A tool to blend bracketed images");
}

QString ExpoBlendingPlugin::details() const
{
    return i18n("<p>This tool permit to blend bracketed images together to create pseudo HDR photo.</p>"
                "<p>To create high definition range image, you need to use images from same subject taken with a tripod and exposed with different exposure settings.</p>"
                "<p>To create image with better results, you can use RAW images instead JPEG, where colors depth is higher and are well adapted for merging pixels by pixels.</p>");
}

QList<DPluginAuthor> ExpoBlendingPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Johannes Wienke"),
                             QString::fromUtf8("languitar at semipol dot de"),
                             QString::fromUtf8("(C) 2010"))
            << DPluginAuthor(QString::fromUtf8("Benjamin Girault"),
                             QString::fromUtf8("benjamin dot girault at gmail dot com"),
                             QString::fromUtf8("(C) 2014"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2019"),
                             i18n("Author and Maintainer"))
            ;
}

void ExpoBlendingPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create Stacked Images..."));
    ac->setObjectName(QLatin1String("expoblending"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotExpoBlending()));

    addAction(ac);
}

void ExpoBlendingPlugin::slotExpoBlending()
{
    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(infoIface(sender())->currentSelectedItems());
    ExpoBlendingManager::instance()->setPlugin(this);
    ExpoBlendingManager::instance()->run();
}

} // namespace GenericExpoBlendingPlugin
