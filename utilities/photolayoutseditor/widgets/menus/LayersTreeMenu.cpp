/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "LayersTreeMenu.h"
#include "LayersTree.h"
#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

LayersTreeMenu::LayersTreeMenu(LayersTree * parent) :
    QMenu(parent)
{
    moveUpItems = this->addAction(i18n("Move up"));
    connect(moveUpItems, SIGNAL(triggered()), parent, SLOT(moveSelectedRowsUp()));
    moveDownItems = this->addAction(i18n("Move down"));
    connect(moveDownItems, SIGNAL(triggered()), parent, SLOT(moveSelectedRowsDown()));
    this->addSeparator();
    deleteItems = this->addAction(i18n("Delete selected"));
    connect(deleteItems, SIGNAL(triggered()), parent, SLOT(removeSelectedRows()));
}
