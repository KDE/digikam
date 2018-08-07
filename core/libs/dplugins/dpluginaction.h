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

    /// Plugin action types to define where they can be used.
    enum ActionType
    {
        InvalidType = -1,
        GenericType = 0,         /// Generic plugins available everywhere (AlbumView, Editor, and LightTable).
        EditorType,              /// Specific plugins for Image Editor and Showfoto.
        BQMType                  /// Specific plugins for Batch Queue Manager.
    };

    /// Plugin action categories for the generic plugins action type.
    enum ActionCategory
    {
        InvalidCat       = -1,
        GenericExportCat = 0,    /// Generic export plugins.
        GenericImportCat,        /// Generic import plugins.
        GenericToolCat           /// Generic processing plugins.

        // TODO: extend categories for Editor and BQM.
    };

public:

    explicit DPluginAction(QObject* const parent = 0);
    ~DPluginAction();

    /**
     * Manage the internal action name.
     */
    void setActionName(const QString& name);
    QString actionName() const;

    /**
     * Manage the internal action type.
     */
    void setActionType(ActionType type);
    ActionType actionType() const;

    /**
     * Manage the internal action category.
     */
    void setActionCategory(ActionCategory cat);
    ActionCategory actionCategory() const;

    /**
     * Return the XML section to merge in KXMLGUIClient host XML definition.
     */
    QString xmlSection() const;

    /**
     * Return details as string details about action properties.
     */
    QString toString() const;
};

} // namespace Digikam

#endif // DIGIKAM_DPLUGIN_ACTION_H
