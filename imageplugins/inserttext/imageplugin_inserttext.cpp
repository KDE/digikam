/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-01-20
 * Description : 
 * 
 * Copyright 2005-2007 by Gilles Caulier 
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
#include "imageeffect_inserttext.h"
#include "imageplugin_inserttext.h"
#include "imageplugin_inserttext.moc"

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_inserttext,
                           KGenericFactory<ImagePlugin_InsertText>("digikamimageplugin_inserttext"));

ImagePlugin_InsertText::ImagePlugin_InsertText(QObject *parent, const char*, const QStringList &)
                      : Digikam::ImagePlugin(parent, "ImagePlugin_InsertText")
{
    m_insertTextAction = new KAction(i18n("Insert Text..."), "inserttext", 
                         CTRL+Key_T, 
                         this, SLOT(slotInsertText()),
                         actionCollection(), "imageplugin_inserttext");

    setXMLFile("digikamimageplugin_inserttext_ui.rc");
    
    DDebug() << "ImagePlugin_InsertText plugin loaded" << endl;
}

ImagePlugin_InsertText::~ImagePlugin_InsertText()
{
}

void ImagePlugin_InsertText::setEnabledActions(bool enable)
{
    m_insertTextAction->setEnabled(enable);
}

void ImagePlugin_InsertText::slotInsertText()
{
    DigikamInsertTextImagesPlugin::ImageEffect_InsertText dlg(parentWidget());
    dlg.exec();
}

