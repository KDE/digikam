/* ============================================================
 * File  : imageplugin_unsharp.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-27
 * Description : 
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

// Local includes.

#include "bannerwidget.h"
#include "imageeffect_unsharp.h"
#include "imageplugin_unsharp.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_unsharp,
                            KGenericFactory<ImagePlugin_Unsharp>("digikamimageplugin_unsharp"));

ImagePlugin_Unsharp::ImagePlugin_Unsharp(QObject *parent, const char*,
                                         const QStringList &)
                   : Digikam::ImagePlugin(parent, "ImagePlugin_Unsharp")
{
    m_unsharpAction = new KAction(i18n("Unsharp Mask..."), "unsharpedmask", 0, 
                          this, SLOT(slotUnsharp()),
                          actionCollection(), "imageplugin_unsharp");

    setXMLFile("digikamimageplugin_unsharp_ui.rc");               
                    
    DDebug() << "ImagePlugin_Unsharp plugin loaded" << endl;
}

ImagePlugin_Unsharp::~ImagePlugin_Unsharp()
{
}

void ImagePlugin_Unsharp::setEnabledActions(bool enable)
{
    m_unsharpAction->setEnabled(enable);
}

void ImagePlugin_Unsharp::slotUnsharp()
{
    QString title = i18n("Unsharp Mask");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamUnsharpMaskImagesPlugin::ImageEffect_Unsharp dlg(parentWidget(),    
                                    title, headerFrame);
    dlg.exec();
    delete headerFrame;
}

#include "imageplugin_unsharp.moc"
