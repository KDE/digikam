/* ============================================================
 * File  : imageplugin_whitebalance.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-11
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

#include "imageeffect_whitebalance.h"
#include "imageplugin_whitebalance.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_whitebalance,
                            KGenericFactory<ImagePlugin_WhiteBalance>("digikamimageplugin_whitebalance"));

ImagePlugin_WhiteBalance::ImagePlugin_WhiteBalance(QObject *parent, const char*, const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_WhiteBalance")
{
    m_whitebalanceAction = new KAction(i18n("White Balance..."), "whitebalance", 0, 
                               this, SLOT(slotWhiteBalance()),
                               actionCollection(), "imageplugin_whitebalance");
    
    setXMLFile("digikamimageplugin_whitebalance_ui.rc");         
                                    
    kdDebug() << "ImagePlugin_WhiteBalance plugin loaded" << endl;
}

ImagePlugin_WhiteBalance::~ImagePlugin_WhiteBalance()
{
}

void ImagePlugin_WhiteBalance::setEnabledActions(bool enable)
{
    m_whitebalanceAction->setEnabled(enable);
}

void ImagePlugin_WhiteBalance::slotWhiteBalance()
{
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();

    DigikamWhiteBalanceImagesPlugin::ImageEffect_WhiteBalance dlg(parentWidget(), data, w, h);
    dlg.exec();
    delete [] data;
}


#include "imageplugin_whitebalance.moc"
