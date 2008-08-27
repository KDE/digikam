/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-25
 * Description : a digiKam image plugin to reduce
 *               vignetting on an image.
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
#include "antivignettingtool.h"
#include "imageplugin_antivignetting.h"
#include "imageplugin_antivignetting.moc"

using namespace DigikamAntiVignettingImagesPlugin;

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_antivignetting,
                           KGenericFactory<ImagePlugin_AntiVignetting>("digikamimageplugin_antivignetting"));

ImagePlugin_AntiVignetting::ImagePlugin_AntiVignetting(QObject *parent, const char*, const QStringList &)
                          : Digikam::ImagePlugin(parent, "ImagePlugin_AntiVignetting")
{
    m_antivignettingAction = new KAction(i18n("Vignetting Correction..."), "antivignetting", 0,
                                 this, SLOT(slotAntiVignetting()),
                                 actionCollection(), "imageplugin_antivignetting");

    setXMLFile("digikamimageplugin_antivignetting_ui.rc");

    DDebug() << "ImagePlugin_AntiVignetting plugin loaded" << endl;
}

ImagePlugin_AntiVignetting::~ImagePlugin_AntiVignetting()
{
}

void ImagePlugin_AntiVignetting::setEnabledActions(bool enable)
{
    m_antivignettingAction->setEnabled(enable);
}

void ImagePlugin_AntiVignetting::slotAntiVignetting()
{
    AntiVignettingTool *tool = new AntiVignettingTool(this);
    loadTool(tool);
}
