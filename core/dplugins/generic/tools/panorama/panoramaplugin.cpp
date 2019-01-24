/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to create panorama.
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

#include "panoramaplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "panomanager.h"

namespace GenericDigikamPanoramaPlugin
{

PanoramaPlugin::PanoramaPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

PanoramaPlugin::~PanoramaPlugin()
{
    if (PanoManager::isCreated())
    {
        delete PanoManager::internalPtr;
    }
}

QString PanoramaPlugin::name() const
{
    return i18n("Create Panorama");
}

QString PanoramaPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PanoramaPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("panorama"));
}

QString PanoramaPlugin::description() const
{
    return i18n("A tool to create panorama");
}

QString PanoramaPlugin::details() const
{
    return i18n("<p>This tool permit to assemble images together to create large panorama.</p>"
                "<p>To create panorama image, you need to use images taken from same point of view with a tripod and exposed with same settings.</p>"
                "<p>The tool is able to assemble shot taken horizontally, vertically, or as a matrix. Take a care that target image will become huge and require a lots of memory to be processed.</p>");
}

QList<DPluginAuthor> PanoramaPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Benjamin Girault"),
                             QString::fromUtf8("benjamin dot girault at gmail dot com"),
                             QString::fromUtf8("(C) 2011-2016"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2019"),
                             i18n("Author and Maintainer"))
            ;
}

void PanoramaPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create panorama..."));
    ac->setObjectName(QLatin1String("panorama"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPanorama()));

    addAction(ac);
}

void PanoramaPlugin::slotPanorama()
{
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(infoIface(sender())->currentSelectedItems());
    PanoManager::instance()->setPlugin(this);
    PanoManager::instance()->run();
}

} // namespace GenericDigikamPanoramaPlugin
