/* ============================================================
 * File  : imageplugin_emboss.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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

#include "imageeffect_emboss.h"
#include "imageplugin_emboss.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_emboss,
                            KGenericFactory<ImagePlugin_Emboss>("digikam"));

ImagePlugin_Emboss::ImagePlugin_Emboss(QObject *parent, const char*,
                                                   const QStringList &)
                    : Digikam::ImagePlugin(parent, "ImagePlugin_Emboss")
{
    new KAction(i18n("Emboss..."), 0, 
                this, SLOT(slotEmboss()),
                actionCollection(), "implugcore_emboss");
                
    
    kdDebug() << "ImagePlugin_Emboss plugin loaded" << endl;
}

ImagePlugin_Emboss::~ImagePlugin_Emboss()
{
}

QStringList ImagePlugin_Emboss::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/Fi&lters/Generic/Action/implugcore_emboss/ ");
    return guiDef;
}

void ImagePlugin_Emboss::slotEmboss()
{
    DigikamEmbossImagesPlugin::ImageEffect_Emboss dlg(parentWidget());
    dlg.exec();
}


#include "imageplugin_emboss.moc"
