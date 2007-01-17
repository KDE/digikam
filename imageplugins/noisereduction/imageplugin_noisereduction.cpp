/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-08-24
 * Description : 
 * 
 * Copyright 2004-2007 by Gilles Caulier
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
#include "imageeffect_noisereduction.h"
#include "imageplugin_noisereduction.h"
#include "imageplugin_noisereduction.moc"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_despeckle,
                            KGenericFactory<ImagePlugin_NoiseReduction>("digikamimageplugin_despeckle"));

ImagePlugin_NoiseReduction::ImagePlugin_NoiseReduction(QObject *parent, const char*, const QStringList &)
                          : Digikam::ImagePlugin(parent, "ImagePlugin_NoiseReduction")
{
    m_noiseReductionAction = new KAction(i18n("Noise Reduction..."), "despeckle", 0,
                             this, SLOT(slotNoiseReduction()),
                             actionCollection(), "imageplugin_despeckle");
                
    setXMLFile("digikamimageplugin_despeckle_ui.rc");                

    DDebug() << "ImagePlugin_NoiseReduction plugin loaded" << endl;
}

ImagePlugin_NoiseReduction::~ImagePlugin_NoiseReduction()
{
}

void ImagePlugin_NoiseReduction::setEnabledActions(bool enable)
{
    m_noiseReductionAction->setEnabled(enable);
}

void ImagePlugin_NoiseReduction::slotNoiseReduction()
{
    QString title = i18n("Noise Reduction");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamNoiseReductionImagesPlugin::ImageEffect_NoiseReduction dlg(parentWidget(),
                                       title, headerFrame);
    dlg.exec();
    delete headerFrame;
}


