/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : image histogram adjust levels.
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


#include "imageplugin_adjustlevels.h"
#include "imageplugin_adjustlevels.moc"

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>

// Local includes

#include "adjustlevelstool.h"
#include "debug.h"

using namespace DigikamAdjustLevelsImagesPlugin;

K_PLUGIN_FACTORY( AdjustLevelsFactory, registerPlugin<ImagePlugin_AdjustLevels>(); )
K_EXPORT_PLUGIN ( AdjustLevelsFactory("digikamimageplugin_adjustlevels") )

ImagePlugin_AdjustLevels::ImagePlugin_AdjustLevels(QObject *parent, const QVariantList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_AdjustLevels")
{
    m_levelsAction  = new KAction(KIcon("adjustlevels"), i18n("Levels Adjust..."), this);
    m_levelsAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_L));
    actionCollection()->addAction("imageplugin_adjustlevels", m_levelsAction );

    connect(m_levelsAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotLevelsAdjust()));

    setXMLFile("digikamimageplugin_adjustlevels_ui.rc");

    kDebug() << "ImagePlugin_AdjustLevels plugin loaded";
}

ImagePlugin_AdjustLevels::~ImagePlugin_AdjustLevels()
{
}

void ImagePlugin_AdjustLevels::setEnabledActions(bool enable)
{
    m_levelsAction->setEnabled(enable);
}

void ImagePlugin_AdjustLevels::slotLevelsAdjust()
{
    AdjustLevelsTool *tool = new AdjustLevelsTool(this);
    loadTool(tool);
}
