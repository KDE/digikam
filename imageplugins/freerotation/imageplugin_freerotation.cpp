/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-28
 * Description : a digiKam image editor plugin to process image
 *               free rotation.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


#include "imageplugin_freerotation.h"
#include "imageplugin_freerotation.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "freerotationtool.h"
#include "debug.h"

using namespace DigikamFreeRotationImagesPlugin;

K_PLUGIN_FACTORY( FreeRotationFactory, registerPlugin<ImagePlugin_FreeRotation>(); )
K_EXPORT_PLUGIN ( FreeRotationFactory("digikamimageplugin_freerotation") )

ImagePlugin_FreeRotation::ImagePlugin_FreeRotation(QObject *parent, const QVariantList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_FreeRotation")
{
    QString pluginName(i18n("Free Rotation"));

    // we want to have an actionCategory for this plugin (if possible), set a name for it
    setActionCategory(pluginName);

    m_freerotationAction = new KAction(KIcon("freerotation"), QString("%1...").arg(pluginName), this);
    connect(m_freerotationAction, SIGNAL(triggered(bool) ), this, SLOT(slotFreeRotation()));
    actionCollection()->addAction("imageplugin_freerotation", m_freerotationAction );

    KAction* point1Action = new KAction(i18n("Set Point 1"), this);
    point1Action->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_1));
    connect(point1Action, SIGNAL(triggered(bool)), this, SIGNAL(signalPoint1Action()));
    actionCollection()->addAction("imageplugin_freerotation_point1", point1Action);

    KAction* point2Action = new KAction(i18n("Set Point 2"), this);
    point2Action->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_2));
    connect(point2Action, SIGNAL(triggered(bool)), this, SIGNAL(signalPoint2Action()));
    actionCollection()->addAction("imageplugin_freerotation_point2", point2Action);

    KAction* autoAdjustAction = new KAction(i18n("Auto Adjust"), this);
    autoAdjustAction->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_R));
    connect(autoAdjustAction, SIGNAL(triggered(bool)), this, SIGNAL(signalAutoAdjustAction()));
    actionCollection()->addAction("imageplugin_freerotation_autoadjust", autoAdjustAction);

    setXMLFile("digikamimageplugin_freerotation_ui.rc");

    kDebug(imagePluginsAreaCode) << "ImagePlugin_FreeRotation plugin loaded";
}

ImagePlugin_FreeRotation::~ImagePlugin_FreeRotation()
{
}

void ImagePlugin_FreeRotation::setEnabledActions(bool enable)
{
    m_freerotationAction->setEnabled(enable);
}

void ImagePlugin_FreeRotation::slotFreeRotation()
{
    FreeRotationTool *tool = new FreeRotationTool(this);

    connect(this, SIGNAL(signalPoint1Action()),
            tool, SLOT(slotAutoAdjustP1Clicked()));

    connect(this, SIGNAL(signalPoint2Action()),
            tool, SLOT(slotAutoAdjustP2Clicked()));

    connect(this, SIGNAL(signalAutoAdjustAction()),
            tool, SLOT(slotAutoAdjustClicked()));

    loadTool(tool);
}
