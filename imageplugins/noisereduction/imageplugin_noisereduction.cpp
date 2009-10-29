/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to reduce CCD noise.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "imageplugin_noisereduction.h"
#include "imageplugin_noisereduction.moc"

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>

// Local includes

#include "noisereductiontool.h"
#include "debug.h"

using namespace DigikamNoiseReductionImagesPlugin;

K_PLUGIN_FACTORY( NoiseReductionFactory, registerPlugin<ImagePlugin_NoiseReduction>(); )
K_EXPORT_PLUGIN ( NoiseReductionFactory("digikamimageplugin_noisereduction") )

ImagePlugin_NoiseReduction::ImagePlugin_NoiseReduction(QObject *parent, const QVariantList &)
                          : Digikam::ImagePlugin(parent, "ImagePlugin_NoiseReduction")
{
    m_noiseReductionAction  = new KAction(KIcon("noisereduction"), i18n("Noise Reduction..."), this);
    actionCollection()->addAction("imageplugin_noisereduction", m_noiseReductionAction );

    connect(m_noiseReductionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotNoiseReduction()));

    setXMLFile("digikamimageplugin_noisereduction_ui.rc");

    kDebug() << "ImagePlugin_NoiseReduction plugin loaded";
}

ImagePlugin_NoiseReduction::~ImagePlugin_NoiseReduction()
{
}

void ImagePlugin_NoiseReduction::setEnabledActions(bool enable)
{
    m_noiseReductionAction->setEnabled(enable);
}

void ImagePlugin_NoiseReduction::slotNoiseReduction()
{
    NoiseReductionTool *tool = new NoiseReductionTool(this);
    loadTool(tool);
}
