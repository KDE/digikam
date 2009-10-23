/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
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

#include "imageplugin_adjustcurves.h"
#include "imageplugin_adjustcurves.moc"

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>

// Local includes

#include "adjustcurvestool.h"
#include "debug.h"

using namespace DigikamAdjustCurvesImagesPlugin;

K_PLUGIN_FACTORY( AdjustCurvesFactory, registerPlugin<ImagePlugin_AdjustCurves>(); )
K_EXPORT_PLUGIN ( AdjustCurvesFactory("digikamimageplugin_adjustcurves") )

ImagePlugin_AdjustCurves::ImagePlugin_AdjustCurves(QObject *parent, const QList<QVariant>&)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_AdjustCurves")
{
    m_curvesAction  = new KAction(KIcon("adjustcurves"), i18n("Curves Adjust..."), this);
    actionCollection()->addAction("imageplugin_adjustcurves", m_curvesAction );

    connect(m_curvesAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotCurvesAdjust()));

    // NOTE: Photoshop 7 use CTRL+M (but it's used in KDE to toogle menu bar).
    m_curvesAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_M));
    setXMLFile("digikamimageplugin_adjustcurves_ui.rc");

    kDebug(imagePluginsAreaCode) << "ImagePlugin_AdjustCurves plugin loaded";
}

ImagePlugin_AdjustCurves::~ImagePlugin_AdjustCurves()
{
}

void ImagePlugin_AdjustCurves::setEnabledActions(bool enable)
{
    m_curvesAction->setEnabled(enable);
}

void ImagePlugin_AdjustCurves::slotCurvesAdjust()
{
    AdjustCurvesTool *tool = new AdjustCurvesTool(this);
    loadTool(tool);
}
