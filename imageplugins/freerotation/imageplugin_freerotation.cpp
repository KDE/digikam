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

#include <kdebug.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "freerotationtool.h"

using namespace DigikamFreeRotationImagesPlugin;

K_PLUGIN_FACTORY( FreeRotationFactory, registerPlugin<ImagePlugin_FreeRotation>(); )
K_EXPORT_PLUGIN ( FreeRotationFactory("digikamimageplugin_freerotation") )

ImagePlugin_FreeRotation::ImagePlugin_FreeRotation(QObject *parent, const QVariantList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_FreeRotation")
{
    m_freerotationAction = new KAction(KIcon("freerotation"), i18n("Free Rotation..."), this);

    connect(m_freerotationAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotFreeRotation()));

    actionCollection()->addAction("imageplugin_freerotation", m_freerotationAction );

    setXMLFile("digikamimageplugin_freerotation_ui.rc");

    kDebug(50006) << "ImagePlugin_FreeRotation plugin loaded";
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
    loadTool(tool);
}
