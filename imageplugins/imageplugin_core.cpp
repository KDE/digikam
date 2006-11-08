/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-06-04
 * Description : digiKam image editor plugin core
 *
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

#include <config.h>

// KDE includes.

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kcursor.h>
#include <kmessagebox.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "imageiface.h"
#include "imageeffect_rgb.h"
#include "imageeffect_hsl.h"
#include "imageeffect_bcg.h"
#include "imageeffect_bwsepia.h"
#include "imageeffect_redeye.h"
#include "imageeffect_blur.h"
#include "imageeffect_sharpen.h"
#include "imageeffect_ratiocrop.h"
#include "imageeffect_autocorrection.h"
#include "imageeffect_iccproof.h"
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

    m_autoCorrectionAction = new KAction(i18n("Auto-Correction..."), "autocorrection", 0,
                                 this, SLOT(slotAutoCorrection()),
                                 actionCollection(), "implugcore_autocorrection");

    m_invertAction = new KAction(i18n("Invert"), "invertimage", 0,
                         this, SLOT(slotInvert()),
                         actionCollection(), "implugcore_invert");
    
    m_convertTo8Bits = new KAction(i18n("8 bits"), "depth16to8", 0,
                           this, SLOT(slotConvertTo8Bits()),
                           actionCollection(), "implugcore_convertto8bits");

    m_convertTo16Bits = new KAction(i18n("16 bits"), "depth8to16", 0,
                            this, SLOT(slotConvertTo16Bits()),
                            actionCollection(), "implugcore_convertto16bits");

    m_colorManagementAction = new KAction(i18n("Color Management..."), "colormanagement", 0,
                                          this, SLOT(slotColorManagement()),
                                          actionCollection(), "implugcore_colormanagement");
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

    DDebug() << "ImagePlugin_Core plugin loaded" << endl;
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
    m_autoCorrectionAction->setEnabled(enable);
    m_invertAction->setEnabled(enable);
    m_BWAction->setEnabled(enable);
    m_aspectRatioCropAction->setEnabled(enable);
    m_sharpenAction->setEnabled(enable);
    m_blurAction->setEnabled(enable);
    m_colorManagementAction->setEnabled(enable);
    m_convertTo8Bits->setEnabled(enable);
    m_convertTo16Bits->setEnabled(enable);
}

void ImagePlugin_Core::slotBlur()
{
    DigikamImagesPluginCore::ImageEffect_Blur dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotSharpen()
{
    DigikamImagesPluginCore::ImageEffect_Sharpen dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotBCG()
{
    DigikamImagesPluginCore::ImageEffect_BCG dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotRGB()
{
    DigikamImagesPluginCore::ImageEffect_RGB dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotHSL()
{
    DigikamImagesPluginCore::ImageEffect_HSL dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotAutoCorrection()
{
    DigikamImagesPluginCore::ImageEffect_AutoCorrection dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotInvert()
{
    parentWidget()->setCursor( KCursor::waitCursor() );
        
    Digikam::ImageIface iface(0, 0);

    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();

    Digikam::DImgImageFilters filter;
    filter.invertImage(data, w, h, sixteenBit);
    iface.putOriginalImage(i18n("Invert"), data);
    delete data;

    parentWidget()->unsetCursor();
}

void ImagePlugin_Core::slotBW()
{
    DigikamImagesPluginCore::ImageEffect_BWSepia dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotRedEye()
{
    DigikamImagesPluginCore::ImageEffect_RedEye::removeRedEye(parentWidget());
}

void ImagePlugin_Core::slotRatioCrop()
{
    DigikamImagesPluginCore::ImageEffect_RatioCrop dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotColorManagement()
{
    DigikamImagesPluginCore::ImageEffect_ICCProof dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotConvertTo8Bits()
{
    Digikam::ImageIface iface(0, 0);

    if (!iface.originalSixteenBit())
    {
       KMessageBox::error(parentWidget(), i18n("This picture is already using a depth of 8 bits / color / pixel."));
       return;
    }
    else
    {
       if (KMessageBox::warningContinueCancel(parentWidget(),
                                              i18n("Performing this operation will reduce image color quality. "
                                                   "Do you want to continue?")) == KMessageBox::Cancel)
           return;
    }
    
    parentWidget()->setCursor( KCursor::waitCursor() );
    iface.convertOriginalColorDepth(32);
    parentWidget()->unsetCursor();
}

void ImagePlugin_Core::slotConvertTo16Bits()
{
    Digikam::ImageIface iface(0, 0);
    
    if (iface.originalSixteenBit())
    {
       KMessageBox::error(parentWidget(), i18n("This picture is already using a depth of 16 bits / color / pixel."));
       return;
    }
    
    parentWidget()->setCursor( KCursor::waitCursor() );
    iface.convertOriginalColorDepth(64);
    parentWidget()->unsetCursor();
}

#include "imageplugin_core.moc"
