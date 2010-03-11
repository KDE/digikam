/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to enhance photograph
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_enhance.moc"

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

#include "restorationtool.h"

using namespace DigikamEnhanceImagePlugin;

K_PLUGIN_FACTORY( EnhanceFactory, registerPlugin<ImagePlugin_Enhance>(); )
K_EXPORT_PLUGIN ( EnhanceFactory("digikamimageplugin_enhance") )

ImagePlugin_Enhance::ImagePlugin_Enhance(QObject* parent, const QVariantList&)
                       : ImagePlugin(parent, "ImagePlugin_Enhance")
{
    m_restorationAction  = new KAction(KIcon("restoration"), i18n("Enhance..."), this);
    actionCollection()->addAction("imageplugin_restoration", m_restorationAction );

    connect(m_restorationAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRestoration()));

    setXMLFile( "digikamimageplugin_enhance_ui.rc" );

    kDebug() << "ImagePlugin_Enhance plugin loaded";
}

ImagePlugin_Enhance::~ImagePlugin_Enhance()
{
}

void ImagePlugin_Enhance::setEnabledActions(bool e)
{
    m_restorationAction->setEnabled(e);
}

void ImagePlugin_Enhance::slotRestoration()
{
    RestorationTool* tool = new RestorationTool(this);
    loadTool(tool);
}
