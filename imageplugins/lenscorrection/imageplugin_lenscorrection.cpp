/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : A plugin to fix camera lens aberrations.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
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


#include "imageplugin_lenscorrection.h"
#include "imageplugin_lenscorrection.moc"

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "config-digikam.h"

#ifdef HAVE_LENSFUN
#include "autocorrectiontool.h"
using namespace DigikamAutoCorrectionImagesPlugin;
#endif // HAVE_LENSFUN

#include "antivignettingtool.h"
#include "lensdistortiontool.h"

using namespace DigikamAntiVignettingImagesPlugin;
using namespace DigikamLensDistortionImagesPlugin;

K_PLUGIN_FACTORY( LensCorrectionFactory, registerPlugin<ImagePlugin_LensCorrection>(); )
K_EXPORT_PLUGIN ( LensCorrectionFactory("digikamimageplugin_lenscorrection") )

ImagePlugin_LensCorrection::ImagePlugin_LensCorrection(QObject *parent, const QVariantList &)
                          : Digikam::ImagePlugin(parent, "ImagePlugin_LensCorrection")
{
    setActionCategory(i18n("Lens Correction"));

#ifdef HAVE_LENSFUN

    m_autoCorrectionAction  = new KAction(KIcon("lensdistortion"), i18n("Auto-Correction..."), this);
    actionCollection()->addAction("imageplugin_autocorrection", m_autoCorrectionAction );

    connect(m_autoCorrectionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAutoCorrection()));

#endif // HAVE_LENSFUN

    m_lensdistortionAction  = new KAction(KIcon("lensdistortion"), i18n("Distortion..."), this);
    actionCollection()->addAction("imageplugin_lensdistortion", m_lensdistortionAction );

    connect(m_lensdistortionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLensDistortion()));

    m_antivignettingAction  = new KAction(KIcon("antivignetting"), i18n("Vignetting Correction..."), this);
    actionCollection()->addAction("imageplugin_antivignetting", m_antivignettingAction );

    connect(m_antivignettingAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAntiVignetting()));

    setXMLFile("digikamimageplugin_lenscorrection_ui.rc");

    kDebug() << "ImagePlugin_LensCorrection plugin loaded";
}

ImagePlugin_LensCorrection::~ImagePlugin_LensCorrection()
{
}

void ImagePlugin_LensCorrection::setEnabledActions(bool enable)
{
#ifdef HAVE_LENSFUN
    m_autoCorrectionAction->setEnabled(enable);
#endif // HAVE_LENSFUN

    m_lensdistortionAction->setEnabled(enable);
    m_antivignettingAction->setEnabled(enable);
}

void ImagePlugin_LensCorrection::slotAutoCorrection()
{
#ifdef HAVE_LENSFUN
    AutoCorrectionTool *tool = new AutoCorrectionTool(this);
    loadTool(tool);
#endif // HAVE_LENSFUN
}

void ImagePlugin_LensCorrection::slotLensDistortion()
{
    LensDistortionTool *tool = new LensDistortionTool(this);
    loadTool(tool);
}

void ImagePlugin_LensCorrection::slotAntiVignetting()
{
    AntiVignettingTool *tool = new AntiVignettingTool(this);
    loadTool(tool);
}
