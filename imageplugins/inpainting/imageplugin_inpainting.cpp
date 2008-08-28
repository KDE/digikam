/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint
 *               a photograph
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <kapplication.h>

// Local includes.

#include "ddebug.h"
#include "imageiface.h"
#include "inpaintingtool.h"
#include "imageplugin_inpainting.h"
#include "imageplugin_inpainting.moc"

using namespace DigikamInPaintingImagesPlugin;
using namespace Digikam;

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_inpainting,
                           KGenericFactory<ImagePlugin_InPainting>("digikamimageplugin_inpainting"));

ImagePlugin_InPainting::ImagePlugin_InPainting(QObject *parent, const char*, const QStringList &)
                      : Digikam::ImagePlugin(parent, "ImagePlugin_InPainting")
{
    m_inPaintingAction = new KAction(i18n("Inpainting..."), "inpainting",
                             CTRL+Key_E, 
                             this, SLOT(slotInPainting()),
                             actionCollection(), "imageplugin_inpainting");

    m_inPaintingAction->setWhatsThis( i18n( "This filter can be used to inpaint a part in a photo. "
                                            "Select a region to inpaint to use this option.") );

    setXMLFile( "digikamimageplugin_inpainting_ui.rc" );

    DDebug() << "ImagePlugin_InPainting plugin loaded" << endl;
}

ImagePlugin_InPainting::~ImagePlugin_InPainting()
{
}

void ImagePlugin_InPainting::setEnabledActions(bool enable)
{
    m_inPaintingAction->setEnabled(enable);
}

void ImagePlugin_InPainting::slotInPainting()
{

    ImageIface iface(0, 0);

    int w = iface.selectedWidth();
    int h = iface.selectedHeight();

    if (!w || !h)
    {
        InPaintingPassivePopup* popup = new InPaintingPassivePopup(kapp->activeWindow());
        popup->setView(i18n("Inpainting Photograph Tool"),
                       i18n("You need to select a region to inpaint to use "
                            "this tool"));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    // -- run the dlg ----------------------------------------------

    InPaintingTool *tool = new InPaintingTool(this);
    loadTool(tool);
}
