/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to restore
 *               a photograph
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_restoration.h"
#include "imageplugin_restoration.moc"

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "restorationtool.h"

using namespace DigikamRestorationImagesPlugin;

K_PLUGIN_FACTORY( RestorationFactory, registerPlugin<ImagePlugin_Restoration>(); )
K_EXPORT_PLUGIN ( RestorationFactory("digikamimageplugin_restoration") )

ImagePlugin_Restoration::ImagePlugin_Restoration(QObject *parent, const QVariantList &)
                       : Digikam::ImagePlugin(parent, "ImagePlugin_Restoration")
{
    m_restorationAction  = new KAction(KIcon("restoration"), i18n("Restoration..."), this);
    actionCollection()->addAction("imageplugin_restoration", m_restorationAction );

    connect(m_restorationAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRestoration()));

    setXMLFile( "digikamimageplugin_restoration_ui.rc" );

    kDebug(50006) << "ImagePlugin_Restoration plugin loaded" << endl;
}

ImagePlugin_Restoration::~ImagePlugin_Restoration()
{
}

void ImagePlugin_Restoration::setEnabledActions(bool enable)
{
    m_restorationAction->setEnabled(enable);
}

void ImagePlugin_Restoration::slotRestoration()
{
    RestorationTool *tool = new RestorationTool(this);
    loadTool(tool);
}
