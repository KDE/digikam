/* ============================================================
 * File  : imageplugin_blowup.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-04-07
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
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

#include "imageeffect_blowup.h"
#include "imageplugin_blowup.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_blowup,
                            KGenericFactory<ImagePlugin_BlowUp>("digikamimageplugin_blowup"));

ImagePlugin_BlowUp::ImagePlugin_BlowUp(QObject *parent, const char*, const QStringList &)
                  : Digikam::ImagePlugin(parent, "ImagePlugin_BlowUp")
{
    m_inPaintingAction = new KAction(i18n("Blowup..."), "blowup", 0, 
                             this, SLOT(slotBlowUp()),
                             actionCollection(), "imageplugin_blowup");
                
    setXMLFile( "digikamimageplugin_blowup_ui.rc" );                                
    
    kdDebug() << "ImagePlugin_BlowUp plugin loaded" << endl;
}

ImagePlugin_BlowUp::~ImagePlugin_BlowUp()
{
}

void ImagePlugin_BlowUp::setEnabledActions(bool enable)
{
    m_inPaintingAction->setEnabled(enable);
}

void ImagePlugin_BlowUp::slotBlowUp()
{
    DigikamBlowUpImagesPlugin::ImageEffect_BlowUp dlg(parentWidget());
    dlg.exec();
}

#include "imageplugin_blowup.moc"
