/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : Batch Queue Manager digiKam plugin definition.
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

#ifndef DIGIKAM_DPLUGIN_BQM_H
#define DIGIKAM_DPLUGIN_BQM_H

// Qt includes

#include <QWidget>

// Local includes

#include "dplugin.h"
#include "batchtool.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginBqm : public DPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPluginBqm(QObject* const parent = 0);

    /**
     * Destructor
     */
    ~DPluginBqm() override;

public:

    /**
     * Holds whether the plugin can be seen in parent view.
     */
    void setVisible(bool b);

    /**
     * Return all plugin tools registered in setup() method with addTool() for a given parent.
     */
    QList<BatchTool*> tools(QObject* const parent) const;

    /**
     * Return the amount of tools registered.
     */
    int toolCount() const;

    /**
     * Return a plugin tool instance found by name in plugin tools list for a given parent.
     */
    BatchTool* findToolByName(const QString& name, QObject* const parent) const;

    /**
     * Return a list of batch tool group as strings registered in this plugin.
     */
    QStringList batchToolGroup() const;

protected:

    void addTool(BatchTool* const t);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_INTERFACE(Digikam::DPluginBqm, "org.kde.digikam.DPluginBqm/1.0.0" )

#endif // DIGIKAM_DPLUGIN_BQM_H
