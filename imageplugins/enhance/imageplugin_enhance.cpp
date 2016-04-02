/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to enhance photograph
 *
 * Copyright (C) 2005-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_enhance.h"

// Qt includes

#include <QKeySequence>
#include <QApplication>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "digikam_debug.h"
#include "dnotificationpopup.h"
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
#include "hotpixels/hotpixelstool.h"
#include "digikam_config.h"

#ifdef HAVE_LENSFUN
#include "lensautofixtool.h"
#endif // HAVE_LENSFUN

namespace DigikamEnhanceImagePlugin
{

K_PLUGIN_FACTORY( EnhanceFactory, registerPlugin<ImagePlugin_Enhance>(); )

class EditorToolPassivePopup : public DNotificationPopup
{
public:

    explicit EditorToolPassivePopup(QWidget* const parent)
        : DNotificationPopup(parent),
          m_parent(parent)
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

    QAction* hotpixelsAction;
    QAction* lensdistortionAction;
    QAction* antivignettingAction;
    QAction* lensAutoFixAction;
    QAction* redeyeAction;
    QAction* restorationAction;
    QAction* blurAction;
    QAction* sharpenAction;
    QAction* noiseReductionAction;
    QAction* localContrastAction;
    QAction* inPaintingAction;
};

ImagePlugin_Enhance::ImagePlugin_Enhance(QObject* const parent, const QVariantList&)
    : ImagePlugin(parent, QLatin1String("ImagePlugin_Enhance")),
      d(new Private)
{
    // to load the rc file from digikam's installation path
    setComponentName(QLatin1String("digikam"), i18nc("to be displayed in shortcuts dialog", "Enhance plugins"));

    KActionCollection *ac = actionCollection();

    d->restorationAction = new QAction(QIcon::fromTheme(QLatin1String("restoration")), i18n("Restoration..."), this);
    ac->addAction(QLatin1String("imageplugin_restoration"), d->restorationAction);
    connect(d->restorationAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRestoration()));

    d->sharpenAction = new QAction(QIcon::fromTheme(QLatin1String("sharpenimage")), i18n("Sharpen..."), this);
    ac->addAction(QLatin1String("imageplugin_sharpen"), d->sharpenAction);
    connect(d->sharpenAction, SIGNAL(triggered(bool)),
            this, SLOT(slotSharpen()));

    d->blurAction = new QAction(QIcon::fromTheme(QLatin1String("blurimage")), i18n("Blur..."), this);
    ac->addAction(QLatin1String("imageplugin_blur"), d->blurAction);
    connect(d->blurAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBlur()));

    d->noiseReductionAction = new QAction(QIcon::fromTheme(QLatin1String("noisereduction")), i18n("Noise Reduction..."), this);
    ac->addAction(QLatin1String("imageplugin_noisereduction"), d->noiseReductionAction);
    connect(d->noiseReductionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotNoiseReduction()));

    d->localContrastAction = new QAction(QIcon::fromTheme(QLatin1String("contrast")), i18n("Local Contrast..."), this);
    ac->addAction(QLatin1String("imageplugin_localcontrast"), d->localContrastAction);
    connect(d->localContrastAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLocalContrast()));

    d->redeyeAction = new QAction(QIcon::fromTheme(QLatin1String("redeyes")), i18n("Red Eye..."), this);
    d->redeyeAction->setWhatsThis(i18n("This filter can be used to correct red eyes in a photo. "
                                       "Select a region including the eyes to use this option."));
    ac->addAction(QLatin1String("imageplugin_redeye"), d->redeyeAction);
    connect(d->redeyeAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRedEye()));

    d->inPaintingAction = new QAction(QIcon::fromTheme(QLatin1String("inpainting")), i18n("In-painting..."), this);
    ac->addAction(QLatin1String("imageplugin_inpainting"), d->inPaintingAction);
    ac->setDefaultShortcut(d->inPaintingAction, Qt::CTRL+Qt::Key_E);
    d->inPaintingAction->setWhatsThis( i18n( "This filter can be used to in-paint a part in a photo. "
                                       "To use this option, select a region to in-paint.") );
    connect(d->inPaintingAction, SIGNAL(triggered(bool)),
            this, SLOT(slotInPainting()));

    d->antivignettingAction = new QAction(QIcon::fromTheme(QLatin1String("antivignetting")), i18n("Vignetting Correction..."), this);
    ac->addAction(QLatin1String("imageplugin_antivignetting"), d->antivignettingAction);
    connect(d->antivignettingAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAntiVignetting()));

    d->lensdistortionAction = new QAction(QIcon::fromTheme(QLatin1String("lensdistortion")), i18n("Distortion..."), this);
    ac->addAction(QLatin1String("imageplugin_lensdistortion"), d->lensdistortionAction);
    connect(d->lensdistortionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLensDistortion()));

    d->hotpixelsAction  = new QAction(QIcon::fromTheme(QLatin1String("hotpixels")), i18n("Hot Pixels..."), this);
    ac->addAction(QLatin1String("imageplugin_hotpixels"), d->hotpixelsAction);
    connect(d->hotpixelsAction, SIGNAL(triggered(bool)),
            this, SLOT(slotHotPixels()));

#ifdef HAVE_LENSFUN

    d->lensAutoFixAction = new QAction(QIcon::fromTheme(QLatin1String("lensautofix")), i18n("Auto-Correction..."), this);
    ac->addAction(QLatin1String("imageplugin_lensautofix"), d->lensAutoFixAction );
    connect(d->lensAutoFixAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLensAutoFix()));

#endif // HAVE_LENSFUN

    HotPixelsTool::registerFilter();

    setActionCategory(i18n("Enhance"));
    setXMLFile(QLatin1String("digikamimageplugin_enhance_ui.rc"));

    qCDebug(DIGIKAM_IMAGEPLUGINS_LOG) << "ImagePlugin_Enhance plugin loaded";
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
        EditorToolPassivePopup* const popup = new EditorToolPassivePopup(qApp->activeWindow());
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
        EditorToolPassivePopup* const popup = new EditorToolPassivePopup(qApp->activeWindow());
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

#include "imageplugin_enhance.moc"
