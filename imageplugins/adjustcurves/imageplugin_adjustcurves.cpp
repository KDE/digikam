/* ============================================================
 * File  : imageplugin_adjustcurves.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
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

#include "adjustcurves.h"
#include "imageplugin_adjustcurves.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_adjustcurves,
                            KGenericFactory<ImagePlugin_AdjustCurves>("digikamimageplugin_adjustcurves"));

ImagePlugin_AdjustCurves::ImagePlugin_AdjustCurves(QObject *parent, const char*,
                                                   const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_AdjustCurves")
{
    new KAction(i18n("Curves Adjust..."), 0, 
                this, SLOT(slotCurvesAdjust()),
                actionCollection(), "imageplugin_adjustcurves");

    kdDebug() << "ImagePlugin_AdjustCurves plugin loaded" << endl;
}

ImagePlugin_AdjustCurves::~ImagePlugin_AdjustCurves()
{
}

QStringList ImagePlugin_AdjustCurves::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/Fi&x/ /Menu/&Colors/ /Action/imageplugin_adjustcurves/ ");
    return guiDef;
}

void ImagePlugin_AdjustCurves::slotCurvesAdjust()
{
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    
    DigikamAdjustCurvesImagesPlugin::AdjustCurveDialog dlg(parentWidget(), data, w, h);
    dlg.exec();
    delete [] data;
}


#include "imageplugin_adjustcurves.moc"
