/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to enhance photograph
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <kpassivepopup.h>
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
#include "lensdistortiontool.h"
#include "hotpixelstool.h"
#include "config-digikam.h"

#ifdef HAVE_LENSFUN
#include "lensautofixtool.h"
#endif // HAVE_LENSFUN

namespace DigikamEnhanceImagePlugin
{

K_PLUGIN_FACTORY( EnhanceFactory, registerPlugin<ImagePlugin_Enhance>(); )
K_EXPORT_PLUGIN ( EnhanceFactory("digikamimageplugin_enhance") )

class EditorToolPassivePopup : public KPassivePopup
{
public:

    explicit EditorToolPassivePopup(QWidget* const parent)
        : KPassivePopup(parent), m_parent(parent)
    {
    }

protected:

    virtual void positionSelf()
    {
        move(m_parent->x() + 30, m_parent->y() + 30);
    }

private:

    QWidget* m_parent;
};

// -----------------------------------------------------------------------------------------------

class ImagePlugin_Enhance::Private
{
public:

    Private() :
        hotpixelsAction(0),
        lensdistortionAction(0),
        antivignettingAction(0),
        lensAutoFixAction(0),
        redeyeAction(0),
        restorationAction(0),
        blurAction(0),
        sharpenAction(0),
        noiseReductionAction(0),
        localContrastAction(0),
        inPaintingAction(0)
    {
    }

    KAction* hotpixelsAction;
    KAction* lensdistortionAction;
    KAction* antivignettingAction;
    KAction* lensAutoFixAction;
    KAction* redeyeAction;
    KAction* restorationAction;
    KAction* blurAction;
    KAction* sharpenAction;
    KAction* noiseReductionAction;
    KAction* localContrastAction;
    KAction* inPaintingAction;
};

ImagePlugin_Enhance::ImagePlugin_Enhance(QObject* const parent, const QVariantList&)
    : ImagePlugin(parent, "ImagePlugin_Enhance"),
      d(new Private)
{
    d->restorationAction = new KAction(KIcon("restoration"), i18n("Restoration..."), this);
    actionCollection()->addAction("imageplugin_restoration", d->restorationAction);
    connect(d->restorationAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRestoration()));

    d->sharpenAction = new KAction(KIcon("sharpenimage"), i18n("Sharpen..."), this);
    actionCollection()->addAction("imageplugin_sharpen", d->sharpenAction);
    connect(d->sharpenAction, SIGNAL(triggered(bool)),
            this, SLOT(slotSharpen()));

    d->blurAction = new KAction(KIcon("blurimage"), i18n("Blur..."), this);
    actionCollection()->addAction("imageplugin_blur", d->blurAction);
    connect(d->blurAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBlur()));

    d->noiseReductionAction = new KAction(KIcon("noisereduction"), i18n("Noise Reduction..."), this);
    actionCollection()->addAction("imageplugin_noisereduction", d->noiseReductionAction);
    connect(d->noiseReductionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotNoiseReduction()));

    d->localContrastAction = new KAction(KIcon("contrast"), i18n("Local Contrast..."), this);
    actionCollection()->addAction("imageplugin_localcontrast", d->localContrastAction);
    connect(d->localContrastAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLocalContrast()));

    d->redeyeAction = new KAction(KIcon("redeyes"), i18n("Red Eye..."), this);
    d->redeyeAction->setWhatsThis(i18n("This filter can be used to correct red eyes in a photo. "
                                       "Select a region including the eyes to use this option."));
    actionCollection()->addAction("imageplugin_redeye", d->redeyeAction);
    connect(d->redeyeAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRedEye()));

    d->inPaintingAction = new KAction(KIcon("inpainting"), i18n("In-painting..."), this);
    actionCollection()->addAction("imageplugin_inpainting", d->inPaintingAction);
    d->inPaintingAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_E));
    d->inPaintingAction->setWhatsThis( i18n( "This filter can be used to in-paint a part in a photo. "
                                       "To use this option, select a region to in-paint.") );
    connect(d->inPaintingAction, SIGNAL(triggered(bool)),
            this, SLOT(slotInPainting()));

    d->antivignettingAction = new KAction(KIcon("antivignetting"), i18n("Vignetting Correction..."), this);
    actionCollection()->addAction("imageplugin_antivignetting", d->antivignettingAction);
    connect(d->antivignettingAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAntiVignetting()));

    d->lensdistortionAction = new KAction(KIcon("lensdistortion"), i18n("Distortion..."), this);
    actionCollection()->addAction("imageplugin_lensdistortion", d->lensdistortionAction);
    connect(d->lensdistortionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLensDistortion()));

    d->hotpixelsAction  = new KAction(KIcon("hotpixels"), i18n("Hot Pixels..."), this);
    actionCollection()->addAction("imageplugin_hotpixels", d->hotpixelsAction);
    connect(d->hotpixelsAction, SIGNAL(triggered(bool)),
            this, SLOT(slotHotPixels()));

#ifdef HAVE_LENSFUN

    d->lensAutoFixAction = new KAction(KIcon("lensautofix"), i18n("Auto-Correction..."), this);
    actionCollection()->addAction("imageplugin_lensautofix", d->lensAutoFixAction );
    connect(d->lensAutoFixAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLensAutoFix()));

#endif // HAVE_LENSFUN

    HotPixelsTool::registerFilter();

    setActionCategory(i18n("Enhance"));
    setXMLFile( "digikamimageplugin_enhance_ui.rc" );

    kDebug() << "ImagePlugin_Enhance plugin loaded";
}

ImagePlugin_Enhance::~ImagePlugin_Enhance()
{
    delete d;
}

void ImagePlugin_Enhance::setEnabledActions(bool b)
{
    d->restorationAction->setEnabled(b);
    d->blurAction->setEnabled(b);
    d->sharpenAction->setEnabled(b);
    d->noiseReductionAction->setEnabled(b);
    d->localContrastAction->setEnabled(b);
    d->redeyeAction->setEnabled(b);
    d->inPaintingAction->setEnabled(b);
    d->lensdistortionAction->setEnabled(b);
    d->antivignettingAction->setEnabled(b);
    d->hotpixelsAction->setEnabled(b);

#ifdef HAVE_LENSFUN
    d->lensAutoFixAction->setEnabled(b);
#endif // HAVE_LENSFUN
}

void ImagePlugin_Enhance::slotHotPixels()
{
    loadTool(new HotPixelsTool(this));
}

void ImagePlugin_Enhance::slotLensDistortion()
{
    loadTool(new LensDistortionTool(this));
}

void ImagePlugin_Enhance::slotRestoration()
{
    loadTool(new RestorationTool(this));
}

void ImagePlugin_Enhance::slotBlur()
{
    loadTool(new BlurTool(this));
}

void ImagePlugin_Enhance::slotSharpen()
{
    loadTool(new SharpenTool(this));
}

void ImagePlugin_Enhance::slotNoiseReduction()
{
    loadTool(new NoiseReductionTool(this));
}

void ImagePlugin_Enhance::slotLocalContrast()
{
    loadTool(new LocalContrastTool(this));
}

void ImagePlugin_Enhance::slotRedEye()
{
    ImageIface iface;

    if (iface.selectionRect().size().isNull())
    {
        EditorToolPassivePopup* popup = new EditorToolPassivePopup(kapp->activeWindow());
        popup->setView(i18n("Red-Eye Correction Tool"),
                       i18n("You need to select a region including the eyes to use "
                            "the red-eye correction tool"));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    loadTool(new RedEyeTool(this));
}

void ImagePlugin_Enhance::slotInPainting()
{
    ImageIface iface;

    if (iface.selectionRect().size().isNull())
    {
        EditorToolPassivePopup* popup = new EditorToolPassivePopup(kapp->activeWindow());
        popup->setView(i18n("In-Painting Photograph Tool"),
                       i18n("To use this tool, you need to select a region "
                            "to in-paint."));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    loadTool(new InPaintingTool(this));
}

void ImagePlugin_Enhance::slotLensAutoFix()
{
#ifdef HAVE_LENSFUN
    loadTool(new LensAutoFixTool(this));
#endif // HAVE_LENSFUN
}

void ImagePlugin_Enhance::slotAntiVignetting()
{
    loadTool(new AntiVignettingTool(this));
}

} // namespace DigikamEnhanceImagePlugin
