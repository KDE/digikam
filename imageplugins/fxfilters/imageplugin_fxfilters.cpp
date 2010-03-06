/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin to apply special effects.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_fxfilters.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes

#include "colorfxtool.h"

using namespace DigikamFxFiltersImagePlugin;

K_PLUGIN_FACTORY( FxFiltersFactory, registerPlugin<ImagePlugin_FxFilters>(); )
K_EXPORT_PLUGIN ( FxFiltersFactory("digikamimageplugin_fxfilters") )

ImagePlugin_FxFilters::ImagePlugin_FxFilters(QObject* parent, const QVariantList&)
                     : ImagePlugin(parent, "ImagePlugin_FxFilters")
{
    m_colorEffectsAction = new KAction(KIcon("colorfx"), i18n("Color Effects..."), this);
    actionCollection()->addAction("imageplugin_colorfx", m_colorEffectsAction );
    connect(m_colorEffectsAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotColorEffects()));

    setXMLFile( "digikamimageplugin_fxfilters_ui.rc" );

    kDebug() << "ImagePlugin_FxFilters plugin loaded";
}

ImagePlugin_FxFilters::~ImagePlugin_FxFilters()
{
}

void ImagePlugin_FxFilters::setEnabledActions(bool b)
{
    m_colorEffectsAction->setEnabled(b);
}

void ImagePlugin_FxFilters::slotColorEffects()
{
    ColorFxTool* tool = new ColorFxTool(this);
    loadTool(tool);
}
