/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-11
 * Description : a digiKam image editor plugin to correct 
 *               image white balance 
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <kactioncollection.h>
#include <kcursor.h>
#include <kactioncollection.h>
// Local includes.

#include "ddebug.h"
#include "imageeffect_whitebalance.h"
#include "imageplugin_whitebalance.h"
#include "imageplugin_whitebalance.moc"

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_whitebalance,
                           KGenericFactory<ImagePlugin_WhiteBalance>("digikamimageplugin_whitebalance"));

ImagePlugin_WhiteBalance::ImagePlugin_WhiteBalance(QObject *parent, const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_WhiteBalance")
{
    m_whitebalanceAction  = new KAction(KIcon("whitebalance"), i18n("White Balance..."), this);
    actionCollection()->addAction("imageplugin_whitebalance", m_whitebalanceAction );
    connect(m_whitebalanceAction, SIGNAL(triggered(bool) ), SLOT(slotWhiteBalance()));
    m_whitebalanceAction->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_W));
    
    setXMLFile("digikamimageplugin_whitebalance_ui.rc");         
                                    
    DDebug() << "ImagePlugin_WhiteBalance plugin loaded" << endl;
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
    QString title = i18n("White Color Balance Correction");
    DigikamWhiteBalanceImagesPlugin::ImageEffect_WhiteBalance dlg(parentWidget());
    dlg.exec();
}
