/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digikam image editor plugin to
 *               simulate charcoal drawing.
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


#include "imageplugin_charcoal.h"
#include "imageplugin_charcoal.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "charcoaltool.h"

using namespace DigikamCharcoalImagesPlugin;

K_PLUGIN_FACTORY( CharcoalFactory, registerPlugin<ImagePlugin_Charcoal>(); )
K_EXPORT_PLUGIN ( CharcoalFactory("digikamimageplugin_charcoal") )

ImagePlugin_Charcoal::ImagePlugin_Charcoal(QObject *parent, const QVariantList &)
                    : Digikam::ImagePlugin(parent, "ImagePlugin_Charcoal")
{
    m_charcoalAction  = new KAction(KIcon("charcoaltool"), i18n("Charcoal Drawing..."), this);
    actionCollection()->addAction("imageplugin_charcoal", m_charcoalAction  );

    connect(m_charcoalAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCharcoal()));

    setXMLFile( "digikamimageplugin_charcoal_ui.rc" );

    kDebug(50006) << "ImagePlugin_Charcoal plugin loaded" << endl;
}

ImagePlugin_Charcoal::~ImagePlugin_Charcoal()
{
}

void ImagePlugin_Charcoal::setEnabledActions(bool enable)
{
    m_charcoalAction->setEnabled(enable);
}

void ImagePlugin_Charcoal::slotCharcoal()
{
    CharcoalTool *tool = new CharcoalTool(this);
    loadTool(tool);
}
