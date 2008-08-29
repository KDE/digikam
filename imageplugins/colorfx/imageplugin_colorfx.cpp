/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin for to apply a color
 *               effect to an image.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "colorfxtool.h"
#include "imageplugin_colorfx.h"
#include "imageplugin_colorfx.moc"

using namespace DigikamColorFXImagesPlugin;

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_colorfx,
                           KGenericFactory<ImagePlugin_ColorFX>("digikamimageplugin_colorfx"));

ImagePlugin_ColorFX::ImagePlugin_ColorFX(QObject *parent, const char*, const QStringList &)
                   : Digikam::ImagePlugin(parent, "ImagePlugin_ColorFX")
{
    m_solarizeAction = new KAction(i18n("Color Effects..."), "colorfx", 0,
                           this, SLOT(slotColorFX()),
                           actionCollection(), "imageplugin_colorfx");

    setXMLFile( "digikamimageplugin_colorfx_ui.rc" );

    DDebug() << "ImagePlugin_ColorFX plugin loaded" << endl;
}

ImagePlugin_ColorFX::~ImagePlugin_ColorFX()
{
}

void ImagePlugin_ColorFX::setEnabledActions(bool enable)
{
    m_solarizeAction->setEnabled(enable);
}

void ImagePlugin_ColorFX::slotColorFX()
{
    ColorFXTool *colorfx = new ColorFXTool(this);
    loadTool(colorfx);
}

