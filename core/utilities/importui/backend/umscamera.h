/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : USB Mass Storage camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef UMSCAMERA_H
#define UMSCAMERA_H

// Qt includes

#include <QStringList>

// Local includes

#include "dkcamera.h"

namespace Digikam
{

class DMetadata;

/** USB Mass Storage camera Implementation of abstract type DKCamera
 */
class UMSCamera : public DKCamera
{
public:

    UMSCamera(const QString& title, const QString& model, const QString& port, const QString& path);
    ~UMSCamera();

    QByteArray                 cameraMD5ID();
    DKCamera::CameraDriverType cameraDriverType();

    bool doConnect();
    void cancel();

    bool getFolders(const QString& folder);
    bool getItemsInfoList(const QString& folder, bool useMetadata, CamItemInfoList& infoList);
    void getItemInfo(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata);

    bool getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail);
    bool getMetadata(const QString& folder, const QString& itemName, DMetadata& meta);

    bool setLockItem(const QString& folder, const QString& itemName, bool lock);

    bool downloadItem(const QString& folder, const QString& itemName, const QString& saveFile);
    bool deleteItem(const QString& folder, const QString& itemName);
    bool uploadItem(const QString& folder, const QString& itemName, const QString& localFile, CamItemInfo& info);

    bool cameraSummary(QString& summary);
    bool cameraManual(QString& manual);
    bool cameraAbout(QString& about);

    bool getFreeSpace(unsigned long& kBSize, unsigned long& kBAvail);

    bool getPreview(QImage& preview);
    bool capture(CamItemInfo& itemInfo);

private:

    void listFolders(const QString& folder, QStringList& subFolderList);

    /** Try to find UUID of UMS camera media using Solid interface.
        Search use mount path as reference. Return UUID as string
        else an empty string
     */
    void getUUIDFromSolid();

private:

    bool m_cancel;
};

}  // namespace Digikam

#endif /* UMSCAMERA_H */
