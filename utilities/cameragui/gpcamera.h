/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-21
 * Description : Gphoto2 camera interface
 * 
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

// Local includes.

#include "dkcamera.h"

class QImage;

namespace Digikam
{

class GPCameraPrivate;
class GPStatus;

// Gphoto2 camera Implementation of abstract type DKCamera

class GPCamera : public DKCamera
{

public:

    GPCamera(const QString& title, const QString& model,
             const QString& port, const QString& path);
    ~GPCamera();

    bool thumbnailSupport();
    bool deleteSupport();
    bool uploadSupport();
    bool mkDirSupport();
    bool delDirSupport();

    bool doConnect();

    void cancel();

    void getAllFolders(const QString& folder, QStringList& subFolderList);
    bool getSubFolders(const QString& folder, QStringList& subFolderList);
    bool getItemsList(const QString& folder, QStringList& itemsList);
    bool getItemsInfoList(const QString& folder, GPItemInfoList& items, bool getImageDimensions = true);
    bool getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail);
    bool getExif(const QString& folder, const QString& itemName, char **edata, int& esize);

    bool setLockItem(const QString& folder, const QString& itemName, bool lock);

    bool downloadItem(const QString& folder, const QString& itemName, const QString& saveFile);
    bool deleteItem(const QString& folder, const QString& itemName);

    // recursively delete all items
    bool deleteAllItems(const QString& folder);

    bool uploadItem(const QString& folder, const QString& itemName, const QString& localFile,
                    GPItemInfo& itemInfo, bool getImageDimensions=true);

    bool cameraSummary(QString& summary);
    bool cameraManual(QString& manual);
    bool cameraAbout(QString& about);

    QString model() const;
    QString port()  const;
    QString path()  const;
    
    // Public static methods shared with Camera Setup
    
    static int  autoDetect(QString& model, QString& port);
    static void getSupportedCameras(int& count, QStringList& clist);
    static void getSupportedPorts(QStringList& plist);
    static void getCameraSupportedPorts(const QString& model, QStringList& plist);

private:

    int setup();
    static void printGphotoErrorDescription(int errorCode);

private:

    GPCameraPrivate *d;
    GPStatus        *m_status;
};

}  // namespace Digikam

#endif /* GPCAMERA_H */
