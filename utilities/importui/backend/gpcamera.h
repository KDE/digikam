/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-21
 * Description : Gphoto2 camera interface
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef GPCAMERA_H
#define GPCAMERA_H

// Local includes

#include "dkcamera.h"

class QImage;

namespace Digikam
{

class GPStatus;
class DMetadata;

/** Gphoto2 camera Implementation of abstract type DKCamera
 */
class GPCamera : public DKCamera
{

public:

    GPCamera(const QString& title, const QString& model,
             const QString& port, const QString& path);
    ~GPCamera();

    QByteArray                 cameraMD5ID();
    DKCamera::CameraDriverType cameraDriverType();

    bool doConnect();

    void cancel();

    bool getFolders(const QString& folder);
    bool getItemsList(const QString& folder, QStringList& itemsList);
    bool getItemsInfoList(const QString& folder, bool useMetadata, CamItemInfoList& items);
    void getItemInfo(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata);

    bool getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail);
    bool getMetadata(const QString& folder, const QString& itemName, DMetadata& meta);

    bool setLockItem(const QString& folder, const QString& itemName, bool lock);

    bool downloadItem(const QString& folder, const QString& itemName, const QString& saveFile);
    bool deleteItem(const QString& folder, const QString& itemName);

    // recursively delete all items
    bool deleteAllItems(const QString& folder);

    bool uploadItem(const QString& folder, const QString& itemName, const QString& localFile, CamItemInfo& itemInfo);

    bool cameraSummary(QString& summary);
    bool cameraManual(QString& manual);
    bool cameraAbout(QString& about);

    bool getFreeSpace(unsigned long& kBSize, unsigned long& kBAvail);
    bool getPreview(QImage& preview);
    bool capture(CamItemInfo& itemInfo);

    // Public static methods shared with Setup Camera

    static int  autoDetect(QString& model, QString& port);
    static void getSupportedCameras(int& count, QStringList& clist);
    static void getSupportedPorts(QStringList& plist);
    static void getCameraSupportedPorts(const QString& model, QStringList& plist);
    static bool findConnectedUsbCamera(int vendorId, int productId, QString& model, QString& port);

private:

    /** Run getItemInfo implementation whithout to be wrapped into GPhoto context
     */
    void getItemInfoInternal(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata);

    static void printGphotoErrorDescription(int errorCode);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* GPCAMERA_H */
