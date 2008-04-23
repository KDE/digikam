/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-25
 * Description : a digiKam image plugin to reduce 
 *               vignetting on an image.
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
#include "imageeffect_antivignetting.h"
#include "imageplugin_antivignetting.h"
#include "imageplugin_antivignetting.moc"

K_PLUGIN_FACTORY( AntiVignettingFactory, registerPlugin<ImagePlugin_AntiVignetting>(); )
K_EXPORT_PLUGIN ( AntiVignettingFactory("digikamimageplugin_antivignetting") )

ImagePlugin_AntiVignetting::ImagePlugin_AntiVignetting(QObject *parent, const QVariantList &)
                          : Digikam::ImagePlugin(parent, "ImagePlugin_AntiVignetting")
{
    m_antivignettingAction  = new KAction(KIcon("antivignetting"), i18n("Vignetting..."), this);
    actionCollection()->addAction("imageplugin_antivignetting", m_antivignettingAction );

    connect(m_antivignettingAction, SIGNAL(triggered(bool)), 
            this, SLOT(slotAntiVignetting()));

    setXMLFile("digikamimageplugin_antivignetting_ui.rc");                
    
    DDebug() << "ImagePlugin_AntiVignetting plugin loaded" << endl;
}

ImagePlugin_AntiVignetting::~ImagePlugin_AntiVignetting()
{
}

void ImagePlugin_AntiVignetting::setEnabledActions(bool enable)
{
    m_antivignettingAction->setEnabled(enable);
}

void ImagePlugin_AntiVignetting::slotAntiVignetting()
{
    DigikamAntiVignettingImagesPlugin::ImageEffect_AntiVignetting dlg(parentWidget());
    dlg.exec();
}
