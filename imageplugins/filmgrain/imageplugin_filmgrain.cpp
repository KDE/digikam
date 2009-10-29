/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-01
 * Description : a digiKam image editor plugin for add film
 *               grain on an image.
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


#include "imageplugin_filmgrain.h"
#include "imageplugin_filmgrain.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "filmgraintool.h"
#include "debug.h"

using namespace DigikamFilmGrainImagesPlugin;

K_PLUGIN_FACTORY( FilmGrainFactory, registerPlugin<ImagePlugin_FilmGrain>(); )
K_EXPORT_PLUGIN ( FilmGrainFactory("digikamimageplugin_filmgrain") )

ImagePlugin_FilmGrain::ImagePlugin_FilmGrain(QObject *parent, const QVariantList &)
                     : Digikam::ImagePlugin(parent, "ImagePlugin_FilmGrain")
{
    m_filmgrainAction  = new KAction(KIcon("filmgrain"), i18n("Add Film Grain..."), this);
    actionCollection()->addAction("imageplugin_filmgrain", m_filmgrainAction );

    connect(m_filmgrainAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFilmGrain()));

    setXMLFile( "digikamimageplugin_filmgrain_ui.rc" );

    kDebug() << "ImagePlugin_FilmGrain plugin loaded";
}

ImagePlugin_FilmGrain::~ImagePlugin_FilmGrain()
{
}

void ImagePlugin_FilmGrain::setEnabledActions(bool enable)
{
    m_filmgrainAction->setEnabled(enable);
}

void ImagePlugin_FilmGrain::slotFilmGrain()
{
    FilmGrainTool *tool = new FilmGrainTool(this);
    loadTool(tool);
}
