/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at gmail dot com>
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

#include "imageplugin_localcontrast.moc"

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "localcontrasttool.h"

using namespace DigikamLocalContrastImagesPlugin;

K_PLUGIN_FACTORY( LocalContrastFactory, registerPlugin<ImagePlugin_LocalContrast>(); )
K_EXPORT_PLUGIN ( LocalContrastFactory("digikamimageplugin_localcontrast") )

ImagePlugin_LocalContrast::ImagePlugin_LocalContrast(QObject *parent, const QVariantList&)
                         : Digikam::ImagePlugin(parent, "ImagePlugin_LocalContrast")
{
    m_localContrastAction  = new KAction(KIcon("contrast"), i18n("Local Contrast..."), this);
    actionCollection()->addAction("imageplugin_localcontrast", m_localContrastAction );

    connect(m_localContrastAction, SIGNAL(triggered(bool)),
            this, SLOT(slotLocalContrast()));

    setXMLFile("digikamimageplugin_localcontrast_ui.rc");

    kDebug() << "ImagePlugin_LocalContrast plugin loaded";
}

ImagePlugin_LocalContrast::~ImagePlugin_LocalContrast()
{
}

void ImagePlugin_LocalContrast::setEnabledActions(bool enable)
{
    m_localContrastAction->setEnabled(enable);
}

void ImagePlugin_LocalContrast::slotLocalContrast()
{
    LocalContrastTool *tool = new LocalContrastTool(this);
    loadTool(tool);
}
