/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin fix lens errors
 * 
 * Copyright (C) 2008 Adrian Schroeter <adrian@suse.de>
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
#include "imageeffect_lenscorrection.h"
#include "imageplugin_lenscorrection.h"
#include "imageplugin_lenscorrection.moc"

K_PLUGIN_FACTORY( LensCorrectionFactory, registerPlugin<ImagePlugin_LensCorrection>(); )
K_EXPORT_PLUGIN ( LensCorrectionFactory("digikamimageplugin_lenscorrection") )

ImagePlugin_LensCorrection::ImagePlugin_LensCorrection(QObject *parent, const QVariantList &)
                          : Digikam::ImagePlugin(parent, "ImagePlugin_LensCorrection")
{
    m_lensCorrectionAction  = new KAction(KIcon("embosstool"), i18n("Lens Correction..."), this);
    actionCollection()->addAction("imageplugin_lenscorrection", m_lensCorrectionAction );

    connect(m_lensCorrectionAction, SIGNAL(triggered(bool)), 
            this, SLOT(slotLensCorrection()));
 
    setXMLFile("digikamimageplugin_lenscorrection_ui.rc");            
        
    DDebug() << "ImagePlugin_LensCorrection plugin loaded" << endl;
}

ImagePlugin_LensCorrection::~ImagePlugin_LensCorrection()
{
}

void ImagePlugin_LensCorrection::setEnabledActions(bool enable)
{
    m_lensCorrectionAction->setEnabled(enable);
}

void ImagePlugin_LensCorrection::slotLensCorrection()
{
    DigikamLensCorrectionImagesPlugin::ImageEffect_LensCorrection dlg(parentWidget());
    dlg.exec();
}
