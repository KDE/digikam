/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-11
 * Description : a digiKam image editor plugin to correct
 *               image white balance
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


#include "imageplugin_whitebalance.h"
#include "imageplugin_whitebalance.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "whitebalancetool.h"
#include "debug.h"

using namespace DigikamWhiteBalanceImagesPlugin;

K_PLUGIN_FACTORY( WhiteBalanceFactory, registerPlugin<ImagePlugin_WhiteBalance>(); )
K_EXPORT_PLUGIN ( WhiteBalanceFactory("digikamimageplugin_whitebalance") )

ImagePlugin_WhiteBalance::ImagePlugin_WhiteBalance(QObject *parent, const QVariantList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_WhiteBalance")
{
    m_whitebalanceAction = new KAction(KIcon("whitebalance"), i18n("White Balance..."), this);
    m_whitebalanceAction->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_W));

    connect(m_whitebalanceAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotWhiteBalance()));

    actionCollection()->addAction("imageplugin_whitebalance", m_whitebalanceAction );

    setXMLFile("digikamimageplugin_whitebalance_ui.rc");

    kDebug(imagePluginsAreaCode) << "ImagePlugin_WhiteBalance plugin loaded";
}

ImagePlugin_WhiteBalance::~ImagePlugin_WhiteBalance()
{
}

void ImagePlugin_WhiteBalance::setEnabledActions(bool enable)
{
    m_whitebalanceAction->setEnabled(enable);
}

void ImagePlugin_WhiteBalance::slotWhiteBalance()
{
    WhiteBalanceTool *tool = new WhiteBalanceTool(this);
    loadTool(tool);
}
