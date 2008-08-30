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

// KDE includes.

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes.

#include "ddebug.h"
#include "blurfxtool.h"
#include "imageplugin_blurfx.h"
#include "imageplugin_blurfx.moc"

using namespace DigikamBlurFXImagesPlugin;

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_blurfx,
                           KGenericFactory<ImagePlugin_BlurFX>("digikamimageplugin_blurfx"));

ImagePlugin_BlurFX::ImagePlugin_BlurFX(QObject *parent, const char*, const QStringList &)
                  : Digikam::ImagePlugin(parent, "ImagePlugin_BlurFX")
{
    m_blurfxAction = new KAction(i18n("Blur Effects..."), "blurfx", 0, 
                         this, SLOT(slotBlurFX()),
                         actionCollection(), "imageplugin_blurfx");

    setXMLFile( "digikamimageplugin_blurfx_ui.rc" );

    DDebug() << "ImagePlugin_BlurFX plugin loaded" << endl;
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
