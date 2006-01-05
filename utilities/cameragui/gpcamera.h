/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-21
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

    GPCamera(const QString& model,
             const QString& port,
             const QString& path);
    ~GPCamera();

    bool thumbnailSupport();
    bool deleteSupport();
    bool uploadSupport();
    bool mkDirSupport();
    bool delDirSupport();

    bool connect();

    void cancel();

    void getAllFolders(const QString& folder, QStringList& subFolderList);
    bool getSubFolders(const QString& folder, QStringList& subFolderList);
    bool getItemsList(const QString& folder, QStringList& itemsList);
    bool getItemsInfoList(const QString& folder, GPItemInfoList& items);

    bool getThumbnail(const QString& folder, const QString& itemName,
                      QImage& thumbnail);
    bool getExif(const QString& folder, const QString& itemName,
                 char **edata, int& esize);

    bool downloadItem(const QString& folder,
                      const QString& itemName,
                      const QString& saveFile);

    bool deleteItem(const QString& folder,
                    const QString& itemName);

    // recursively delete all items
    bool deleteAllItems(const QString& folder);

    bool uploadItem(const QString& folder,
                    const QString& itemName,
                    const QString& localFile);

    void cameraSummary(QString& summary);
    void cameraManual(QString& manual);
    void cameraAbout(QString& about);

    QString model() const;
    QString port()  const;
    QString path()  const;
    
    // Static Functions
    
    static void getSupportedCameras(int& count, QStringList& clist);
    static void getSupportedPorts(QStringList& plist);
    static void getCameraSupportedPorts(const QString& model,
                                        QStringList& plist);
    static int  autoDetect(QString& model, QString& port);
    

private:

    int  setup();

    GPCameraPrivate *d;
    GPStatus        *status;
};

}  // namespace Digikam

#endif /* GPCAMERA_H */
