/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to change image perspective .
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_perspective.h"
#include "imageplugin_perspective.moc"

// KDE includes.

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>

// Local includes.

#include "perspectivetool.h"

using namespace DigikamPerspectiveImagesPlugin;

K_PLUGIN_FACTORY( PerspectiveFactory, registerPlugin<ImagePlugin_Perspective>(); )
K_EXPORT_PLUGIN ( PerspectiveFactory("digikamimageplugin_perspective") )

ImagePlugin_Perspective::ImagePlugin_Perspective(QObject *parent, const QVariantList&)
                       : Digikam::ImagePlugin(parent, "ImagePlugin_Perspective")
{
    m_perspectiveAction  = new KAction(KIcon("perspective"), i18n("Perspective Adjustment..."), this);
    actionCollection()->addAction("imageplugin_perspective", m_perspectiveAction );

    connect(m_perspectiveAction, SIGNAL(triggered(bool)),
            this, SLOT(slotPerspective()));

    setXMLFile("digikamimageplugin_perspective_ui.rc");

    kDebug(50006) << "ImagePlugin_Perspective plugin loaded" << endl;
}

ImagePlugin_Perspective::~ImagePlugin_Perspective()
{
}

void ImagePlugin_Perspective::setEnabledActions(bool enable)
{
    m_perspectiveAction->setEnabled(enable);
}

void ImagePlugin_Perspective::slotPerspective()
{
    PerspectiveTool *tool = new PerspectiveTool(this);
    loadTool(tool);
}
