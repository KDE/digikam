/* ============================================================
 * File  : imageplugin_adjustlevels.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-06-04
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

#include "adjustlevels.h"
#include "imageplugin_adjustlevels.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_adjustlevels,
                            KGenericFactory<ImagePlugin_AdjustLevels>("digikamimageplugin_adjustlevels"));

ImagePlugin_AdjustLevels::ImagePlugin_AdjustLevels(QObject *parent, const char*,
                                                   const QStringList &)
                : Digikam::ImagePlugin(parent, "ImagePlugin_AdjustLevels")
{
    new KAction(i18n("Levels Adjust..."), 0, 
                this, SLOT(slotLevelsAdjust()),
                actionCollection(), "imageplugin_adjustlevels");

    kdDebug() << "ImagePlugin_AdjustLevels plugin loaded" << endl;
}

ImagePlugin_AdjustLevels::~ImagePlugin_AdjustLevels()
{
}

QStringList ImagePlugin_AdjustLevels::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/imageplugin_adjustlevels/ ");
    return guiDef;
}

void ImagePlugin_AdjustLevels::slotLevelsAdjust()
{
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    
    DigikamAdjustLevelsImagesPlugin::AdjustLevelDialog dlg(parentWidget(), data, w, h);
    dlg.exec();
    delete [] data;
}


#include "imageplugin_adjustlevels.moc"
