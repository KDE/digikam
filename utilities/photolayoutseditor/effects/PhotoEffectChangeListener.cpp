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
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "PhotoEffectChangeListener.h"
#include "PhotoEffectsGroup.h"
#include "AbstractPhoto.h"
#include "global.h"

#include "qtpropertymanager.h"
#include "qtvariantproperty.h"

using namespace PhotoLayoutsEditor;

class PhotoLayoutsEditor::PhotoEffectChangeCommand : public QUndoCommand
{
        AbstractPhotoEffectInterface * effect;
        QString propertyName;
        QVariant value;
    public:
        PhotoEffectChangeCommand(AbstractPhotoEffectInterface * effect, QUndoCommand * parent = 0) :
            QUndoCommand(parent),
            effect(effect)
        {
        }
        virtual void redo()
        {
            QVariant temp = effect->propertyValue(propertyName);
            effect->setPropertyValue(propertyName, value);
            value = temp;
        }
        virtual void undo()
        {
            QVariant temp = effect->propertyValue(propertyName);
            effect->setPropertyValue(propertyName, value);
            value = temp;
        }
        void setPropertyValue(const QString & propertyName, const QVariant & value)
        {
            this->propertyName = propertyName;
            this->value = value;
        }
};

PhotoEffectChangeListener::PhotoEffectChangeListener(AbstractPhotoEffectInterface * effect, QObject * parent, bool createCommands) :
    QObject(parent),
    effect(effect),
    command(0),
    createCommands(createCommands)
{
}

void PhotoEffectChangeListener::propertyChanged(QtProperty * property)
{
    if (!effect)
        return;

    if (!command)
        command = new PhotoEffectChangeCommand(effect);

    QtIntPropertyManager * integerManager = qobject_cast<QtIntPropertyManager*>(property->propertyManager());
    if (integerManager)
    {
        command->setPropertyValue(property->propertyName(), integerManager->value(property));
        return;
    }
    QtDoublePropertyManager * doubleManager = qobject_cast<QtDoublePropertyManager*>(property->propertyManager());
    if (doubleManager)
    {
        command->setPropertyValue(property->propertyName(), doubleManager->value(property));
        return;
    }
    QtColorPropertyManager * colorManager = qobject_cast<QtColorPropertyManager*>(property->propertyManager());
    if (colorManager)
    {
        command->setPropertyValue(property->propertyName(), colorManager->value(property));
        return;
    }
    QtVariantPropertyManager * variantManager = qobject_cast<QtVariantPropertyManager*>(property->propertyManager());
    if (variantManager)
    {
        command->setPropertyValue(property->propertyName(), variantManager->value(property));
        return;
    }
}

void PhotoEffectChangeListener::editingFinished()
{
    if (command)
    {
        if (createCommands)
            PLE_PostUndoCommand(command);
        else
        {
            command->redo();
            delete command;
        }
    }
    command = 0;
}
