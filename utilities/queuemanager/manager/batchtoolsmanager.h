/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tools Manager.
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BATCHTOOLSMANAGER_H
#define BATCHTOOLSMANAGER_H

// Qt includes

#include <QObject>

// Local includes

#include "batchtool.h"

namespace Digikam
{

class BatchToolsManager : public QObject
{
    Q_OBJECT

public:

    explicit BatchToolsManager(QObject* const parent = 0);
    ~BatchToolsManager();

    void registerTool(BatchTool* const tool, QWidget* const w);
    void unregisterTool(BatchTool* const tool);

    BatchTool* findTool(const QString& name, BatchTool::BatchToolGroup group)           const;
    QWidget*   findSettingsWidget(const QString& name, BatchTool::BatchToolGroup group) const;

    BatchToolsList toolsList() const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* BATCHTOOLSMANAGER_H */
