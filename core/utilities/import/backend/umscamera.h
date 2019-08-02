/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : USB Mass Storage camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_UMS_CAMERA_H
#define DIGIKAM_UMS_CAMERA_H

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

    explicit UMSCamera(const QString& title, const QString& model, const QString& port, const QString& path);
    ~UMSCamera();

    QByteArray                 cameraMD5ID() override;
    DKCamera::CameraDriverType cameraDriverType() override;

    bool doConnect() override;
    void cancel() override;

    bool getFolders(const QString& folder) override;
    bool getItemsInfoList(const QString& folder, bool useMetadata, CamItemInfoList& infoList) override;
    void getItemInfo(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata) override;

    bool getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail) override;
    bool getMetadata(const QString& folder, const QString& itemName, DMetadata& meta) override;

    bool setLockItem(const QString& folder, const QString& itemName, bool lock) override;

    bool downloadItem(const QString& folder, const QString& itemName, const QString& saveFile) override;
    bool deleteItem(const QString& folder, const QString& itemName) override;
    bool uploadItem(const QString& folder, const QString& itemName, const QString& localFile, CamItemInfo& info) override;

    bool cameraSummary(QString& summary) override;
    bool cameraManual(QString& manual) override;
    bool cameraAbout(QString& about) override;

    bool getFreeSpace(unsigned long& kBSize, unsigned long& kBAvail) override;

    bool getPreview(QImage& preview) override;
    bool capture(CamItemInfo& itemInfo) override;

private:

    /** Try to find UUID of UMS camera media using Solid interface.
        Search use mount path as reference. Return UUID as string
        else an empty string
     */
    void getUUIDFromSolid();

private:

    bool m_cancel;
};

} // namespace Digikam

#endif // DIGIKAM_UMS_CAMERA_H
