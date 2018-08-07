/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : action container for external plugin
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dpluginaction.h"

// Qt includes

#include <QVariant>

namespace Digikam
{

DPluginAction::DPluginAction(QObject* const parent)
    : QAction(parent)
{
}

DPluginAction::~DPluginAction()
{
}

void DPluginAction::setActionName(const QString& name)
{
    setProperty("DPluginActionName", name);
}

QString DPluginAction::actionName() const
{
    return (property("DPluginActionName").toString());
}

void DPluginAction::setActionType(ActionType type)
{
    setProperty("DPluginActionType", (int)type);
}

DPluginAction::ActionType DPluginAction::actionType() const
{
    bool b = false;
    int v  = property("DPluginActionType").toInt(&b);

    if (b) return (ActionType)v;

    return InvalidType;
}

void DPluginAction::setActionCategory(ActionCategory cat)
{
    setProperty("DPluginActionCategory", (int)cat);
}

DPluginAction::ActionCategory DPluginAction::actionCategory() const
{
    bool b = false;
    int v  = property("DpluginActionCategory").toInt(&b);

    if (b) return (ActionCategory)v;

    return InvalidCat;
}

QString DPluginAction::xmlSection() const
{
    return QString::fromLatin1("<Action name=\"%1\" />\n").arg(actionName());
}

QString DPluginAction::toString() const
{
    return QString::fromUtf8("%1: \"%2\"").arg(actionName()).arg(text());
}

} // namespace Digikam
