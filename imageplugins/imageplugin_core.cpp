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
#include <kcursor.h>
#include <kdebug.h>

// Local includes.

#include <imageiface.h>
#include "histogramviewer.h"
#include "imageeffect_rgb.h"
#include "imageeffect_hsl.h"
#include "imageeffect_bcg.h"
#include "imageeffect_bwsepia.h"
#include "imageeffect_redeye.h"
#include "imageeffect_blur.h"
#include "imageeffect_sharpen.h"
#include "imageeffect_colorsenhance.h"
#include "imageplugin_core.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_core,
                            KGenericFactory<ImagePlugin_Core>("digikam"));

ImagePlugin_Core::ImagePlugin_Core(QObject *parent, const char*,
                                   const QStringList &)
                : Digikam::ImagePlugin(parent, "ImagePlugin_Core")
{
    //-------------------------------                
    // Fix/Colors menu actions

    new KAction(i18n("Blur..."), 0, 
                this, SLOT(slotBlur()),
                actionCollection(), "implugcore_blur");
    
    new KAction(i18n("Sharpen..."), 0, 
                this, SLOT(slotSharpen()),
                actionCollection(), "implugcore_sharpen");

    m_redeyeAction = new KAction(i18n("Red Eye Reduction"), 0, 
                                 this, SLOT(slotRedEye()),
                                 actionCollection(), "implugcore_redeye");
    m_redeyeAction->setEnabled(false);
    
    new KAction(i18n("Brightness/Contrast/Gamma..."), 0, 
                this, SLOT(slotBCG()),
                actionCollection(), "implugcore_bcg");

    new KAction(i18n("Hue/Saturation/Lightness..."), 0, 
                this, SLOT(slotHSL()),
                actionCollection(), "implugcore_hsl");                

    new KAction(i18n("Color balance..."), 0, 
                this, SLOT(slotRGB()),
                actionCollection(), "implugcore_rgb");                

    new KAction(i18n("Normalize"), 0, 
                this, SLOT(slotNormalize()),
                actionCollection(), "implugcore_normalize");                

    new KAction(i18n("Equalize"), 0, 
                this, SLOT(slotEqualize()),
                actionCollection(), "implugcore_equalize");                

    new KAction(i18n("Auto levels"), 0, 
                this, SLOT(slotAutoLevels()),
                actionCollection(), "implugcore_autolevels");                

    //-------------------------------                
    // Image menu actions.
                                    
    new KAction(i18n("Histogram..."), 0, 
                this, SLOT(slotHistogramViewer()),
                actionCollection(), "implugcore_histogramviewer");

    //-------------------------------                
    // Filters menu actions.
                                    
    new KAction(i18n("Convert to Black-White"), 0, 
                this, SLOT(slotBW()),
                actionCollection(), "implugcore_bw");
    
    new KAction(i18n("Convert to Sepia"), 0, 
                this, SLOT(slotSepia()),
                actionCollection(), "implugcore_sepia");
    
    kdDebug() << "ImagePlugin_Core plugin loaded" << endl;
}

ImagePlugin_Core::~ImagePlugin_Core()
{
}

QStringList ImagePlugin_Core::guiDefinition() const
{
    QStringList guiDef;

    guiDef.append("MenuBar/Menu/&Image/Image/Action/implugcore_histogramviewer/ ");

    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_bcg/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_hsl/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_rgb/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Separator/ /  ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_normalize/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_equalize/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Menu/&Colors/Colors/Action/implugcore_autolevels/ ");
    
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_colors/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Separator/ / ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_blur/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_sharpen/ ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Separator/ / ");
    guiDef.append("MenuBar/Menu/Fi&x/Fix/Action/implugcore_redeye/ ");

    guiDef.append("MenuBar/Menu/Fi&lters/Generic/Action/implugcore_bw/ ");
    guiDef.append("MenuBar/Menu/Fi&lters/Generic/Action/implugcore_sepia/ ");

    // enable i18n

    i18n( "&Image" );
    i18n( "Fi&x" );
    i18n( "Fi&lters" );
    
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

void ImagePlugin_Core::slotNormalize()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_ColorsEnhance::normalizeImage();
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotEqualize()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_ColorsEnhance::equalizeImage();
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotAutoLevels()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_ColorsEnhance::autoLevelsCorrectionImage();
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotHistogramViewer()
{
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    
    HistogramViewer dlg(parentWidget(), data, w, h);
    dlg.exec();
    delete [] data;
}

void ImagePlugin_Core::slotBW()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_BWSepia::convertTOBW();    
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotSepia()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    ImageEffect_BWSepia::convertTOSepia();    
    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotRedEye()
{
    ImageEffect_RedEye::removeRedEye(parentWidget());    
}


#include "imageplugin_core.moc"
