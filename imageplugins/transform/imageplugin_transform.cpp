/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to transform image geometry.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_transform.moc"

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "perspectivetool.h"

using namespace DigikamTransformImagePlugin;

K_PLUGIN_FACTORY( TransformFactory, registerPlugin<ImagePlugin_Transform>(); )
K_EXPORT_PLUGIN ( TransformFactory("digikamimageplugin_transform") )

ImagePlugin_Transform::ImagePlugin_Transform(QObject* parent, const QVariantList&)
                     : ImagePlugin(parent, "ImagePlugin_Transform")
{
    m_perspectiveAction  = new KAction(KIcon("perspective"), i18n("Perspective Adjustment..."), this);
    actionCollection()->addAction("imageplugin_perspective", m_perspectiveAction );
    connect(m_perspectiveAction, SIGNAL(triggered(bool)),
            this, SLOT(slotPerspective()));

    setXMLFile("digikamimageplugin_transform_ui.rc");

    kDebug() << "ImagePlugin_Transform plugin loaded";
}

ImagePlugin_Transform::~ImagePlugin_Transform()
{
}

void ImagePlugin_Transform::setEnabledActions(bool b)
{
    m_perspectiveAction->setEnabled(b);
}

void ImagePlugin_Transform::slotPerspective()
{
    PerspectiveTool* tool = new PerspectiveTool(this);
    loadTool(tool);
}
