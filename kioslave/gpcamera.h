/* ============================================================
 * File  : gpcamera.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-21
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

class QImage;

class GPCameraPrivate;
class GPStatus;
class DigikamCamera;

class GPCamera
{

public:

    enum {
        GPError=0,
        GPInit,
        GPSetup,
        GPSuccess
    } Errors;
    

    GPCamera(DigikamCamera* parent,
             const QString& model,
             const QString& port,
             const QString& path);
    ~GPCamera();

    bool thumbnailSupport();
    bool deleteSupport();
    bool uploadSupport();
    bool mkDirSupport();
    bool delDirSupport();

    
    int  initialize();

    void cancel();

    int  getSubFolders(const QString& folder,
                      QValueList<QString>& subFolderList);

    void getAllItemsInfo(const QString& folder);
    
    int getItemsInfo(const QString& folder);

    int getThumbnail(const QString& folder,
                     const QString& imageName,
                     QImage& thumbnail);

    int getExif(const QString& folder,
                const QString& imageName,
                char **edata, int& esize);

    int downloadItem(const QString& folder,
                     const QString& itemName,
                     const QString& saveFile);

    int deleteItem(const QString& folder,
                   const QString& itemName);

    // recursively delete all items
    int deleteAllItems(const QString& folder);

    int uploadItem(const QString& folder,
                   const QString& itemName,
                   const QString& localFile);

    void cameraSummary(QString& summary);
    void cameraManual(QString& manual);
    void cameraAbout(QString& about);

    
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

#endif /* GPCAMERA_H */
