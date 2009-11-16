/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a
 *               template to an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


#include "imageplugin_superimpose.moc"

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

#include "superimposetool.h"

using namespace DigikamSuperImposeImagesPlugin;

K_PLUGIN_FACTORY( SuperImposeFactory, registerPlugin<ImagePlugin_SuperImpose>(); )
K_EXPORT_PLUGIN ( SuperImposeFactory("digikamimageplugin_superimpose") )

ImagePlugin_SuperImpose::ImagePlugin_SuperImpose(QObject *parent, const QVariantList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_SuperImpose")
{
    m_superimposeAction  = new KAction(KIcon("superimpose"), i18n("Template Superimpose..."), this);
    actionCollection()->addAction("imageplugin_superimpose", m_superimposeAction );

    connect(m_superimposeAction, SIGNAL(triggered(bool)),
            this, SLOT(slotSuperImpose()));

    setXMLFile("digikamimageplugin_superimpose_ui.rc");

    kDebug() << "ImagePlugin_SuperImpose plugin loaded";
}

ImagePlugin_SuperImpose::~ImagePlugin_SuperImpose()
{
}

void ImagePlugin_SuperImpose::setEnabledActions(bool enable)
{
    m_superimposeAction->setEnabled(enable);
}

void ImagePlugin_SuperImpose::slotSuperImpose()
{
    SuperImposeTool *tool = new SuperImposeTool(this);
    loadTool(tool);
}
