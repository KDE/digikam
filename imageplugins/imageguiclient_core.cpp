/* ============================================================
 * File  : imageguiclient_core.cpp
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

#include <kaction.h>
#include <klocale.h>

#include "imageeffect_bcg.h"
#include "imageeffect_solarize.h"
#include "imageeffect_bwsepia.h"
#include "imageeffect_redeye.h"

#include "imageguiclient_core.h"

ImageGUIClient_Core::ImageGUIClient_Core()
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
    new KAction(i18n("Red Eye Reduction"), 0, 
                this, SLOT(slotRedEye()),
                actionCollection(), "implugcore_redeye");
}

ImageGUIClient_Core::~ImageGUIClient_Core()
{
    
}

QStringList ImageGUIClient_Core::guiDefinition() const
{
    QStringList guiDef;

    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_bcg/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_bw/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_sepia/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_solarize/ ");
    guiDef.append("MenuBar/Menu/&Filters/Generic/Action/implugcore_redeye/ ");

    return guiDef;
}

void ImageGUIClient_Core::slotBCG()
{
    ImageEffect_BCG dlg;
    dlg.exec();
}

void ImageGUIClient_Core::slotSolarize()
{
    ImageEffect_Solarize dlg;
    dlg.exec();
}

void ImageGUIClient_Core::slotBW()
{
    ImageEffect_BWSepia::convertTOBW();    
}

void ImageGUIClient_Core::slotSepia()
{
    ImageEffect_BWSepia::convertTOSepia();    
}

void ImageGUIClient_Core::slotRedEye()
{
    ImageEffect_RedEye::removeRedEye();    
}

#include "imageguiclient_core.moc"
