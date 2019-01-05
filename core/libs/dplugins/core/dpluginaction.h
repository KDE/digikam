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

#ifndef DIGIKAM_DPLUGIN_ACTION_H
#define DIGIKAM_DPLUGIN_ACTION_H

// Qt includes

#include <QString>
#include <QAction>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginAction : public QAction
{
public:

    /// Plugin action types to resume where they can be used.
    enum ActionType
    {
        InvalidType = -1,

        Generic = 0,            /// Generic action available everywhere (AlbumView, Editor, and LightTable).
        Editor,                 /// Specific action for Image Editor and Showfoto.
        Bqm                     /// Specific action for Batch Queue Manager.
    };

    /// Plugin action categories.
    enum ActionCategory
    {
        InvalidCat    = -1,

        GenericExport = 0,      /// Generic export action.
        GenericImport,          /// Generic import action.
        GenericTool,            /// Generic processing action.
        GenericMetadata,        /// Generic Metadata adjustement action.

        EditorColor,            /// Image Editor color correction action.
        EditorEnhance,          /// Image Editor enhance action.
        EditorTransform,        /// Image Editor transform action.
        EditorDecorate,         /// Image Editor decorate action.
        EditorEffects,          /// Image Editor special effects action.

        BqmColor,               /// Batch Queue Manager color action.
        BqmEnhance,             /// Batch Queue Manager enhance action.
        BqmTransform,           /// Batch Queue Manager transform action.
        BqmDecorate,            /// Batch Queue Manager decorate action.
        BqmFilters,             /// Batch Queue Manager filters action.
        BqmConvert,             /// Batch Queue Manager convert action.
        BqmMetadata,            /// Batch Queue Manager metadata action.
        BqmCustom               /// Batch Queue Manager custom action.
    };

public:

    explicit DPluginAction(QObject* const parent = 0);
    ~DPluginAction();

    /**
     * Manage the internal action category.
     */
    void setActionCategory(ActionCategory cat);
    ActionCategory actionCategory() const;
    QString actionCategoryToString() const;

    /**
     * Return the action type depending of category.
     */
    ActionType actionType() const;

    /**
     * Return the plugin id string hosting this action.
     */
    QString pluginId() const;

    /**
     * Return the XML section to merge in KXMLGUIClient host XML definition.
     */
    QString xmlSection() const;

    /**
     * Return details as string about action properties.
     * For debug purpose only.
     */
    QString toString() const;
};

} // namespace Digikam

#endif // DIGIKAM_DPLUGIN_ACTION_H
