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
            return QLatin1String("Generic/Export");
        case GenericImport:
            return QLatin1String("Generic/Import");
        case GenericTool:
            return QLatin1String("Generic/Tool");
        case GenericMetadata:
            return QLatin1String("Generic/Metadata");

        case EditorColor:
            return QLatin1String("Editor/Color");
        case EditorEnhance:
            return QLatin1String("Editor/Enhance");
        case EditorTransform:
            return QLatin1String("Editor/Transform");
        case EditorDecorate:
            return QLatin1String("Editor/Decorate");
        case EditorEffects:
            return QLatin1String("Editor/Effects");

        case BqmColor:
            return QLatin1String("Bqm/Color");
        case BqmEnhance:
            return QLatin1String("Bqm/Enhance");
        case BqmTransform:
            return QLatin1String("Bqm/Transform");
        case BqmDecorate:
            return QLatin1String("Bqm/Decorate");
        case BqmFilters:
            return QLatin1String("Bqm/Filters");
        case BqmConvert:
            return QLatin1String("Bqm/Convert");
        case BqmMetadata:
            return QLatin1String("Bqm/Metadata");
        case BqmCustom:
            return QLatin1String("Bqm/Custom");

        default:
            break;
    }

    return QLatin1String("Invalid");
}

QString DPluginAction::xmlSection() const
{
    return QString::fromLatin1("<Action name=\"%1\" />\n").arg(objectName());
}

QString DPluginAction::toString() const
{
    return QString::fromUtf8("%1: \"%2\" - %3").arg(objectName())
                                               .arg(text())
                                               .arg(actionCategoryToString());
}

QString DPluginAction::pluginId() const
{
    return property("DPluginId").toString();
}

} // namespace Digikam
