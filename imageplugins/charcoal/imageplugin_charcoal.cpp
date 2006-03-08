/* ============================================================
 * File  : imageplugin_charcoal.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-26
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
#include <kdebug.h>

// Local includes.

#include "bannerwidget.h"
#include "imageeffect_charcoal.h"
#include "imageplugin_charcoal.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_charcoal,
                            KGenericFactory<ImagePlugin_Charcoal>("digikamimageplugin_charcoal"));

ImagePlugin_Charcoal::ImagePlugin_Charcoal(QObject *parent, const char*,
                                           const QStringList &)
                    : Digikam::ImagePlugin(parent, "ImagePlugin_Charcoal")
{
    m_charcoalAction = new KAction(i18n("Charcoal Drawing..."), "charcoaltool", 0, 
                           this, SLOT(slotCharcoal()),
                           actionCollection(), "imageplugin_charcoal");
                
    setXMLFile( "digikamimageplugin_charcoal_ui.rc" );
    
    kdDebug() << "ImagePlugin_Charcoal plugin loaded" << endl;
}

ImagePlugin_Charcoal::~ImagePlugin_Charcoal()
{
}

void ImagePlugin_Charcoal::setEnabledActions(bool enable)
{
    m_charcoalAction->setEnabled(enable);
}

void ImagePlugin_Charcoal::slotCharcoal()
{
    QString title = i18n("Charcoal Drawing");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamCharcoalImagesPlugin::ImageEffect_Charcoal dlg(parentWidget(),
                                 title, headerFrame);
    dlg.exec();
    delete headerFrame;
}


#include "imageplugin_charcoal.moc"
