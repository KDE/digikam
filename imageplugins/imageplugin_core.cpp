/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-06-04
 * Description :
 *
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
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

#include <imageiface.h>
#include <imagefilters.h>
#include "imageeffect_rgb.h"
#include "imageeffect_hsl.h"
#include "imageeffect_bcg.h"
#include "imageeffect_bwsepia.h"
#include "imageeffect_redeye.h"
#include "imageeffect_blur.h"
#include "imageeffect_sharpen.h"
#include "imageeffect_ratiocrop.h"
#include "imageplugin_core.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_core,
                            KGenericFactory<ImagePlugin_Core>("digikam"));

ImagePlugin_Core::ImagePlugin_Core(QObject *parent, const char*,
                                   const QStringList &)
                : Digikam::ImagePlugin(parent, "ImagePlugin_Core")
{
    //-------------------------------
    // Fix and Colors menu actions

    m_blurAction = new KAction(i18n("Blur..."), "blurimage", 0,
                       this, SLOT(slotBlur()),
                       actionCollection(), "implugcore_blur");

    m_sharpenAction = new KAction(i18n("Sharpen..."), "sharpenimage", 0,
                          this, SLOT(slotSharpen()),
                          actionCollection(), "implugcore_sharpen");

    m_redeyeAction = new KAction(i18n("Red Eye Reduction..."), "redeyes", 0,
                                 this, SLOT(slotRedEye()),
                                 actionCollection(), "implugcore_redeye");
    m_redeyeAction->setWhatsThis( i18n( "This filter can be used to correct red eyes in a photo. "
                                        "Select a region including the eyes to use this option.") );

    m_BCGAction = new KAction(i18n("Brightness/Contrast/Gamma..."), "contrast", 0,
                      this, SLOT(slotBCG()),
                      actionCollection(), "implugcore_bcg");

    m_HSLAction = new KAction(i18n("Hue/Saturation/Lightness..."), "adjusthsl", 0,
                      this, SLOT(slotHSL()),
                      actionCollection(), "implugcore_hsl");

    m_RGBAction = new KAction(i18n("Color Balance..."), "adjustrgb", 0,
                      this, SLOT(slotRGB()),
                      actionCollection(), "implugcore_rgb");

    m_stretchContrastAction = new KAction(i18n("Stretch Contrast"), "stretchcontrast", 0,
                                  this, SLOT(slotNormalize()),
                                  actionCollection(), "implugcore_stretch_contrast");
    m_stretchContrastAction->setWhatsThis( i18n( "This option enhances the contrast and brightness "
                                                 "of the RGB values of an image by stretching the lowest "
                                                 "and highest values to their fullest range, adjusting "
                                                 "everything in between." ));                

    
    m_normalizeAction = new KAction(i18n("Normalize"), "normalize", 0,
                            this, SLOT(slotNormalize()),
                            actionCollection(), "implugcore_normalize");
    m_normalizeAction->setWhatsThis( i18n( "This option scales brightness values across the active "
                                           "image so that the darkest point becomes black, and the "
                                           "brightest point becomes as bright as possible without "
                                           "altering its hue. This is often a \"magic fix\" for "
                                           "images that are dim or washed out."));                

    m_equalizeAction = new KAction(i18n("Equalize"), "equalize", 0,
                           this, SLOT(slotEqualize()),
                           actionCollection(), "implugcore_equalize");
    m_equalizeAction->setWhatsThis( i18n( "This option adjusts the brightness of colors across the "
                                          "active image so that the histogram for the Value channel "
                                          "is as nearly as possible flat, that is, so that each possible "
                                          "brightness value appears at about the same number of pixels "
                                          "as each other value. Sometimes Equalize works wonderfully at "
                                          "enhancing the contrasts in an image. Other times it gives "
                                          "garbage. It is a very powerful operation, which can either work "
                                          "miracles on an image or destroy it."));  
                                         
    m_autolevelseAction = new KAction(i18n("Auto Levels"), "autolevels", 0,
                              this, SLOT(slotAutoLevels()),
                              actionCollection(), "implugcore_autolevels");
    m_autolevelseAction->setWhatsThis( i18n( "This option maximizes the tonal range in the Red, Green, "
                                             "and Blue channels. It search the image shadow and highlight "
                                             "limit values and adjust the Red, Green, and Blue channels "
                                             "to a full histogram range."));  

    m_invertAction = new KAction(i18n("Invert"), "invertimage", 0,
                         this, SLOT(slotInvert()),
                         actionCollection(), "implugcore_invert");
                                                           
    //-------------------------------
    // Filters menu actions.

    m_BWAction = new KAction(i18n("Black && White..."), "bwtonal", 0,
                     this, SLOT(slotBW()),
                     actionCollection(), "implugcore_blackwhite");

    //-------------------------------
    // Transform menu actions.
    
    m_aspectRatioCropAction = new KAction(i18n("Aspect Ratio Crop..."), "ratiocrop", 0,
                                  this, SLOT(slotRatioCrop()),
                                  actionCollection(), "implugcore_ratiocrop");

    //-------------------------------
    // Init. menu actions.

    setXMLFile("digikamimageplugin_core_ui.rc");

    kdDebug() << "ImagePlugin_Core plugin loaded" << endl;
}

ImagePlugin_Core::~ImagePlugin_Core()
{
}

void ImagePlugin_Core::setEnabledSelectionActions(bool)
{
}

void ImagePlugin_Core::setEnabledActions(bool enable)
{
    m_redeyeAction->setEnabled(enable);
    m_BCGAction->setEnabled(enable);
    m_HSLAction->setEnabled(enable);
    m_RGBAction->setEnabled(enable);
    m_stretchContrastAction->setEnabled(enable);
    m_normalizeAction->setEnabled(enable);
    m_equalizeAction->setEnabled(enable);
    m_autolevelseAction->setEnabled(enable);
    m_invertAction->setEnabled(enable);
    m_BWAction->setEnabled(enable);
    m_aspectRatioCropAction->setEnabled(enable);
    m_sharpenAction->setEnabled(enable);
    m_blurAction->setEnabled(enable);
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

void ImagePlugin_Core::slotStretchContrast()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth(); 
    int   h    = iface.originalHeight();
    
    Digikam::ImageFilters::stretchContrastImage(data, w, h);
    
    iface.putOriginalData(i18n("Stretch Contrast"), data);
    delete [] data;

    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotNormalize()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
    
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth(); 
    int   h    = iface.originalHeight();
    
    Digikam::ImageFilters::normalizeImage(data, w, h);
    
    iface.putOriginalData(i18n("Normalize"), data);
    delete [] data;

    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotEqualize()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
        
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth(); 
    int   h    = iface.originalHeight();
    
    Digikam::ImageFilters::equalizeImage(data, w, h);
    
    iface.putOriginalData(i18n("Equalize"), data);
    delete [] data;

    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotAutoLevels()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
        
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth(); 
    int   h    = iface.originalHeight();
    
    Digikam::ImageFilters::autoLevelsCorrectionImage(data, w, h);
    
    iface.putOriginalData(i18n("Auto Levels"), data);
    delete [] data;

    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotInvert()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
        
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int   w    = iface.originalWidth(); 
    int   h    = iface.originalHeight();
    
    Digikam::ImageFilters::invertImage(data, w, h);
    
    iface.putOriginalData(i18n("Invert"), data);
    delete [] data;

    parentWidget()->setCursor( KCursor::arrowCursor()  );
}

void ImagePlugin_Core::slotBW()
{
    ImageEffect_BWSepia dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotRedEye()
{
    ImageEffect_RedEye::removeRedEye(parentWidget());
}

void ImagePlugin_Core::slotRatioCrop()
{
    ImageEffect_RatioCrop dlg(parentWidget());
    dlg.exec();
}

#include "imageplugin_core.moc"
