/* ============================================================
 * File  : imageplugin_core.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-04
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kdebug.h>

#include "imageeffect_bcg.h"
#include "imageeffect_solarize.h"
#include "imageeffect_bwsepia.h"
#include "imageeffect_redeye.h"

#include "imageplugin_core.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_core,
                            KGenericFactory<ImagePlugin_Core>("digikam"));

ImagePlugin_Core::ImagePlugin_Core(QObject *parent, const char*,
                                   const QStringList &)
    : Digikam::ImagePlugin(parent, "ImagePlugin_Core")
{
    new KAction(i18n("Brightness/Contrast/Gamma..."), 0, 
                this, SLOT(slotBCG()),
                actionCollection(), "implugcore_bcg");
    new KAction(i18n("Convert to Black-White"), 0, 
                this, SLOT(slotBW()),
                actionCollection(), "implugcore_bw");
    new KAction(i18n("Convert to Sepia"), 0, 
                this, SLOT(slotSepia()),
                actionCollection(), "implugcore_sepia");
    new KAction(i18n("Solarize Image..."), 0, 
                this, SLOT(slotSolarize()),
                actionCollection(), "implugcore_solarize");
    
    m_redeyeAction = new KAction(i18n("Red Eye Reduction"), 0, 
                                 this, SLOT(slotRedEye()),
                                 actionCollection(), "implugcore_redeye");
    m_redeyeAction->setEnabled(false);

    kdDebug() << "ImagePlugin_Core plugin loaded" << endl;
}

ImagePlugin_Core::~ImagePlugin_Core()
{
}

QStringList ImagePlugin_Core::guiDefinition() const
{
    QStringList guiDef;

    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_bcg/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_bw/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_sepia/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_solarize/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_redeye/ ");

    return guiDef;
}

void ImagePlugin_Core::setEnabledSelectionActions(bool enable)
{
    m_redeyeAction->setEnabled(enable);
}    

void ImagePlugin_Core::slotBCG()
{
    ImageEffect_BCG dlg;
    dlg.exec();
}

void ImagePlugin_Core::slotSolarize()
{
    ImageEffect_Solarize dlg;
    dlg.exec();
}

void ImagePlugin_Core::slotBW()
{
    ImageEffect_BWSepia::convertTOBW();    
}

void ImagePlugin_Core::slotSepia()
{
    ImageEffect_BWSepia::convertTOSepia();    
}

void ImagePlugin_Core::slotRedEye()
{
    ImageEffect_RedEye::removeRedEye();    
}

#include "imageplugin_core.moc"
