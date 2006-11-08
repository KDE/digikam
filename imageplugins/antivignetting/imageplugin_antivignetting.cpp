/* ============================================================
 * File  : imageplugin_antivignetting.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-12-25
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
#include "imageeffect_antivignetting.h"
#include "imageplugin_antivignetting.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_antivignetting,
                            KGenericFactory<ImagePlugin_AntiVignetting>("digikamimageplugin_antivignetting"));

ImagePlugin_AntiVignetting::ImagePlugin_AntiVignetting(QObject *parent, const char*, const QStringList &)
                          : Digikam::ImagePlugin(parent, "ImagePlugin_AntiVignetting")
{
    m_antivignettingAction = new KAction(i18n("Anti Vignetting..."), "antivignetting", 0, 
                                 this, SLOT(slotAntiVignetting()),
                                 actionCollection(), "imageplugin_antivignetting");

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
    QString title = i18n("Anti-Vignetting");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamAntiVignettingImagesPlugin::ImageEffect_AntiVignetting dlg(parentWidget(),
                                title, headerFrame);
    dlg.exec();
    delete headerFrame;
}

#include "imageplugin_antivignetting.moc"
