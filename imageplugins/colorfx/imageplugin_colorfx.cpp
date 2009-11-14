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


// #include "imageplugin_colorfx.h"
#include "imageplugin_colorfx.moc"

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

#include "colorfxtool.h"

using namespace DigikamColorFXImagesPlugin;

K_PLUGIN_FACTORY( ColorFXFactory, registerPlugin<ImagePlugin_ColorFX>(); )
K_EXPORT_PLUGIN ( ColorFXFactory("digikamimageplugin_colorfx") )

ImagePlugin_ColorFX::ImagePlugin_ColorFX(QObject *parent, const QVariantList &)
                   : Digikam::ImagePlugin(parent, "ImagePlugin_ColorFX")
{
    m_solarizeAction  = new KAction(KIcon("colorfx"), i18n("Color Effects..."), this);
    actionCollection()->addAction("imageplugin_colorfx", m_solarizeAction );

    connect(m_solarizeAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotColorFX()));

    setXMLFile( "digikamimageplugin_colorfx_ui.rc" );

    kDebug() << "ImagePlugin_ColorFX plugin loaded";
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
    ColorFXTool *tool = new ColorFXTool(this);
    loadTool(tool);
}
