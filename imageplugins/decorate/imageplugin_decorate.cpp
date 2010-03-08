/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a plugin to insert a text over an image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_decorate.moc"

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
#include "bordertool.h"

using namespace DigikamDecorateImagePlugin;

K_PLUGIN_FACTORY( DecorateFactory, registerPlugin<ImagePlugin_Decorate>(); )
K_EXPORT_PLUGIN ( DecorateFactory("digikamimageplugin_decorate") )

ImagePlugin_Decorate::ImagePlugin_Decorate(QObject *parent, const QVariantList &)
                    : ImagePlugin(parent, "ImagePlugin_Decorate")
{
    m_insertTextAction = new KAction(KIcon("insert-text"), i18n("Insert Text..."), this);
    m_insertTextAction->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_T));
    actionCollection()->addAction("imageplugin_inserttext", m_insertTextAction );
    connect(m_insertTextAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotInsertText()));

    m_borderAction = new KAction(KIcon("bordertool"), i18n("Add Border..."), this);
    actionCollection()->addAction("imageplugin_border", m_borderAction );
    connect(m_borderAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBorder()));            
            
    setXMLFile("digikamimageplugin_decorate_ui.rc");

    kDebug() << "ImagePlugin_Decorate plugin loaded";
}

ImagePlugin_Decorate::~ImagePlugin_Decorate()
{
}

void ImagePlugin_Decorate::setEnabledActions(bool b)
{
    m_insertTextAction->setEnabled(b);
    m_borderAction->setEnabled(b);
}

void ImagePlugin_Decorate::slotInsertText()
{
    InsertTextTool* tool = new InsertTextTool(this);
    loadTool(tool);
}

void ImagePlugin_Decorate::slotBorder()
{
    BorderTool* tool = new BorderTool(this);
    loadTool(tool);
}
