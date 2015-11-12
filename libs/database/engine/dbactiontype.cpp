/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-20
 * Description : DBActionType which wrappes other data types
 *
 * Copyright (C) 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
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

#include "dbactiontype.h"

namespace Digikam
{

class DBActionType::Private
{

public:
    Private()
        : isValue(false)
    {
    }

    bool     isValue;
    QVariant actionValue;
};

DBActionType::DBActionType()
    : d(new Private)
{
}

DBActionType::DBActionType(const DBActionType& actionType)
    : d(new Private)
{
    d->isValue     = actionType.d->isValue;
    d->actionValue = actionType.d->actionValue;
}

DBActionType::~DBActionType()
{
    delete d;
}

DBActionType DBActionType::value(const QVariant& value)
{
    DBActionType actionType;
    actionType.setValue(true);
    actionType.setActionValue(value);
    return actionType;
}

DBActionType DBActionType::fieldEntry(const QVariant& actionValue)
{
    DBActionType actionType;
    actionType.setValue(false);
    actionType.setActionValue(actionValue);
    return actionType;
}

QVariant DBActionType::getActionValue()
{
    return d->actionValue;
}

void DBActionType::setActionValue(const QVariant& actionValue)
{
    d->actionValue = actionValue;
}

bool DBActionType::isValue() const
{
    return d->isValue;
}

void DBActionType::setValue(bool isValue)
{
    d->isValue = isValue;
}

}  // namespace Digikam
