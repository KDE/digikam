/* ============================================================
 * File  : imageplugin_despeckle.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-24
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

// Digikam includes.

#include <imageiface.h>

// Local includes.

#include "despeckle.h"
#include "imageplugin_despeckle.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_despeckle,
                            KGenericFactory<ImagePlugin_Despeckle>("digikam"));

ImagePlugin_Despeckle::ImagePlugin_Despeckle(QObject *parent, const char*,
                                             const QStringList &)
                : Digikam::ImagePlugin(parent, "ImagePlugin_Despeckle")
{
    new KAction(i18n("Noise reduction..."), 0, 
                this, SLOT(slotDespeckle()),
                actionCollection(), "implugcore_despeckle");

    kdDebug() << "ImagePlugin_Despeckle plugin loaded" << endl;
}

ImagePlugin_Despeckle::~ImagePlugin_Despeckle()
{
}

QStringList ImagePlugin_Despeckle::guiDefinition() const
{
    QStringList guiDef;
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_despeckle/ ");
    return guiDef;
}

void ImagePlugin_Despeckle::slotDespeckle()
{
    DigikamDespeckleFilterImagesPlugin::DespeckleDialog dlg(parentWidget());
    dlg.exec();
}


#include "imageplugin_despeckle.moc"
