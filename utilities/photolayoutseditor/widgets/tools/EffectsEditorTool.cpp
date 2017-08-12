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

#include "EffectsEditorTool.h"
#include "AbstractPhoto.h"
#include "PhotoEffectsLoader.h"
#include "PhotoEffectsGroup.h"
#include "AbstractPhotoEffectInterface.h"
#include "ToolsDockWidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QComboBox>

#include <klocalizedstring.h>

#include "qtpropertybrowser.h"

using namespace PhotoLayoutsEditor;

EffectsEditorTool::EffectsEditorTool(Scene * scene, QWidget * parent) :
    AbstractItemsListViewTool(i18n("Effects"), scene, Canvas::SingleSelcting, parent)
{
}

QStringList EffectsEditorTool::options() const
{
    return PhotoEffectsLoader::registeredEffectsNames();
}

AbstractMovableModel * EffectsEditorTool::model()
{
    if (currentItem() && currentItem()->effectsGroup())
        return currentItem()->effectsGroup();
    return 0;
}

QObject * EffectsEditorTool::createItem(const QString & name)
{
    return PhotoEffectsLoader::getEffectByName(name);
}

QWidget * EffectsEditorTool::createEditor(QObject * item, bool createCommands)
{
    AbstractPhotoEffectInterface * effect = qobject_cast<AbstractPhotoEffectInterface*>(item);
    if (!effect)
        return 0;
    return PhotoEffectsLoader::propertyBrowser(effect, createCommands);
}
