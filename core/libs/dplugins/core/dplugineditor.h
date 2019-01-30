/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : Image Editor digiKam plugin definition.
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

#ifndef DIGIKAM_DPLUGIN_EDITOR_H
#define DIGIKAM_DPLUGIN_EDITOR_H

// Qt includes

#include <QWidget>

// Local includes

#include "dinfointerface.h"
#include "dplugin.h"
#include "dpluginaction.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginEditor : public DPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPluginEditor(QObject* const parent = 0);

    /**
     * Destructor
     */
    ~DPluginEditor() override;

public:

    /**
     * Holds whether the plugin can be seen in parent view.
     */
    void setVisible(bool b);

    /**
     * Return all plugin actions registered in setup() method with addAction() for a given parent.
     */
    QList<DPluginAction*> actions(QObject* const parent) const;

    /**
     * Return the amount of tools registered to all parents.
     */
    int count() const;

    /**
     * Return a plugin action instance found by name in plugin action list for a given parent.
     */
    DPluginAction* findActionByName(const QString& name, QObject* const parent) const;

    /**
     * Return a list of categories as strings registered in this plugin.
     */
    QStringList categories() const;

protected:

    void addAction(DPluginAction* const ac);

    /**
     * Return the info interface instance for the given action.
     */
    DInfoInterface* infoIface(QObject* const ac) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_INTERFACE(Digikam::DPluginEditor, "org.kde.digikam.DPluginEditor/1.0.0" )

#endif // DIGIKAM_DPLUGIN_EDITOR_H
