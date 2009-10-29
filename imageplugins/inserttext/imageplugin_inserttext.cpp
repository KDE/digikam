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


#include "imageplugin_inserttext.h"
#include "imageplugin_inserttext.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes

#include "inserttexttool.h"

using namespace DigikamInsertTextImagesPlugin;

K_PLUGIN_FACTORY( InsertTextFactory, registerPlugin<ImagePlugin_InsertText>(); )
K_EXPORT_PLUGIN ( InsertTextFactory("digikamimageplugin_inserttext") )

ImagePlugin_InsertText::ImagePlugin_InsertText(QObject *parent, const QVariantList &)
                      : Digikam::ImagePlugin(parent, "ImagePlugin_InsertText")
{
    m_insertTextAction = new KAction(KIcon("insert-text"), i18n("Insert Text..."), this);
    m_insertTextAction->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_T));

    connect(m_insertTextAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotInsertText()));

    actionCollection()->addAction("imageplugin_inserttext", m_insertTextAction );

    setXMLFile("digikamimageplugin_inserttext_ui.rc");

    kDebug() << "ImagePlugin_InsertText plugin loaded";
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
