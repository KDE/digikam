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

#include "BorderChangeListener.h"
#include "BordersGroup.h"
#include "global.h"
#include "KEditFactory.h"

#include "qtpropertybrowser.h"
#include "qtvariantproperty.h"

#include <QUndoCommand>
#include <QMetaProperty>

#include <klocalizedstring.h>

#include "digikam_debug.h"

using namespace PhotoLayoutsEditor;

class PhotoLayoutsEditor::BorderChangeCommand : public QUndoCommand
{
        BorderDrawerInterface * drawer;
        QString propertyName;
        QVariant value;
    public:
        BorderChangeCommand(BorderDrawerInterface * drawer, QUndoCommand * parent = 0) :
            QUndoCommand(i18n("Border Change"), parent),
            drawer(drawer)
        {
        }
        virtual void redo()
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "BorderChangeCommand redo";
            QVariant temp = drawer->propertyValue(propertyName);
            drawer->setPropertyValue(propertyName, value);
            value = temp;
        }
        virtual void undo()
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "BorderChangeCommand undo";
            QVariant temp = drawer->propertyValue(propertyName);
            drawer->setPropertyValue(propertyName, value);
            value = temp;
        }
        void setPropertyValue(const QString & propertyName, const QVariant & value)
        {
            this->propertyName = propertyName;
            this->value = value;
        }
};

BorderChangeListener::BorderChangeListener(BorderDrawerInterface * drawer, QObject * parent, bool createCommands) :
    QObject(parent),
    drawer(drawer),
    command(0),
    createCommands(createCommands)
{
}

void BorderChangeListener::propertyChanged(QtProperty * property)
{
    if (!drawer)
        return;

    if (!command)
        command = new BorderChangeCommand(drawer);

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
    QtEnumPropertyManager * enumManager = qobject_cast<QtEnumPropertyManager*>(property->propertyManager());
    if (enumManager)
    {
        command->setPropertyValue(property->propertyName(), enumManager->enumNames(property).at(enumManager->value(property)));
        return;
    }
    QtVariantPropertyManager * variantManager = qobject_cast<QtVariantPropertyManager*>(property->propertyManager());
    if (variantManager)
    {
        command->setPropertyValue(property->propertyName(), variantManager->value(property));
        return;
    }
}

void BorderChangeListener::editingFinished()
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
