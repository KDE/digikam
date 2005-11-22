/* ============================================================
 * File  : imageplugin_oilpaint.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-25
 * Description : 
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#include "imageeffect_oilpaint.h"
#include "imageplugin_oilpaint.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_oilpaint,
                            KGenericFactory<ImagePlugin_OilPaint>("digikamimageplugin_oilpaint"));

ImagePlugin_OilPaint::ImagePlugin_OilPaint(QObject *parent, const char*,
                                                   const QStringList &)
                    : Digikam::ImagePlugin(parent, "ImagePlugin_OilPaint")
{
    m_oilpaintAction = new KAction(i18n("Oil Paint..."), "oilpaint", 0, 
                       this, SLOT(slotOilPaint()),
                       actionCollection(), "imageplugin_oilpaint");
                    
    setXMLFile( "digikamimageplugin_oilpaint_ui.rc" );          
    
    kdDebug() << "ImagePlugin_OilPaint plugin loaded" << endl;
}

ImagePlugin_OilPaint::~ImagePlugin_OilPaint()
{
}

void ImagePlugin_OilPaint::setEnabledActions(bool enable)
{
    m_oilpaintAction->setEnabled(enable);
}

void ImagePlugin_OilPaint::slotOilPaint()
{
    DigikamOilPaintImagesPlugin::ImageEffect_OilPaint dlg(parentWidget());
    dlg.exec();
}


#include "imageplugin_oilpaint.moc"
