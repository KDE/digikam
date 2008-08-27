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

// KDE includes.

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
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
#include "imageplugin_core.moc"

K_PLUGIN_FACTORY( CorePluginFactory, registerPlugin<ImagePlugin_Core>(); )
K_EXPORT_PLUGIN ( CorePluginFactory("digikamimageplugin_core") )

ImagePlugin_Core::ImagePlugin_Core(QObject *parent, const QVariantList &)
                : Digikam::ImagePlugin(parent, "ImagePlugin_Core")
{
    //-------------------------------
    // Fix and Colors menu actions

    m_blurAction = new KAction(KIcon("blurimage"), i18n("Blur..."), this);
    actionCollection()->addAction("implugcore_blur", m_blurAction );
    connect(m_blurAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotBlur()));

    m_sharpenAction = new KAction(KIcon("sharpenimage"), i18n("Sharpen..."), this);
    actionCollection()->addAction("implugcore_sharpen", m_sharpenAction );
    connect(m_sharpenAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotSharpen()));

    m_redeyeAction = new KAction(KIcon("redeyes"), i18n("Red Eye..."), this);
    m_redeyeAction->setWhatsThis( i18n( "This filter can be used to correct red eyes in a photo. "
                                        "Select a region including the eyes to use this option.") );
    actionCollection()->addAction("implugcore_redeye", m_redeyeAction );
    connect(m_redeyeAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotRedEye()));

    m_BCGAction = new KAction(KIcon("contrast"), i18n("Brightness/Contrast/Gamma..."), this);
    actionCollection()->addAction("implugcore_bcg", m_BCGAction );
    connect(m_BCGAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotBCG()));

    // NOTE: Photoshop 7 use CTRL+U.
    m_HSLAction = new KAction(KIcon("adjusthsl"), i18n("Hue/Saturation/Lightness..."), this);
    m_HSLAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_U));      
    actionCollection()->addAction("implugcore_hsl", m_HSLAction );
    connect(m_HSLAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotHSL()));

    // NOTE: Photoshop 7 use CTRL+B.
    m_RGBAction = new KAction(KIcon("adjustrgb"), i18n("Color Balance..."), this);
    m_RGBAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_B));      
    actionCollection()->addAction("implugcore_rgb", m_RGBAction );
    connect(m_RGBAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotRGB()));

    // NOTE: Photoshop 7 use CTRL+SHIFT+B with
    m_autoCorrectionAction = new KAction(KIcon("autocorrection"), i18n("Auto-Correction..."), this);
    m_autoCorrectionAction->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_B));
    actionCollection()->addAction("implugcore_autocorrection", m_autoCorrectionAction );
    connect(m_autoCorrectionAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotAutoCorrection()));

    // NOTE: Photoshop 7 use CTRL+I.
    m_invertAction = new KAction(KIcon("invertimage"), i18n("Invert"), this);
    m_invertAction->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_I));      
    actionCollection()->addAction("implugcore_invert", m_invertAction );
    connect(m_invertAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotInvert()));

    m_convertTo8Bits = new KAction(KIcon("depth16to8"), i18n("8 bits"), this);
    actionCollection()->addAction("implugcore_convertto8bits", m_convertTo8Bits );
    connect(m_convertTo8Bits, SIGNAL(triggered(bool) ), 
            this, SLOT(slotConvertTo8Bits()));
    
    m_convertTo16Bits = new KAction(KIcon("depth8to16"), i18n("16 bits"), this);
    actionCollection()->addAction("implugcore_convertto16bits", m_convertTo16Bits );
    connect(m_convertTo16Bits, SIGNAL(triggered(bool) ), 
            this, SLOT(slotConvertTo16Bits()));
    
    m_colorManagementAction = new KAction(KIcon("colormanagement"), i18n("Color Management..."), this);
    actionCollection()->addAction("implugcore_colormanagement", m_colorManagementAction );
    connect(m_colorManagementAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotColorManagement()));

    //-------------------------------
    // Filters menu actions.

    m_BWAction = new KAction(KIcon("bwtonal"), i18n("Black && White..."), this);
    actionCollection()->addAction("implugcore_blackwhite", m_BWAction );
    connect(m_BWAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotBW()));

    //-------------------------------
    // Transform menu actions.

    m_aspectRatioCropAction = new KAction(KIcon("ratiocrop"), i18n("Aspect Ratio Crop..."), this);
    actionCollection()->addAction("implugcore_ratiocrop", m_aspectRatioCropAction );
    connect(m_aspectRatioCropAction, SIGNAL(triggered(bool) ), 
            this, SLOT(slotRatioCrop()));

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
    m_convertTo8Bits->setEnabled(enable);
    m_convertTo16Bits->setEnabled(enable);
    m_invertAction->setEnabled(enable);
    m_BCGAction->setEnabled(enable);
    m_RGBAction->setEnabled(enable);
    m_blurAction->setEnabled(enable);
    m_redeyeAction->setEnabled(enable);
    m_autoCorrectionAction->setEnabled(enable);
    m_BWAction->setEnabled(enable);
    m_colorManagementAction->setEnabled(enable);
    m_HSLAction->setEnabled(enable);
    m_sharpenAction->setEnabled(enable);
    m_aspectRatioCropAction->setEnabled(enable);
}

void ImagePlugin_Core::slotInvert()
{
    parentWidget()->setCursor( Qt::WaitCursor );

    Digikam::ImageIface iface(0, 0);

    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();

    Digikam::DImgImageFilters filter;
    filter.invertImage(data, w, h, sixteenBit);
    iface.putOriginalImage(i18n("Invert"), data);
    delete [] data;

    parentWidget()->unsetCursor();
}

void ImagePlugin_Core::slotConvertTo8Bits()
{
    Digikam::ImageIface iface(0, 0);

    if (!iface.originalSixteenBit())
    {
       KMessageBox::error(parentWidget(), i18n("This image is already using a depth of 8 bits / color / pixel."));
       return;
    }
    else
    {
       if (KMessageBox::warningContinueCancel(parentWidget(),
                                              i18n("Performing this operation will reduce image color quality. "
                                                   "Do you want to continue?")) == KMessageBox::Cancel)
           return;
    }

    parentWidget()->setCursor( Qt::WaitCursor );
    iface.convertOriginalColorDepth(32);
    parentWidget()->unsetCursor();
}

void ImagePlugin_Core::slotConvertTo16Bits()
{
    Digikam::ImageIface iface(0, 0);

    if (iface.originalSixteenBit())
    {
       KMessageBox::error(parentWidget(), i18n("This image is already using a depth of 16 bits / color / pixel."));
       return;
    }

    parentWidget()->setCursor( Qt::WaitCursor );
    iface.convertOriginalColorDepth(64);
    parentWidget()->unsetCursor();
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

void ImagePlugin_Core::slotBlur()
{
    DigikamImagesPluginCore::ImageEffect_Blur dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotAutoCorrection()
{
    DigikamImagesPluginCore::ImageEffect_AutoCorrection dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotRedEye()
{
    Digikam::ImageIface iface(0, 0);

    if (!iface.selectedWidth() || !iface.selectedHeight())
    {
        DigikamImagesPluginCore::RedEyePassivePopup* popup = new
                                 DigikamImagesPluginCore::RedEyePassivePopup(parentWidget());
        popup->setView(i18n("Red-Eye Correction Tool"),
                       i18n("You need to select a region including the eyes to use "
                            "the red-eye correction tool"));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    DigikamImagesPluginCore::ImageEffect_RedEye dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotBW()
{
    DigikamImagesPluginCore::ImageEffect_BWSepia dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotHSL()
{
    DigikamImagesPluginCore::ImageEffect_HSL dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotSharpen()
{
    DigikamImagesPluginCore::ImageEffect_Sharpen dlg(parentWidget());
    dlg.exec();
}

void ImagePlugin_Core::slotRatioCrop()
{
    DigikamImagesPluginCore::ImageEffect_RatioCrop dlg(parentWidget());
    dlg.exec();
}
