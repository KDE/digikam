/* ============================================================
 * File  : imageplugin_lensdistortion.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-27
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
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
#include <kcursor.h>
#include <kdebug.h>

// Local includes.

#include "imageeffect_lensdistortion.h"
#include "imageplugin_lensdistortion.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_lensdistortion,
                            KGenericFactory<ImagePlugin_LensDistortion>("digikamimageplugin_lensdistortion"));

ImagePlugin_LensDistortion::ImagePlugin_LensDistortion(QObject *parent, const char*, const QStringList &)
                            : Digikam::ImagePlugin(parent, "ImagePlugin_LensDistortion")
{
    new KAction(i18n("Lens Distortion Correction..."), 0, 
                this, SLOT(slotLensDistortion()),
                actionCollection(), "imageplugin_lensdistortion");
                
    
    kdDebug() << "ImagePlugin_LensDistortion plugin loaded" << endl;
}

ImagePlugin_LensDistortion::~ImagePlugin_LensDistortion()
{
}

QStringList ImagePlugin_LensDistortion::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/Fi&x/ /Action/imageplugin_lensdistortion/ ");
    return guiDef;
}

void ImagePlugin_LensDistortion::slotLensDistortion()
{
    DigikamLensDistortionImagesPlugin::ImageEffect_LensDistortion dlg(parentWidget());
    dlg.exec();
}


#include "imageplugin_lensdistortion.moc"
