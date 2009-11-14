/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-23
 * Description : a plugin to shear an image
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


#include "imageplugin_sheartool.h"
#include "imageplugin_sheartool.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes

#include "sheartool.h"

using namespace DigikamShearToolImagesPlugin;

K_PLUGIN_FACTORY( ShearToolFactory, registerPlugin<ImagePlugin_ShearTool>(); )
K_EXPORT_PLUGIN ( ShearToolFactory("digikamimageplugin_sheartool") )

ImagePlugin_ShearTool::ImagePlugin_ShearTool(QObject *parent, const QVariantList &)
                     : Digikam::ImagePlugin(parent, "ImagePlugin_ShearTool")
{
    m_sheartoolAction  = new KAction(KIcon("shear"), i18n("Shear..."), this);
    actionCollection()->addAction("imageplugin_sheartool", m_sheartoolAction );

    connect(m_sheartoolAction, SIGNAL(triggered(bool)),
            this, SLOT(slotShearTool()));

    setXMLFile("digikamimageplugin_sheartool_ui.rc");

    kDebug() << "ImagePlugin_ShearTool plugin loaded";
}

ImagePlugin_ShearTool::~ImagePlugin_ShearTool()
{
}

void ImagePlugin_ShearTool::setEnabledActions(bool enable)
{
    m_sheartoolAction->setEnabled(enable);
}

void ImagePlugin_ShearTool::slotShearTool()
{
    ShearTool *tool = new ShearTool(this);
    loadTool(tool);
}
