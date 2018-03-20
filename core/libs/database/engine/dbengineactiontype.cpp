/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-20
 * Description : Database Engine container to wrap data types
 *
 * Copyright (C) 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "dbengineactiontype.h"

namespace Digikam
{

class DbEngineActionType::Private
{

public:
    Private()
        : isValue(false)
    {
    }

    bool     isValue;
    QVariant actionValue;
};

DbEngineActionType::DbEngineActionType()
    : d(new Private)
{
}

DbEngineActionType::DbEngineActionType(const DbEngineActionType& actionType)
    : d(new Private)
{
    d->isValue     = actionType.d->isValue;
    d->actionValue = actionType.d->actionValue;
}

DbEngineActionType::~DbEngineActionType()
{
    delete d;
}

DbEngineActionType DbEngineActionType::value(const QVariant& value)
{
    DbEngineActionType actionType;
    actionType.setValue(true);
    actionType.setActionValue(value);
    return actionType;
}

DbEngineActionType DbEngineActionType::fieldEntry(const QVariant& actionValue)
{
    DbEngineActionType actionType;
    actionType.setValue(false);
    actionType.setActionValue(actionValue);
    return actionType;
}

QVariant DbEngineActionType::getActionValue()
{
    return d->actionValue;
}

void DbEngineActionType::setActionValue(const QVariant& actionValue)
{
    d->actionValue = actionValue;
}

bool DbEngineActionType::isValue() const
{
    return d->isValue;
}

void DbEngineActionType::setValue(bool isValue)
{
    d->isValue = isValue;
}

}  // namespace Digikam
