/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-27
 * Description : a plugin to reduce lens distorsions to an image.
 * 
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "imageeffect_lensdistortion.h"
#include "imageplugin_lensdistortion.h"
#include "imageplugin_lensdistortion.moc"

K_PLUGIN_FACTORY( LensDistortionFactory, registerPlugin<ImagePlugin_LensDistortion>(); )
K_EXPORT_PLUGIN ( LensDistortionFactory("digikamimageplugin_lensdistortion") )

ImagePlugin_LensDistortion::ImagePlugin_LensDistortion(QObject *parent, const QVariantList &)
                          : Digikam::ImagePlugin(parent, "ImagePlugin_LensDistortion")
{
    m_lensdistortionAction  = new KAction(KIcon("embosstool"), i18n("Lens Distortion..."), this);
    actionCollection()->addAction("imageplugin_lensdistortion", m_lensdistortionAction );

    connect(m_lensdistortionAction, SIGNAL(triggered(bool)), 
            this, SLOT(slotLensDistortion()));
 
    setXMLFile("digikamimageplugin_lensdistortion_ui.rc");            
        
    DDebug() << "ImagePlugin_LensDistortion plugin loaded" << endl;
}

ImagePlugin_LensDistortion::~ImagePlugin_LensDistortion()
{
}

void ImagePlugin_LensDistortion::setEnabledActions(bool enable)
{
    m_lensdistortionAction->setEnabled(enable);
}

void ImagePlugin_LensDistortion::slotLensDistortion()
{
    DigikamLensDistortionImagesPlugin::ImageEffect_LensDistortion dlg(parentWidget());
    dlg.exec();
}
