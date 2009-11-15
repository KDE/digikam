/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-09
 * Description : a plugin to apply Blur FX to images
 *
 * Copyright 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "imageplugin_blurfx.moc"

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

#include "blurfxtool.h"

using namespace DigikamBlurFXImagesPlugin;

K_PLUGIN_FACTORY( BlurFXFactory, registerPlugin<ImagePlugin_BlurFX>(); )
K_EXPORT_PLUGIN ( BlurFXFactory("digikamimageplugin_blurfx") )

ImagePlugin_BlurFX::ImagePlugin_BlurFX(QObject *parent, const QVariantList &)
                  : Digikam::ImagePlugin(parent, "ImagePlugin_BlurFX")
{
    m_blurfxAction  = new KAction(KIcon("blurfx"), i18n("Blur Effects..."), this);
    actionCollection()->addAction("imageplugin_blurfx", m_blurfxAction );

    connect(m_blurfxAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBlurFX()));

    setXMLFile( "digikamimageplugin_blurfx_ui.rc" );

    kDebug() << "ImagePlugin_BlurFX plugin loaded";
}

ImagePlugin_BlurFX::~ImagePlugin_BlurFX()
{
}

void ImagePlugin_BlurFX::setEnabledActions(bool enable)
{
    m_blurfxAction->setEnabled(enable);
}

void ImagePlugin_BlurFX::slotBlurFX()
{
    BlurFXTool *tool = new BlurFXTool(this);
    loadTool(tool);
}
