/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to enhance photograph
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_enhance.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes

#include "restorationtool.h"
#include "blurtool.h"
#include "sharpentool.h"
#include "noisereductiontool.h"
#include "localcontrasttool.h"
#include "redeyetool.h"
#include "imageiface.h"
#include "inpaintingtool.h"
#include "antivignettingtool.h"
#include "config-digikam.h"

#ifdef HAVE_LENSFUN
#include "lensautofixtool.h"
#endif // HAVE_LENSFUN

using namespace DigikamEnhanceImagePlugin;

K_PLUGIN_FACTORY( EnhanceFactory, registerPlugin<ImagePlugin_Enhance>(); )
K_EXPORT_PLUGIN ( EnhanceFactory("digikamimageplugin_enhance") )

ImagePlugin_Enhance::ImagePlugin_Enhance(QObject* parent, const QVariantList&)
                   : ImagePlugin(parent, "ImagePlugin_Enhance")
{
    m_restorationAction  = new KAction(KIcon("restoration"), i18n("Restoration..."), this);
    actionCollection()->addAction("imageplugin_restoration", m_restorationAction);
    connect(m_restorationAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRestoration()));

    m_sharpenAction = new KAction(KIcon("sharpenimage"), i18n("Sharpen..."), this);
    actionCollection()->addAction("imageplugin_sharpen", m_sharpenAction);
    connect(m_sharpenAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotSharpen()));

    m_blurAction = new KAction(KIcon("blurimage"), i18n("Blur..."), this);
    actionCollection()->addAction("imageplugin_blur", m_blurAction);
    connect(m_blurAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotBlur()));

    m_noiseReductionAction = new KAction(KIcon("noisereduction"), i18n("Noise Reduction..."), this);
    actionCollection()->addAction("imageplugin_noisereduction", m_noiseReductionAction);
    connect(m_noiseReductionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotNoiseReduction()));

    m_localContrastAction = new KAction(KIcon("contrast"), i18n("Local Contrast..."), this);
    actionCollection()->addAction("imageplugin_localcontrast", m_localContrastAction);
    connect(m_localContrastAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLocalContrast()));

    m_redeyeAction = new KAction(KIcon("redeyes"), i18n("Red Eye..."), this);
    m_redeyeAction->setWhatsThis(i18n("This filter can be used to correct red eyes in a photo. "
                                      "Select a region including the eyes to use this option."));
    actionCollection()->addAction("implugcore_redeye", m_redeyeAction);
    connect(m_redeyeAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotRedEye()));

    m_inPaintingAction = new KAction(KIcon("inpainting"), i18n("In-painting..."), this);
    m_inPaintingAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_E));
    m_inPaintingAction->setWhatsThis( i18n( "This filter can be used to in-paint a part in a photo. "
                                            "To use this option, select a region to in-paint.") );
    connect(m_inPaintingAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotInPainting()));

    m_antivignettingAction  = new KAction(KIcon("antivignetting"), i18n("Vignetting Correction..."), this);
    actionCollection()->addAction("imageplugin_antivignetting", m_antivignettingAction);
    connect(m_antivignettingAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAntiVignetting()));

#ifdef HAVE_LENSFUN

    m_lensAutoFixAction = new KAction(KIcon("lensdistortion"), i18n("Auto-Correction..."), this);
    actionCollection()->addAction("imageplugin_lensautofix", m_lensAutoFixAction );
    connect(m_lensAutoFixAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLensAutoFix()));

#endif // HAVE_LENSFUN

    setXMLFile( "digikamimageplugin_enhance_ui.rc" );

    kDebug() << "ImagePlugin_Enhance plugin loaded";
}

ImagePlugin_Enhance::~ImagePlugin_Enhance()
{
}

void ImagePlugin_Enhance::setEnabledActions(bool b)
{
    m_restorationAction->setEnabled(b);
    m_blurAction->setEnabled(b);
    m_sharpenAction->setEnabled(b);
    m_noiseReductionAction->setEnabled(b);
    m_localContrastAction->setEnabled(b);
    m_redeyeAction->setEnabled(b);
    m_inPaintingAction->setEnabled(b);
    m_antivignettingAction->setEnabled(b);

#ifdef HAVE_LENSFUN
    m_lensAutoFixAction->setEnabled(b);
#endif // HAVE_LENSFUN
}

void ImagePlugin_Enhance::slotRestoration()
{
    RestorationTool* tool = new RestorationTool(this);
    loadTool(tool);
}

void ImagePlugin_Enhance::slotBlur()
{
    BlurTool* tool = new BlurTool(this);
    loadTool(tool);
}

void ImagePlugin_Enhance::slotSharpen()
{
    SharpenTool* tool = new SharpenTool(this);
    loadTool(tool);
}

void ImagePlugin_Enhance::slotNoiseReduction()
{
    NoiseReductionTool* tool = new NoiseReductionTool(this);
    loadTool(tool);
}

void ImagePlugin_Enhance::slotLocalContrast()
{
    LocalContrastTool* tool = new LocalContrastTool(this);
    loadTool(tool);
}

void ImagePlugin_Enhance::slotRedEye()
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

    RedEyeTool* tool = new RedEyeTool(this);
    loadTool(tool);
}

void ImagePlugin_Enhance::slotInPainting()
{
    ImageIface iface(0, 0);

    int w = iface.selectedWidth();
    int h = iface.selectedHeight();

    if (!w || !h)
    {
        InPaintingPassivePopup* popup = new InPaintingPassivePopup(kapp->activeWindow());
        popup->setView(i18n("In-Painting Photograph Tool"),
                       i18n("To use this tool, you need to select a region "
                            "to in-paint."));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    InPaintingTool *tool = new InPaintingTool(this);
    loadTool(tool);
}

void ImagePlugin_Enhance::slotLensAutoFix()
{
#ifdef HAVE_LENSFUN
    LensAutoFixTool* tool = new LensAutoFixTool(this);
    loadTool(tool);
#endif // HAVE_LENSFUN
}

void ImagePlugin_Enhance::slotAntiVignetting()
{
    AntiVignettingTool* tool = new AntiVignettingTool(this);
    loadTool(tool);
}
