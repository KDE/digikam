/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-02-20
 * Description : Synchronized container for maintenance data.
 *
 * Copyright (C) 2017 by Mario Frank <mafrank at uni minus potsdam dot de>
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

#ifndef MAINTENANCE_DATA_H
#define MAINTENANCE_DATA_H

#include "imageinfo.h"

namespace FacesEngine
{
    class Identity;
}

namespace Digikam
{

using FacesEngine::Identity;

class ImageQualitySettings;

class MaintenanceData
{
public:
    explicit MaintenanceData();
    ~MaintenanceData();

    void      setImageIds(const QList<qlonglong>& ids);
    void      setThumbnailIds(const QList<int>& ids);
    void      setImagePaths(const QList<QString>& paths);
    void      setImageInfos(const QList<ImageInfo>& infos);
    void      setIdentities(const QList<Identity>& identities);

    qlonglong getImageId() const;
    int       getThumbnailId() const;
    QString   getImagePath() const;
    ImageInfo getImageInfo() const;
    Identity  getIdentity() const;

private:

    class Private;
    Private* const d;
};

}
#endif /* MAINTENANCE_DATA_H */
