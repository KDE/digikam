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

// KDE includes.

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kcursor.h>

// Local includes.

#include "ddebug.h"
#include "adjustlevelstool.h"
#include "imageplugin_adjustlevels.h"
#include "imageplugin_adjustlevels.moc"

using namespace DigikamAdjustLevelsImagesPlugin;

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_adjustlevels,
                           KGenericFactory<ImagePlugin_AdjustLevels>("digikamimageplugin_adjustlevels"))

ImagePlugin_AdjustLevels::ImagePlugin_AdjustLevels(QObject *parent, const char*,
                                                   const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_AdjustLevels")
{
    m_levelsAction = new KAction(i18n("Levels Adjust..."), "adjustlevels", 
                                 CTRL+Key_L,         // NOTE: Photoshop 7 use CTRL+L. 
                                 this, SLOT(slotLevelsAdjust()),
                                 actionCollection(), "imageplugin_adjustlevels");

    setXMLFile("digikamimageplugin_adjustlevels_ui.rc");

    DDebug() << "ImagePlugin_AdjustLevels plugin loaded" << endl;
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
    AdjustLevelsTool *levels = new AdjustLevelsTool(this);
    loadTool(levels);
}
