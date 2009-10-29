/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Plugin for Digikam using Liquid Rescale Library.
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_contentawareresizing.h"
#include "imageplugin_contentawareresizing.moc"

// KDE includes

#include <kgenericfactory.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>

// Local includes

#include "contentawareresizetool.h"
#include "debug.h"

using namespace DigikamContentAwareResizingImagesPlugin;

K_PLUGIN_FACTORY( ContentAwareResizingFactory, registerPlugin<ImagePlugin_ContentAwareResizing>(); )
K_EXPORT_PLUGIN ( ContentAwareResizingFactory("digikamimageplugin_contentawareresizing") )

ImagePlugin_ContentAwareResizing::ImagePlugin_ContentAwareResizing(QObject *parent, const QVariantList &)
                                : Digikam::ImagePlugin(parent, "ImagePlugin_ContentAwareResizing")
{
    m_contentAwareResizingAction = new KAction(KIcon("transform-scale"), i18n("Liquid Rescale..."), this);
    // m_contentAwareResizingAction->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_C));

    connect(m_contentAwareResizingAction, SIGNAL(triggered(bool)),
            this, SLOT(slotContentAwareResizing()));

    actionCollection()->addAction("imageplugin_contentawareresizing", m_contentAwareResizingAction);
    setXMLFile("digikamimageplugin_contentawareresizing_ui.rc");

    kDebug() << "ImagePlugin_ContentAwareResizing plugin loaded";
}

ImagePlugin_ContentAwareResizing::~ImagePlugin_ContentAwareResizing()
{
}

void ImagePlugin_ContentAwareResizing::setEnabledActions(bool enable)
{
    m_contentAwareResizingAction->setEnabled(enable);
}

void ImagePlugin_ContentAwareResizing::slotContentAwareResizing()
{
    ContentAwareResizeTool *tool = new ContentAwareResizeTool(this);
    loadTool(tool);
}
