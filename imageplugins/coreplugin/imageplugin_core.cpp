/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : digiKam image editor plugin core
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <kapplication.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "imageiface.h"
#include "rgbtool.h"
#include "hsltool.h"
#include "bcgtool.h"
#include "bwsepiatool.h"
#include "redeyetool.h"
#include "blurtool.h"
#include "sharpentool.h"
#include "ratiocroptool.h"
#include "autocorrectiontool.h"
#include "iccprooftool.h"
#include "imageplugin_core.h"
#include "imageplugin_core.moc"

using namespace DigikamImagesPluginCore;
using namespace Digikam;

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_core,
                           KGenericFactory<ImagePlugin_Core>("digikam"));

ImagePlugin_Core::ImagePlugin_Core(QObject *parent, const char*, const QStringList&)
                : ImagePlugin(parent, "ImagePlugin_Core")
{
    //-------------------------------
    // Fix and Colors menu actions

    m_blurAction = new KAction(i18n("Blur..."), "blurimage", 0,
                       this, SLOT(slotBlur()),
                       actionCollection(), "implugcore_blur");

    m_sharpenAction = new KAction(i18n("Sharpen..."), "sharpenimage", 0,
                          this, SLOT(slotSharpen()),
                          actionCollection(), "implugcore_sharpen");

    m_redeyeAction = new KAction(i18n("Red Eye..."), "redeyes", 0,
                                 this, SLOT(slotRedEye()),
                                 actionCollection(), "implugcore_redeye");
    m_redeyeAction->setWhatsThis( i18n( "This filter can be used to correct red eyes in a photo. "
                                        "Select a region including the eyes to use this option.") );

    m_BCGAction = new KAction(i18n("Brightness/Contrast/Gamma..."), "contrast", 0,
                      this, SLOT(slotBCG()),
                      actionCollection(), "implugcore_bcg");

    m_HSLAction = new KAction(i18n("Hue/Saturation/Lightness..."), "adjusthsl",
                      CTRL+Key_U,      // NOTE: Photoshop 7 use CTRL+U.
                      this, SLOT(slotHSL()),
                      actionCollection(), "implugcore_hsl");

    m_RGBAction = new KAction(i18n("Color Balance..."), "adjustrgb",
                      CTRL+Key_B,      // NOTE: Photoshop 7 use CTRL+B.
                      this, SLOT(slotRGB()),
                      actionCollection(), "implugcore_rgb");

    m_autoCorrectionAction = new KAction(i18n("Auto-Correction..."), "autocorrection",
                                 CTRL+SHIFT+Key_B, // NOTE: Photoshop 7 use CTRL+SHIFT+B with 'Auto-Color' option.
                                 this, SLOT(slotAutoCorrection()),
                                 actionCollection(), "implugcore_autocorrection");

    m_invertAction = new KAction(i18n("Invert"), "invertimage",
                         CTRL+Key_I,      // NOTE: Photoshop 7 use CTRL+I.
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
    BlurTool *tool = new BlurTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotSharpen()
{
    SharpenTool *tool = new SharpenTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotBCG()
{
    BCGTool *bcg = new BCGTool(this);
    loadTool(bcg);
}

void ImagePlugin_Core::slotRGB()
{
    RGBTool *rgb = new RGBTool(this);
    loadTool(rgb);
}

void ImagePlugin_Core::slotHSL()
{
    HSLTool *hsl = new HSLTool(this);
    loadTool(hsl);
}

void ImagePlugin_Core::slotAutoCorrection()
{
    AutoCorrectionTool *autocorrection = new AutoCorrectionTool(this);
    loadTool(autocorrection);
}

void ImagePlugin_Core::slotInvert()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    ImageIface iface(0, 0);

    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();

    DImgImageFilters filter;
    filter.invertImage(data, w, h, sixteenBit);
    iface.putOriginalImage(i18n("Invert"), data);
    delete [] data;

    kapp->restoreOverrideCursor();
}

void ImagePlugin_Core::slotBW()
{
    BWSepiaTool *bwsepia = new BWSepiaTool(this);
    loadTool(bwsepia);
}

void ImagePlugin_Core::slotRedEye()
{
    ImageIface iface(0, 0);

    if (!iface.selectedWidth() || !iface.selectedHeight())
    {
        RedEyePassivePopup* popup = new RedEyePassivePopup(kapp->activeWindow());
        popup->setView(i18n("Red-Eye Correction Tool"),
                       i18n("You need to select a region including the eyes to use "
                            "the red-eye correction tool"));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    RedEyeTool *redeye = new RedEyeTool(this);
    loadTool(redeye);
}

void ImagePlugin_Core::slotColorManagement()
{
    ICCProofTool *tool = new ICCProofTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotRatioCrop()
{
    RatioCropTool *ratiocrop = new RatioCropTool(this);
    loadTool(ratiocrop);
}

void ImagePlugin_Core::slotConvertTo8Bits()
{
    ImageIface iface(0, 0);

    if (!iface.originalSixteenBit())
    {
       KMessageBox::error(kapp->activeWindow(), i18n("This image is already using a depth of 8 bits / color / pixel."));
       return;
    }
    else
    {
       if (KMessageBox::warningContinueCancel(kapp->activeWindow(),
                                              i18n("Performing this operation will reduce image color quality. "
                                                   "Do you want to continue?")) == KMessageBox::Cancel)
           return;
    }

    kapp->setOverrideCursor( KCursor::waitCursor() );
    iface.convertOriginalColorDepth(32);
    kapp->restoreOverrideCursor();
}

void ImagePlugin_Core::slotConvertTo16Bits()
{
    ImageIface iface(0, 0);

    if (iface.originalSixteenBit())
    {
       KMessageBox::error(kapp->activeWindow(), i18n("This image is already using a depth of 16 bits / color / pixel."));
       return;
    }

    kapp->setOverrideCursor( KCursor::waitCursor() );
    iface.convertOriginalColorDepth(64);
    kapp->restoreOverrideCursor();
}
