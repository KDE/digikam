/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin to emboss
 *               an image.
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


#include "imageplugin_emboss.h"
#include "imageplugin_emboss.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "embosstool.h"
#include "debug.h"

using namespace DigikamEmbossImagesPlugin;

K_PLUGIN_FACTORY( EmbossFactory, registerPlugin<ImagePlugin_Emboss>(); )
K_EXPORT_PLUGIN ( EmbossFactory("digikamimageplugin_emboss") )

ImagePlugin_Emboss::ImagePlugin_Emboss(QObject *parent, const QVariantList &)
                  : Digikam::ImagePlugin(parent, "ImagePlugin_Emboss")
{

    m_embossAction  = new KAction(KIcon("embosstool"), i18n("Emboss..."), this);
    actionCollection()->addAction("imageplugin_emboss", m_embossAction );

    connect(m_embossAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEmboss()));

    setXMLFile( "digikamimageplugin_emboss_ui.rc" );

    kDebug(imagePluginsAreaCode) << "ImagePlugin_Emboss plugin loaded";
}

ImagePlugin_Emboss::~ImagePlugin_Emboss()
{
}

void ImagePlugin_Emboss::setEnabledActions(bool enable)
{
    m_embossAction->setEnabled(enable);
}

void ImagePlugin_Emboss::slotEmboss()
{
    EmbossTool *tool = new EmbossTool(this);
    loadTool(tool);
}
