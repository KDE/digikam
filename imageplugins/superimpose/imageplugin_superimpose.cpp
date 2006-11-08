/* ============================================================
 * File  : imageplugin_superimpose.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-01-04
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

// Local includes.

#include "bannerwidget.h"
#include "imageeffect_superimpose.h"
#include "imageplugin_superimpose.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_superimpose,
                            KGenericFactory<ImagePlugin_SuperImpose>("digikamimageplugin_superimpose"));

ImagePlugin_SuperImpose::ImagePlugin_SuperImpose(QObject *parent, const char*, const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_SuperImpose")
{
    m_superimposeAction = new KAction(i18n("Template Superimpose..."), "superimpose", 0, 
                              this, SLOT(slotSuperImpose()),
                              actionCollection(), "imageplugin_superimpose");

    setXMLFile("digikamimageplugin_superimpose_ui.rc");        
                                    
    DDebug() << "ImagePlugin_SuperImpose plugin loaded" << endl;
}

ImagePlugin_SuperImpose::~ImagePlugin_SuperImpose()
{
}

void ImagePlugin_SuperImpose::setEnabledActions(bool enable)
{
    m_superimposeAction->setEnabled(enable);
}

void ImagePlugin_SuperImpose::slotSuperImpose()
{
    QString title = i18n("Template Superimpose to Photograph");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamSuperImposeImagesPlugin::ImageEffect_SuperImpose dlg(parentWidget(),
                                    title, headerFrame);
    dlg.exec();
    delete headerFrame;
}

#include "imageplugin_superimpose.moc"
