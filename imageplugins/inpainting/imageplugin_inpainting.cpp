/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint
 *               a photograph
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.
  
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>

// Local includes.

#include "ddebug.h"
#include "imageeffect_inpainting.h"
#include "imageplugin_inpainting.h"
#include "imageplugin_inpainting.moc"

K_PLUGIN_FACTORY( InPaintingFactory, registerPlugin<ImagePlugin_InPainting>(); )
K_EXPORT_PLUGIN ( InPaintingFactory("digikamimageplugin_inpainting") )

ImagePlugin_InPainting::ImagePlugin_InPainting(QObject *parent, const QVariantList &)
                      : Digikam::ImagePlugin(parent, "ImagePlugin_InPainting")
{
    m_inPaintingAction = new KAction(KIcon("inpainting"), i18n("Inpainting..."), this);
    m_inPaintingAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_E));
    m_inPaintingAction->setWhatsThis( i18n( "This filter can be used to inpaint a part in a photo. "
                                            "Select a region to inpaint to use this option.") );                

    connect(m_inPaintingAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotInPainting()));

    actionCollection()->addAction("imageplugin_inpainting", m_inPaintingAction );

    setXMLFile( "digikamimageplugin_inpainting_ui.rc" );                                
    
    DDebug() << "ImagePlugin_InPainting plugin loaded" << endl;
}

ImagePlugin_InPainting::~ImagePlugin_InPainting()
{
}

void ImagePlugin_InPainting::setEnabledActions(bool enable)
{
    m_inPaintingAction->setEnabled(enable);
}

void ImagePlugin_InPainting::slotInPainting()
{
    DigikamInPaintingImagesPlugin::ImageEffect_InPainting::inPainting(parentWidget());
}

