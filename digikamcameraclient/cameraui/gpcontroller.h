/* ============================================================
 * File  : gpcontroller.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-22
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

#ifndef GPCONTROLLER_H
#define GPCONTROLLER_H

#include <qobject.h>
#include <qthread.h>
#include <qmutex.h>

#include "mtqueue.h"
#include "gpcommand.h"
#include "cameratype.h"

class QString;
class QImage;
class GPCamera;
class ThumbnailSize;

class GPController : public QObject, public QThread
{
    Q_OBJECT

public:

    GPController(QObject *parent, const CameraType& ctype);
    ~GPController();

    void requestInitialize();
    void requestGetSubFolders(const QString& folder);
    void requestMakeFolder(const QString& folder,
                           const QString& newFolder);
    void requestDeleteFolder(const QString& folder);
    void requestGetItemsInfo(const QString& folder);
    void requestGetAllItemsInfo(const QString& folder);
    void requestGetThumbnail(const QString& folder,
                             const QString& imageName,
                             const ThumbnailSize& thumbSize);
    void requestDownloadItem(const QString& folder,
                             const QString& itemName,
                             const QString& saveFile);
    void requestDeleteItem(const QString& folder,
                           const QString& itemName);
    void requestDeleteAllItems(const QString& rootFolder);
    void requestUploadItem(const QString& folder,
                           const QString& localFile,
                           const QString& uploadName);
    void requestOpenItem(const QString& folder,
                         const QString& itemName,
                         const QString& saveFile);
    void requestOpenItemWithService(const QString& folder,
                                    const QString& itemName,
                                    const QString& saveFile,
                                    const QString& serviceName);
    void requestExifInfo(const QString& folder,
                         const QString& itemName);
    void cancel();
    void getInformation(QString& summary, QString& manual,
                        QString& about);
    

protected:

    void run();

private:

    void initialize();
    void getSubFolders(const QString& folder);
    void makeFolder(const QString& folder,
                    const QString& newFolder);
    void deleteFolder(const QString& folder);
    void getItemsInfo(const QString& folder);
    void getAllItemsInfo(const QString& folder);
    void getThumbnail(const QString& folder,
                      const QString& imageName,
                      const ThumbnailSize& thumbSize);
    void downloadItem(const QString& folder,
                      const QString& itemName,
                      const QString& saveFile);
    void deleteItem(const QString& folder,
                    const QString& itemName);
    void deleteAllItems(const QString& rootFolder);
    void uploadItem(const QString& folder,
                    const QString& uploadName,
                    const QString& localFile);
    void openItem(const QString& folder,
                  const QString& itemName,
                  const QString& saveFile);
    void openItemWithService(const QString& folder,
                             const QString& itemName,
                             const QString& saveFile,
                             const QString& serviceName);
    void exifInfo(const QString& folder,
                  const QString& itemName);
    void error(const QString& errorMsg);

    void scaleHighlightThumbnail(QImage& thumbnail,
                                 const ThumbnailSize& thumbSize);
    void showBusy(bool val);
    

    QObject  *parent_;
    GPCamera *camera_;
    QMutex    mutex_;
    MTQueue<GPCommand> cmdQueue_;
    bool      close_;

private slots:

    void slotStatusMsg(const QString& msg);
    void slotProgressVal(int val);
    void slotErrorMsg(const QString& msg);
    
};

#endif /* GPCONTROLLER_H */
