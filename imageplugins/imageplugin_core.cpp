/* ============================================================
 * File  : imageplugin_core.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-06-04
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju and Gilles Caulier
 *
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

// KDE includes.
  
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kdebug.h>

// Local includes.

#include "imageeffect_rgb.h"
#include "imageeffect_hsl.h"
#include "imageeffect_bcg.h"
#include "imageeffect_solarize.h"
#include "imageeffect_bwsepia.h"
#include "imageeffect_redeye.h"
#include "imageeffect_blur.h"
#include "imageeffect_sharpen.h"
#include "imageplugin_core.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_core,
                            KGenericFactory<ImagePlugin_Core>("digikam"));

ImagePlugin_Core::ImagePlugin_Core(QObject *parent, const char*,
                                   const QStringList &)
    : Digikam::ImagePlugin(parent, "ImagePlugin_Core")
{
    new KAction(i18n("Blur..."), 0, 
                this, SLOT(slotBlur()),
                actionCollection(), "implugcore_blur");
    
    new KAction(i18n("Sharpen..."), 0, 
                this, SLOT(slotSharpen()),
                actionCollection(), "implugcore_sharpen");

    // Fix/Colors menu actions
    
    m_colorsAction = new KActionMenu(i18n("&Colors"), "blend",
                                     actionCollection(),
                                     "implugcore_colors");
    m_colorsAction->setDelayed(false);

    m_colorsAction->insert( 
                new KAction(i18n("Brightness/Contrast/Gamma..."), 0, 
                this, SLOT(slotBCG()),
                actionCollection(), "implugcore_bcg") );
                        
    m_colorsAction->insert(
                new KAction(i18n("Hue/Saturation/Lightness..."), 0, 
                this, SLOT(slotHSL()),
                actionCollection(), "implugcore_hsl") );
    
    m_colorsAction->insert(
                new KAction(i18n("Color balance..."), 0, 
                this, SLOT(slotRGB()),
                actionCollection(), "implugcore_rgb") );

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

    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_bw/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_sepia/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_solarize/ ");

    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_colors/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Separator/ / ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_blur/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_sharpen/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Separator/ / ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_redeye/ ");

    // enable i18n

    i18n( "&Filters" );
    i18n( "Fi&x" );
    
    return guiDef;
}

void ImagePlugin_Core::setEnabledSelectionActions(bool enable)
{
    m_redeyeAction->setEnabled(enable);
}    

void ImagePlugin_Core::slotBlur()
{
    ImageEffect_Blur dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotSharpen()
{
    ImageEffect_Sharpen dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotBCG()
{
    ImageEffect_BCG dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotRGB()
{
    ImageEffect_RGB dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotHSL()
{
    ImageEffect_HSL dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotSolarize()
{
    ImageEffect_Solarize dlg(parentWidget());
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
