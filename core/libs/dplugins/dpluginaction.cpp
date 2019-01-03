/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : action container for external plugin
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "digikam_debug.h"

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

DPluginAction::ActionType DPluginAction::actionType() const
{
    switch (actionCategory())
    {
        case GenericExport:
        case GenericImport:
        case GenericTool:
        case GenericMetadata:
            return Generic;

        case EditorColor:
        case EditorEnhance:
        case EditorTransform:
        case EditorDecorate:
        case EditorEffects:
            return Editor;

        case BqmColor:
        case BqmEnhance:
        case BqmTransform:
        case BqmDecorate:
        case BqmFilters:
        case BqmConvert:
        case BqmMetadata:
        case BqmCustom:
            return Bqm;

        default:
            break;
    }

    return InvalidType;
}

void DPluginAction::setActionCategory(ActionCategory cat)
{
    setProperty("DPluginActionCategory", (int)cat);
}

DPluginAction::ActionCategory DPluginAction::actionCategory() const
{
    bool b = false;
    int v  = property("DPluginActionCategory").toInt(&b);

    if (b) return (ActionCategory)v;

    return InvalidCat;
}

QString DPluginAction::actionCategoryToString() const
{
    switch (actionCategory())
    {
        case GenericExport:
            return QLatin1String("GenericExport");
        case GenericImport:
            return QLatin1String("GenericImport");
        case GenericTool:
            return QLatin1String("GenericTool");
        case GenericMetadata:
            return QLatin1String("GenericMetadata");

        case EditorColor:
            return QLatin1String("EditorColor");
        case EditorEnhance:
            return QLatin1String("EditorEnhance");
        case EditorTransform:
            return QLatin1String("EditorTransform");
        case EditorDecorate:
            return QLatin1String("EditorDecorate");
        case EditorEffects:
            return QLatin1String("EditorEffects");

        case BqmColor:
            return QLatin1String("BqmColor");
        case BqmEnhance:
            return QLatin1String("BqmEnhance");
        case BqmTransform:
            return QLatin1String("BqmTransform");
        case BqmDecorate:
            return QLatin1String("BqmDecorate");
        case BqmFilters:
            return QLatin1String("BqmFilters");
        case BqmConvert:
            return QLatin1String("BqmConvert");
        case BqmMetadata:
            return QLatin1String("BqmMetadata");
        case BqmCustom:
            return QLatin1String("BqmCustom");

        default:
            break;
    }

    return QLatin1String("Invalid");
}

QString DPluginAction::xmlSection() const
{
    return QString::fromLatin1("<Action name=\"%1\" />\n").arg(actionName());
}

QString DPluginAction::toString() const
{
    return QString::fromUtf8("%1: \"%2\" - %3").arg(actionName())
                                               .arg(text())
                                               .arg(actionCategoryToString());
}

QString DPluginAction::pluginId() const
{
    return property("DPluginId").toString();
}

} // namespace Digikam
