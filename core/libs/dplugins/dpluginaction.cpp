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
        case GenericExportCat:
        case GenericImportCat:
        case GenericToolCat:
            return GenericType;

        case EditorColor:
        case EditorEnhance:
        case EditorTransform:
        case EditorDecorate:
        case EditorEffects:
            return EditorType;

        case BqmColor:
        case BqmEnhance:
        case BqmTransform:
        case BqmDecorate:
        case BqmFilters:
        case BqmConvert:
        case BqmMetadata:
        case BqmCustom:
            return BqmType;
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
        case GenericExportCat:
            return QLatin1String("GenericExportCat");
        case GenericImportCat:
            return QLatin1String("GenericImportCat");
        case GenericToolCat:
            return QLatin1String("GenericToolCat");

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
    }

    return QLatin1String("InvalidCat");
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

} // namespace Digikam
