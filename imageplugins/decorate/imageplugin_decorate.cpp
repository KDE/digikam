/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a plugin to insert a text over an image.
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "texturetool.h"

namespace DigikamDecorateImagePlugin
{

K_PLUGIN_FACTORY( DecorateFactory, registerPlugin<ImagePlugin_Decorate>(); )
K_EXPORT_PLUGIN ( DecorateFactory("digikamimageplugin_decorate") )

class ImagePlugin_Decorate::Private
{
public:

    Private() :
        textureAction(0),
        borderAction(0),
        insertTextAction(0)
    {
    }

    KAction* textureAction;
    KAction* borderAction;
    KAction* insertTextAction;
};

ImagePlugin_Decorate::ImagePlugin_Decorate(QObject* const parent, const QVariantList&)
    : ImagePlugin(parent, "ImagePlugin_Decorate"),
      d(new Private)
{
    d->insertTextAction = new KAction(KIcon("insert-text"), i18n("Insert Text..."), this);
    d->insertTextAction->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_T));
    actionCollection()->addAction("imageplugin_inserttext", d->insertTextAction );
    connect(d->insertTextAction, SIGNAL(triggered(bool)),
            this, SLOT(slotInsertText()));

    d->borderAction = new KAction(KIcon("bordertool"), i18n("Add Border..."), this);
    actionCollection()->addAction("imageplugin_border", d->borderAction );
    connect(d->borderAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBorder()));

    d->textureAction = new KAction(KIcon("texture"), i18n("Apply Texture..."), this);
    actionCollection()->addAction("imageplugin_texture", d->textureAction );
    connect(d->textureAction, SIGNAL(triggered(bool)),
            this, SLOT(slotTexture()));

    setActionCategory(i18n("Decorate"));
    setXMLFile("digikamimageplugin_decorate_ui.rc");

    kDebug() << "ImagePlugin_Decorate plugin loaded";
}

ImagePlugin_Decorate::~ImagePlugin_Decorate()
{
    delete d;
}

void ImagePlugin_Decorate::setEnabledActions(bool b)
{
    d->insertTextAction->setEnabled(b);
    d->borderAction->setEnabled(b);
    d->textureAction->setEnabled(b);
}

void ImagePlugin_Decorate::slotInsertText()
{
    loadTool(new InsertTextTool(this));
}

void ImagePlugin_Decorate::slotBorder()
{
    loadTool(new BorderTool(this));
}

void ImagePlugin_Decorate::slotTexture()
{
    loadTool(new TextureTool(this));
}

} // namespace DigikamDecorateImagePlugin
