/* ============================================================
 * File  : imageplugin_filmgrain.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-10-01
 * Description : 
 * 
 * Copyright 2004-2005 by Gilles Caulier
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
#include "imageeffect_filmgrain.h"
#include "imageplugin_filmgrain.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_filmgrain,
                            KGenericFactory<ImagePlugin_FilmGrain>("digikamimageplugin_filmgrain"));

ImagePlugin_FilmGrain::ImagePlugin_FilmGrain(QObject *parent, const char*, const QStringList &)
                     : Digikam::ImagePlugin(parent, "ImagePlugin_FilmGrain")
{
    m_filmgrainAction = new KAction(i18n("Add Film Grain..."), "filmgrain", 0, 
                            this, SLOT(slotFilmGrain()),
                            actionCollection(), "imageplugin_filmgrain");
                
    setXMLFile( "digikamimageplugin_filmgrain_ui.rc" );                                
    
    DDebug() << "ImagePlugin_FilmGrain plugin loaded" << endl;
}

ImagePlugin_FilmGrain::~ImagePlugin_FilmGrain()
{
}

void ImagePlugin_FilmGrain::setEnabledActions(bool enable)
{
    m_filmgrainAction->setEnabled(enable);
}

void ImagePlugin_FilmGrain::slotFilmGrain()
{
    QString title = i18n("Add Film Grain to Photograph");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamFilmGrainImagesPlugin::ImageEffect_FilmGrain dlg(parentWidget(),
            title, headerFrame);
    dlg.exec();
    delete headerFrame;
}


#include "imageplugin_filmgrain.moc"
