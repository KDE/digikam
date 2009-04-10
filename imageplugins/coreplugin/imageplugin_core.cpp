/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : digiKam image editor plugin core
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_core.h"
#include "imageplugin_core.moc"

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>

// Local includes

#include "dimg.h"
#include "dimgimagefilters.h"
#include "imageiface.h"
#include "autocorrectiontool.h"
#include "bcgtool.h"
#include "bwsepiatool.h"
#include "hsltool.h"
#include "iccprooftool.h"
#include "imageresize.h"
#include "blurtool.h"
#include "ratiocroptool.h"
#include "sharpentool.h"
#include "redeyetool.h"
#include "rgbtool.h"

using namespace DigikamImagesPluginCore;
using namespace Digikam;

K_PLUGIN_FACTORY( CorePluginFactory, registerPlugin<ImagePlugin_Core>(); )
K_EXPORT_PLUGIN ( CorePluginFactory("digikamimageplugin_core") )

ImagePlugin_Core::ImagePlugin_Core(QObject *parent, const QVariantList &)
                : ImagePlugin(parent, "ImagePlugin_Core")
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


    m_resizeAction = new KAction(KIcon("transform-scale"), i18n("&Resize..."), this);
    actionCollection()->addAction("implugcore_resize", m_resizeAction);
    connect(m_resizeAction, SIGNAL(triggered()),
            this, SLOT(slotResize()));

    //-------------------------------
    // Init. menu actions.

    setXMLFile("digikamimageplugin_core_ui.rc");

    kDebug(50006) << "ImagePlugin_Core plugin loaded" << endl;
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
    kapp->activeWindow()->setCursor( Qt::WaitCursor );

    ImageIface iface(0, 0);

    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();

    DImgImageFilters filter;
    filter.invertImage(data, w, h, sixteenBit);
    iface.putOriginalImage(i18n("Invert"), data);
    delete [] data;

    kapp->activeWindow()->unsetCursor();
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
       if (KMessageBox::warningContinueCancel(
                        kapp->activeWindow(),
                        i18n("Performing this operation will reduce image color quality. "
                             "Do you want to continue?"), QString(), 
                        KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                        QString("ImagePluginCore16To8Bits")) == KMessageBox::Cancel)
           return;
    }

    kapp->setOverrideCursor( Qt::WaitCursor );
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

    kapp->setOverrideCursor( Qt::WaitCursor );
    iface.convertOriginalColorDepth(64);
    kapp->restoreOverrideCursor();
}

void ImagePlugin_Core::slotBCG()
{
    BCGTool *tool = new BCGTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotRGB()
{
    RGBTool *tool = new RGBTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotBlur()
{
    BlurTool *tool = new BlurTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotAutoCorrection()
{
    AutoCorrectionTool *tool = new AutoCorrectionTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotRedEye()
{
    ImageIface iface(0, 0);

    if (!iface.selectedWidth() || !iface.selectedHeight())
    {
        RedEyePassivePopup* popup = new
                                 RedEyePassivePopup(kapp->activeWindow());
        popup->setView(i18n("Red-Eye Correction Tool"),
                       i18n("You need to select a region including the eyes to use "
                            "the red-eye correction tool"));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    RedEyeTool *tool = new RedEyeTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotColorManagement()
{
    ICCProofTool *tool = new ICCProofTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotBW()
{
    BWSepiaTool *tool = new BWSepiaTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotHSL()
{
    HSLTool *tool = new HSLTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotSharpen()
{
    SharpenTool *tool = new SharpenTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotRatioCrop()
{
    RatioCropTool *tool = new RatioCropTool(this);
    loadTool(tool);
}

void ImagePlugin_Core::slotResize()
{
    ImageResize *tool = new ImageResize(this);
    loadTool(tool);
}
