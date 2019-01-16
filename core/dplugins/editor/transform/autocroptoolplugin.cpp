/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to auto-crop an image.
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

#include "autocroptoolplugin.h"

// Qt includes

#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "imageiface.h"
#include "autocrop.h"

namespace Digikam
{

AutoCropToolPlugin::AutoCropToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

AutoCropToolPlugin::~AutoCropToolPlugin()
{
}

QString AutoCropToolPlugin::name() const
{
    return i18n("Auto-Crop");
}

QString AutoCropToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AutoCropToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-crop"));
}

QString AutoCropToolPlugin::description() const
{
    return i18n("A tool to auto-crop an image");
}

QString AutoCropToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can crop automatically an image by detection of inner black border,"
                "generated while panorama stitching for example.</p>");
}

QList<DPluginAuthor> AutoCropToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2005-2019"))
            ;
}

void AutoCropToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "&Auto-Crop"));
    ac->setObjectName(QLatin1String("editorwindow_transform_autocrop"));
    ac->setWhatsThis(i18n("This option can be used to crop automatically the image."));
    ac->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_X);
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotAutoCrop()));

    addAction(ac);
}

void AutoCropToolPlugin::slotAutoCrop()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    ImageIface iface;
    AutoCrop ac(iface.original());
    ac.startFilterDirectly();
    QRect rect = ac.autoInnerCrop();
    iface.crop(rect);

    QApplication::restoreOverrideCursor();
}

} // namespace Digikam
