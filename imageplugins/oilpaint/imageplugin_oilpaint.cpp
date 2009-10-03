/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-25
 * Description : a plugin to simulate Oil Painting
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


#include "imageplugin_oilpaint.h"
#include "imageplugin_oilpaint.moc"

// KDE includes

#include <klocale.h>
#include <klibloader.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "oilpainttool.h"
#include "debug.h"

using namespace DigikamOilPaintImagesPlugin;

K_PLUGIN_FACTORY( OilPaintFactory, registerPlugin<ImagePlugin_OilPaint>(); )
K_EXPORT_PLUGIN ( OilPaintFactory("digikamimageplugin_oilpaint") )

ImagePlugin_OilPaint::ImagePlugin_OilPaint(QObject *parent, const QVariantList &)
                    : Digikam::ImagePlugin(parent, "ImagePlugin_OilPaint")
{
    m_oilpaintAction  = new KAction(KIcon("oilpaint"), i18n("Oil Paint..."), this);
    actionCollection()->addAction("imageplugin_oilpaint", m_oilpaintAction);

    connect(m_oilpaintAction, SIGNAL(triggered(bool) ),
            this ,SLOT(slotOilPaint()));

    setXMLFile( "digikamimageplugin_oilpaint_ui.rc" );

    kDebug(imagePluginsAreaCode) << "ImagePlugin_OilPaint plugin loaded";
}

ImagePlugin_OilPaint::~ImagePlugin_OilPaint()
{
}

void ImagePlugin_OilPaint::setEnabledActions(bool enable)
{
    m_oilpaintAction->setEnabled(enable);
}

void ImagePlugin_OilPaint::slotOilPaint()
{
    OilPaintTool *tool = new OilPaintTool(this);
    loadTool(tool);
}
