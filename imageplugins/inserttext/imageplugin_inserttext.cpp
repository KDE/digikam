/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a plugin to insert a text over an image.
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

// Local includes.

#include "ddebug.h"
#include "inserttexttool.h"
#include "imageplugin_inserttext.h"
#include "imageplugin_inserttext.moc"

using namespace DigikamInsertTextImagesPlugin;

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_inserttext,
                           KGenericFactory<ImagePlugin_InsertText>("digikamimageplugin_inserttext"));

ImagePlugin_InsertText::ImagePlugin_InsertText(QObject *parent, const char*, const QStringList &)
                      : Digikam::ImagePlugin(parent, "ImagePlugin_InsertText")
{
    m_insertTextAction = new KAction(i18n("Insert Text..."), "inserttext", 
                         SHIFT+CTRL+Key_T, 
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
    InsertTextTool *tool = new InsertTextTool(this);
    loadTool(tool);
}
