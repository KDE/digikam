/* ============================================================
 * File  : imageplugin_oilpaint.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-25
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

#include "imageeffect_oilpaint.h"
#include "imageplugin_oilpaint.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_oilpaint,
                            KGenericFactory<ImagePlugin_OilPaint>("digikam"));

ImagePlugin_OilPaint::ImagePlugin_OilPaint(QObject *parent, const char*,
                                                   const QStringList &)
                    : Digikam::ImagePlugin(parent, "ImagePlugin_OilPaint")
{
    new KAction(i18n("Oil paint..."), 0, 
                this, SLOT(slotTest()),
                actionCollection(), "implugcore_oilpaint");
                
    
    kdDebug() << "ImagePlugin_OilPaint plugin loaded" << endl;
}

ImagePlugin_OilPaint::~ImagePlugin_OilPaint()
{
}

QStringList ImagePlugin_OilPaint::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/Fi&lters/Generic/Action/implugcore_oilpaint/ ");
    return guiDef;
}

void ImagePlugin_OilPaint::slotTest()
{
    DigikamOilPaintImagesPlugin::ImageEffect_OilPaint dlg(parentWidget());
    dlg.exec();
}


#include "imageplugin_oilpaint.moc"
