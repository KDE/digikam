/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : Generic digiKam plugin definition.
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

#ifndef DIGIKAM_DPLUGIN_GENERIC_H
#define DIGIKAM_DPLUGIN_GENERIC_H

// Qt includes

#include <QWidget>

// Local includes

#include "dinfointerface.h"
#include "dplugin.h"
#include "dpluginaction.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginGeneric : public DPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPluginGeneric(QObject* const parent = 0);

    /**
     * Destructor
     */
    ~DPluginGeneric() override;

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
     * Return the info interface instance for the given action object.
     */
    DInfoInterface* infoIface(QObject* const ac) const;

    /**
     * Helper function to reactivate the desktop visibility of tool widget.
     */
    bool reactivateToolDialog(QWidget* const dlg) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_INTERFACE(Digikam::DPluginGeneric, "org.kde.digikam.DPluginGeneric/1.1.0" )

#endif // DIGIKAM_DPLUGIN_GENERIC_H
