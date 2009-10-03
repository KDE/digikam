/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-11
 * Description : a plugin to apply Distortion FX to an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Original Distortion algorithms copyrighted 2004-2005 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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


#include "imageplugin_distortionfx.h"
#include "imageplugin_distortionfx.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "distortionfxtool.h"
#include "debug.h"

using namespace DigikamDistortionFXImagesPlugin;

K_PLUGIN_FACTORY( DistortionFXFactory, registerPlugin<ImagePlugin_DistortionFX>(); )
K_EXPORT_PLUGIN ( DistortionFXFactory("digikamimageplugin_distortionfx") )

ImagePlugin_DistortionFX::ImagePlugin_DistortionFX(QObject *parent, const QVariantList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_DistortionFX")
{
    m_distortionfxAction  = new KAction(KIcon("distortionfx"), i18n("Distortion Effects..."), this);
    actionCollection()->addAction("imageplugin_distortionfx", m_distortionfxAction );

    connect(m_distortionfxAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotDistortionFX()));

    setXMLFile( "digikamimageplugin_distortionfx_ui.rc" );

    kDebug(imagePluginsAreaCode) << "ImagePlugin_DistortionFX plugin loaded";
}

ImagePlugin_DistortionFX::~ImagePlugin_DistortionFX()
{
}

void ImagePlugin_DistortionFX::setEnabledActions(bool enable)
{
    m_distortionfxAction->setEnabled(enable);
}

void ImagePlugin_DistortionFX::slotDistortionFX()
{
    DistortionFXTool *tool = new DistortionFXTool(this);
    loadTool(tool);
}
