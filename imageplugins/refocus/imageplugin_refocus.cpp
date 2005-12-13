/* ============================================================
 * File  : imageplugin_refocus.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-29
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

#include "bannerwidget.h"
#include "imageeffect_refocus.h"
#include "imageplugin_refocus.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_refocus,
                            KGenericFactory<ImagePlugin_Refocus>("digikamimageplugin_refocus"));

ImagePlugin_Refocus::ImagePlugin_Refocus(QObject *parent, const char*, const QStringList &)
                   : Digikam::ImagePlugin(parent, "ImagePlugin_Refocus")
{
    m_refocusAction = new KAction(i18n("Refocus..."), "refocus", 0, 
                          this, SLOT(slotRefocus()),
                          actionCollection(), "imageplugin_refocus");
                
    setXMLFile( "digikamimageplugin_refocus_ui.rc" );                                
    
    kdDebug() << "ImagePlugin_Refocus plugin loaded" << endl;
}

ImagePlugin_Refocus::~ImagePlugin_Refocus()
{
}

void ImagePlugin_Refocus::setEnabledActions(bool enable)
{
    m_refocusAction->setEnabled(enable);
}

void ImagePlugin_Refocus::slotRefocus()
{
    QString title = i18n("Refocus Photograph");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamRefocusImagesPlugin::ImageEffect_Refocus dlg(parentWidget(),
                                title, headerFrame);
    dlg.exec();
    delete headerFrame;
}

#include "imageplugin_refocus.moc"
