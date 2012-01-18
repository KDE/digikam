/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-16
 * Description : Maintenance tool using image info job as items processor.
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

#ifndef MAINTENANCEIMGINFJOBTOOL_H
#define MAINTENANCEIMGINFJOBTOOL_H

// Qt includes

#include <QString>

// Local includes

#include "maintenancetool.h"
#include "imageinfo.h"

namespace Digikam
{

class MaintenanceImgInfJobTool : public MaintenanceTool
{
    Q_OBJECT

public:

    MaintenanceImgInfJobTool(const QString& id, Mode mode=AllItems, int albumId=-1,
                             const ImageInfoList& list=ImageInfoList());
    virtual ~MaintenanceImgInfJobTool();

protected:

    void populateItemsToProcess();
    void processOne();
    bool isEmpty() const;

    virtual void gotNewImageInfoList(const ImageInfoList&) {};

private Q_SLOTS:

    void slotAlbumItemsInfo(const ImageInfoList&);
    void slotOneAlbumComplete();

private:

    class MaintenanceImgInfJobToolPriv;
    MaintenanceImgInfJobToolPriv* const d;
};

}  // namespace Digikam

#endif /* MAINTENANCEIMGINFJOBTOOL_H */
