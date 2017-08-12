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

#include "BorderEditTool.h"
#include "ToolsDockWidget.h"
#include "AbstractPhoto.h"

#include "BordersGroup.h"
#include "BorderDrawersLoader.h"

#include <QDebug>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFont>
#include <QColorDialog>
#include <QMetaProperty>
#include <QPushButton>

#include <klocalizedstring.h>

#include "KEditFactory.h"
#include "qttreepropertybrowser.h"

using namespace PhotoLayoutsEditor;

BorderEditTool::BorderEditTool(Scene * scene, QWidget * parent) :
    AbstractItemsListViewTool(i18n("Borders editor"), scene, Canvas::SingleSelcting, parent)
{
}

QStringList BorderEditTool::options() const
{
    return BorderDrawersLoader::registeredDrawers();
}

AbstractMovableModel * BorderEditTool::model()
{
    if (currentItem() && currentItem()->bordersGroup())
        return currentItem()->bordersGroup();
    return 0;
}

QObject * BorderEditTool::createItem(const QString & name)
{
    return BorderDrawersLoader::getDrawerByName(name);
}

QWidget * BorderEditTool::createEditor(QObject * item, bool createCommands)
{
    BorderDrawerInterface * drawer = qobject_cast<BorderDrawerInterface*>(item);
    if (!drawer)
        return 0;
    return BorderDrawersLoader::createEditor(drawer, createCommands);
}
