/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-16
 * Description : Maintenance tool using picture paths list
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINTENANCEPICTPATHTOOL_H
#define MAINTENANCEPICTPATHTOOL_H

// Qt includes

#include <QString>

// Local includes

#include "maintenancetool.h"

namespace Digikam
{

class MaintenancePictPathTool : public MaintenanceTool
{
    Q_OBJECT

public:

    MaintenancePictPathTool(const QString& id, Mode mode=AllItems, int albumId=-1);
    virtual ~MaintenancePictPathTool();

protected:

    /** Return all paths to process. Data container can be customized.
     */
    QStringList& allPicturesPath();

    void populateItemsToProcess();
    bool isEmpty() const;

private:

    class MaintenancePictPathToolPriv;
    MaintenancePictPathToolPriv* const d;
};

}  // namespace Digikam

#endif /* MAINTENANCEPICTPATHTOOL_H */
