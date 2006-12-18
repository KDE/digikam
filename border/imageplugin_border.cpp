/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-01-20
 * Description : 
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
#include "imageeffect_border.h"
#include "imageplugin_border.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_border,
                            KGenericFactory<ImagePlugin_Border>("digikamimageplugin_border"));

ImagePlugin_Border::ImagePlugin_Border(QObject *parent, const char*, const QStringList &)
                  : Digikam::ImagePlugin(parent, "ImagePlugin_Border")
{
    m_borderAction = new KAction(i18n("Add Border..."), "bordertool", 0, 
                         this, SLOT(slotBorder()),
                         actionCollection(), "imageplugin_border");

    setXMLFile("digikamimageplugin_border_ui.rc");
    
    DDebug() << "ImagePlugin_Border plugin loaded" << endl;
}

ImagePlugin_Border::~ImagePlugin_Border()
{
}

void ImagePlugin_Border::setEnabledActions(bool enable)
{
    m_borderAction->setEnabled(enable);
}

void ImagePlugin_Border::slotBorder()
{
    QString title = i18n("Add Border Around Photograph");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamBorderImagesPlugin::ImageEffect_Border dlg(parentWidget(),
                               title, headerFrame);
    dlg.exec();
    delete headerFrame;    
}

#include "imageplugin_border.moc"
