/* ============================================================
 * File  : gpcontroller.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-22
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 *
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

// C ansi includes.
 
#include <iostream>

// Qt includes.

#include <qapplication.h>
#include <qstring.h>
#include <qimage.h>
#include <qcolor.h>

// KDE includes.

#include <klocale.h>
#include <kdebug.h>

// Local includes.

#include "gpfileiteminfo.h"
#include "thumbnailsize.h"
#include "mtlist.h"
#include "gpcamera.h"
#include "gpevents.h"
#include "gpmessages.h"
#include "gpcontroller.h"


GPController::GPController(QObject *parent, const CameraType& ctype)
    : QObject(parent)
{
    parent_ = parent;
    camera_ = new GPCamera(QString(ctype.model().latin1()),
                           QString(ctype.port().latin1()),
                           QString(ctype.path().latin1()));
    close_ = false;

    connect(GPMessages::gpMessagesWrapper(), SIGNAL(statusChanged(const QString&)),
            this, SLOT(slotStatusMsg(const QString&)) );

    connect(GPMessages::gpMessagesWrapper(), SIGNAL(progressChanged(int)),
            this, SLOT(slotProgressVal(int)) );

    connect(GPMessages::gpMessagesWrapper(), SIGNAL(errorMessage(const QString&)),
            this, SLOT(slotErrorMsg(const QString&)));

    
}

GPController::~GPController()
{
    close_ = true;
    wait();

    cmdQueue_.flush();
    GPMessages::deleteMessagesWrapper();
    delete camera_;
    
}

void GPController::requestInitialize()
{
    cmdQueue_.enqueue(new GPCommand(GPCommand::Init));    
}

void GPController::requestGetSubFolders(const QString& folder)
{
    cmdQueue_.enqueue(new GPCommandGetSubFolders(folder));    
}

void GPController::requestMakeFolder(const QString& folder,
                                     const QString& newFolder)
{
    cmdQueue_.enqueue(new GPCommandMakeFolder(folder,
                                              newFolder));
}

void GPController::requestDeleteFolder(const QString& folder)
{
    cmdQueue_.enqueue(new GPCommandDeleteFolder(folder));
}

void GPController::requestGetItemsInfo(const QString& folder)
{
    cmdQueue_.enqueue(new GPCommandGetItemsInfo(folder));    
}

void GPController::requestGetAllItemsInfo(const QString& folder)
{
    cmdQueue_.enqueue(new GPCommandGetAllItemsInfo(folder));    
}

void GPController::requestGetThumbnail(const QString& folder,
                                       const QString& imageName,
                                       const ThumbnailSize& thumbSize)
{
    cmdQueue_.enqueue(new GPCommandGetThumbnail(folder, imageName,
                                                thumbSize));
}

void GPController::requestDownloadItem(const QString& folder,
                                       const QString& itemName,
                                       const QString& saveFile)
{
    cmdQueue_.enqueue(new GPCommandDownloadItem(folder, itemName,
                                                saveFile));
}

void GPController::requestDeleteItem(const QString& folder,
                                     const QString& itemName)
{
    cmdQueue_.enqueue(new GPCommandDeleteItem(folder, itemName));
}

void GPController::requestDeleteAllItems(const QString& rootFolder)
{
    cmdQueue_.enqueue(new GPCommandDeleteAllItems(rootFolder));
}

void GPController::requestUploadItem(const QString& folder,
                                     const QString& localFile,
                                     const QString& uploadName)
{
    cmdQueue_.enqueue(new GPCommandUploadItem(folder, localFile,
                                              uploadName));
}

void GPController::requestOpenItem(const QString& folder,
                                   const QString& itemName,
                                   const QString& saveFile)
{
    cmdQueue_.enqueue(new GPCommandOpenItem(folder, itemName,
                                            saveFile));    
}

void GPController::requestOpenItemWithService(const QString& folder,
                                              const QString& itemName,
                                              const QString& saveFile,
                                              const QString& serviceName)
{
    cmdQueue_.enqueue(new GPCommandOpenItemWithService(folder,
                                                       itemName,
                                                       saveFile,
                                                       serviceName));
}

void GPController::requestExifInfo(const QString& folder,
                                   const QString& itemName)
{
    cmdQueue_.enqueue(new GPCommandExifInfo(folder,
                                            itemName));
}

void GPController::cancel()
{
    cmdQueue_.flush();
    mutex_.lock();
    camera_->cancel();
    mutex_.unlock();
}

void GPController::run()
{
    while (true) {

        if (cmdQueue_.isEmpty())
            showBusy(false);

        if (close_) return;

        while(cmdQueue_.isEmpty()) {
            if (close_) return;
            msleep(200);
        }

        GPCommand *cmd = cmdQueue_.dequeue();
        if (!cmd) continue;

        showBusy(true);
        
        switch(cmd->type()) {

        case(GPCommand::Init): {
            initialize();
            break;
        }

        case(GPCommand::GetSubFolders): {
            GPCommandGetSubFolders *command
                = static_cast<GPCommandGetSubFolders *>(cmd);
            getSubFolders(command->folder());
            delete command;
            cmd = 0;
            break;
        }

        case(GPCommand::GetItemsInfo): {
            GPCommandGetItemsInfo *command
                = static_cast<GPCommandGetItemsInfo *>(cmd);
            getItemsInfo(command->folder());
            delete command;
            cmd = 0;
            break;
        }

        case(GPCommand::GetAllItemsInfo): {
            GPCommandGetAllItemsInfo *command
                = static_cast<GPCommandGetAllItemsInfo *>(cmd);
            getAllItemsInfo(command->folder());
            delete command;
            cmd = 0;
            break;
        }

        case(GPCommand::GetThumbnail): {
            GPCommandGetThumbnail *command
                = static_cast<GPCommandGetThumbnail *>(cmd);
            getThumbnail(command->folder(), command->imageName(),
                         command->thumbSize());
            delete command;
            cmd = 0;
            break;
        }

        case(GPCommand::DownloadItem): {
            GPCommandDownloadItem *command
                = static_cast<GPCommandDownloadItem *>(cmd);
            downloadItem(command->folder(), command->itemName(),
                         command->saveFile());
            delete command;
            cmd = 0;
            break;
        }

        case(GPCommand::DeleteItem): {
            GPCommandDeleteItem *command
                = static_cast<GPCommandDeleteItem *>(cmd);
            deleteItem(command->folder(), command->itemName());
            delete command;
            cmd = 0;
            break;
        }

        case(GPCommand::DeleteAllItems): {
            GPCommandDeleteAllItems *command
                = static_cast<GPCommandDeleteAllItems *>(cmd);
            deleteAllItems(command->rootFolder());
            break;
        }

        case(GPCommand::UploadItem): {
            GPCommandUploadItem *command
                = static_cast<GPCommandUploadItem *>(cmd);
            uploadItem(command->folder(), command->uploadName(),
                       command->localFile());
            delete command;
            cmd = 0;
            break;
        }

        case(GPCommand::OpenItem): {
            GPCommandOpenItem *command
                = static_cast<GPCommandOpenItem *>(cmd);
            openItem(command->folder(), command->itemName(),
                     command->saveFile());
            delete command;
            cmd = 0;
            break;
        }
            
        case(GPCommand::OpenItemWithService): {
            GPCommandOpenItemWithService *command
                = static_cast<GPCommandOpenItemWithService *>(cmd);
            openItemWithService(command->folder(),
                                command->itemName(),
                                command->saveFile(),
                                command->serviceName());
            delete command;
            cmd = 0;
            break;
        }

        case(GPCommand::ExifInfo): {
            GPCommandExifInfo *command
                = static_cast<GPCommandExifInfo *>(cmd);
            exifInfo(command->folder(),
                     command->itemName());
            delete command;
            cmd = 0;
            break;
        }

        default:
            qWarning("GPController: Unknown Command");
            break;
        }

        if (cmd)
            delete cmd;

    }
}

void GPController::initialize()
{
    mutex_.lock();
    int result = camera_->initialize();
    mutex_.unlock();

    if (result == GPCamera::GPSuccess) {
        QApplication::postEvent(parent_, new GPEvent(GPEvent::Init));
    }
    else if (result == GPCamera::GPSetup) {
        QString msg(i18n("Camera Model or Port not specified correctly.\n"
                         "Please run Setup"));
        error(msg);
    }
    else {
        QString msg(i18n("Failed to initialize camera.\n"
                         "Please ensure camera is connected properly and turned on"));
        error(msg);
    }
    
}

void GPController::getSubFolders(const QString& folder)
{
    QValueList<QString> subFolderList;
    subFolderList.clear();

    mutex_.lock();
    int result = camera_->getSubFolders(folder, subFolderList);
    mutex_.unlock();

    if (result == GPCamera::GPSuccess) {
        QApplication::postEvent(parent_, new GPEventGetSubFolders(folder,
                                                    subFolderList));

        if (subFolderList.count() > 0) {
            for (unsigned int i=0; i<subFolderList.count(); i++) {
                QString subFolder(folder);
                if (subFolder.endsWith("/"))
                    subFolder += subFolderList[i];
                else
                    subFolder += "/" + subFolderList[i];
                                    
                getSubFolders(subFolder);
            }
        }
        return;
    }
    else {
        QString msg(i18n("Failed to get subfolder names from '%1'\n").arg(folder));
        error(msg);
        return;
    }
    
}

void GPController::makeFolder(const QString&,
                              const QString&)
{
    
}

void GPController::deleteFolder(const QString&)
{
    
}

void GPController::getItemsInfo(const QString& folder)
{
    GPFileItemInfoList infoList;
    infoList.clear();

    mutex_.lock();
    int result = camera_->getItemsInfo(folder, infoList);
    mutex_.unlock();

    if (result == GPCamera::GPSuccess) {
        QApplication::postEvent(parent_, new GPEventGetItemsInfo(folder,
                                                   infoList));
    }
    else {
        QString msg(i18n("Failed to get images information from '%1'\n").arg(folder));
        error(msg);
    }
}

void GPController::getAllItemsInfo(const QString& folder)
{
    GPFileItemInfoList infoList;
    infoList.clear();

    mutex_.lock();
    camera_->getAllItemsInfo(folder, infoList);
    mutex_.unlock();
    QApplication::postEvent(parent_, new GPEventGetAllItemsInfo(infoList));
    
}

void GPController::getThumbnail(const QString& folder,
                                const QString& imageName,
                                const ThumbnailSize& thumbSize)
{
    QImage thumbnail;

    mutex_.lock();
    int result = camera_->getThumbnail(folder, imageName,
                                       thumbnail);
    mutex_.unlock();

    if (result == GPCamera::GPSuccess) {

        scaleHighlightThumbnail(thumbnail, thumbSize);

        QApplication::postEvent(parent_, new GPEventGetThumbnail(folder,
                                                   imageName,
                                                   thumbnail));
    }
    else 
        {
        QString msg = i18n("Failed to get preview for '%1/%2'").arg(folder).arg(imageName);
        kdError() << msg << endl;
        }

}

void GPController::downloadItem(const QString& folder,
                                const QString& itemName,
                                const QString& saveFile)
{
    mutex_.lock();
    int result = camera_->downloadItem(folder, itemName,
                                       saveFile);
    mutex_.unlock();

    if (result != GPCamera::GPSuccess) {
        QString msg(i18n("Failed to download '%1' from '%2'").arg(itemName).arg(folder));
        error(msg);
    }
    else {
        QApplication::postEvent(parent_, new GPEventDownloadItem(folder,
                                                   itemName));
    }

}

void GPController::openItem(const QString& folder,
                            const QString& itemName,
                            const QString& saveFile)
{
    mutex_.lock();
    int result = camera_->downloadItem(folder, itemName,
                                       saveFile);
    mutex_.unlock();

    if (result != GPCamera::GPSuccess) {
        QString msg(i18n("Failed to open '%1'").arg(itemName));
        error(msg);
    }
    else {
        QApplication::postEvent(parent_, new GPEventOpenItem(saveFile));
    }
}

void GPController::openItemWithService(const QString& folder,
                                       const QString& itemName,
                                       const QString& saveFile,
                                       const QString& serviceName)
{
    mutex_.lock();
    int result = camera_->downloadItem(folder, itemName,
                                       saveFile);
    mutex_.unlock();

    if (result != GPCamera::GPSuccess) {
        QString msg(i18n("Failed to open '%1'").arg(itemName));
        error(msg);
    }
    else {
        QApplication::postEvent(parent_,
                  new GPEventOpenItemWithService(saveFile,
                                                 serviceName));
    }
}

void GPController::deleteItem(const QString& folder,
                              const QString& itemName)
{
   mutex_.lock();
   int result = camera_->deleteItem(folder, itemName);
   mutex_.unlock();

   if (result != GPCamera::GPSuccess) {
       QString msg(i18n("Failed to delete '%1'").arg(itemName));
       error(msg);
   }
   else {
       QApplication::postEvent(parent_,
                 new GPEventDeleteItem(folder, itemName));
   }
}

void GPController::deleteAllItems(const QString& rootFolder)
{
    mutex_.lock();
    int result = camera_->deleteAllItems(rootFolder);
    mutex_.unlock();

    if (result != GPCamera::GPSuccess) {
       QString msg(i18n("Failed Operation: Deleting all items in Camera"));
       error(msg);
   }
   else {
       QApplication::postEvent(parent_,
                 new GPEvent(GPEvent::DeleteAllItems));
   }
}

void GPController::uploadItem(const QString& folder,
                              const QString& uploadName,
                              const QString& localFile)
{
    mutex_.lock();
    int result = camera_->uploadItem(folder, uploadName,
                                     localFile);
    mutex_.unlock();

    if (result != GPCamera::GPSuccess) {
        QString msg(i18n("Failed to upload '%1'").arg(localFile));
        error(msg);
    }
    else {

        GPFileItemInfoList infoList;
        GPFileItemInfoList infoList2;
        infoList.clear();
        infoList2.clear();

        mutex_.lock();
        int result = camera_->getItemsInfo(folder, infoList);
        mutex_.unlock();

        if (result == GPCamera::GPSuccess) {

            while ( !(infoList.isEmpty()) ) {

                GPFileItemInfo info( infoList.first() );
                infoList.pop_front();

                if (info.name == uploadName) {
                    infoList2.push_back(info);
                    break;
                }

            }

            if (!infoList2.isEmpty())
                QApplication::postEvent(parent_, new GPEventGetItemsInfo(folder,
                                                  infoList2));

        }

    }
}

void GPController::exifInfo(const QString& folder,
                            const QString& itemName)
{
    char *data = 0;
    int   size = 0;
    int result = 0;

    mutex_.lock();
    result = camera_->getExif(folder, itemName,
                              &data, size);
    mutex_.unlock();

    if (result != GPCamera::GPSuccess) {
        QString msg(i18n("Failed to get Exif Information for '%1'").arg(itemName));
        error(msg);
    }
    else {
        QApplication::postEvent(parent_, new GPEventExifInfo(folder, itemName,
                                               data, size));
    }
}

void GPController::error(const QString& errorMsg)
{
    kdError() << errorMsg << endl;
    QApplication::postEvent(parent_, new GPEventError(errorMsg));
}

void GPController::scaleHighlightThumbnail(QImage& thumbnail,
                                           const ThumbnailSize& thumbSize)
{

    thumbnail = thumbnail.smoothScale(thumbSize.size(),
                                      thumbSize.size(),
                                      QImage::ScaleMin);

    QColor darkColor(48, 48, 48);
    QColor lightColor(215, 215, 215);

    int w = thumbnail.width();
    int h = thumbnail.height();

    // Right
    for (int y=0; y<h; y++) {
        if (y > 1 && y < h-2)
             thumbnail.setPixel(w-3, y, lightColor.rgb());
        thumbnail.setPixel(w-1, y, darkColor.rgb());
        thumbnail.setPixel(w-2, y, darkColor.rgb());
    }

    // Bottom
    for (int x=0; x<w; x++) {
         if (x > 1 && x < w-2)
             thumbnail.setPixel(x, h-3, lightColor.rgb());
        thumbnail.setPixel(x, h-1, darkColor.rgb());
        thumbnail.setPixel(x, h-2, darkColor.rgb());
    }

    // Top
    for (int x=0; x<w; x++) {
        if (x > 1 && x < w-2)
             thumbnail.setPixel(x, 2, lightColor.rgb());
        thumbnail.setPixel(x, 0, darkColor.rgb());
        thumbnail.setPixel(x, 1, darkColor.rgb());
    }

    // Left
    for (int y=0; y<h; y++) {
         if (y > 1 && y < h-2)
             thumbnail.setPixel(2, y, lightColor.rgb());
        thumbnail.setPixel(0, y, darkColor.rgb());
        thumbnail.setPixel(1, y, darkColor.rgb());
    }

}

void GPController::slotStatusMsg(const QString& msg)
{
    if (!msg.isEmpty())
        QApplication::postEvent(parent_, new GPEventStatusMsg(msg));
}

void GPController::slotProgressVal(int val)
{
    QApplication::postEvent(parent_, new GPEventProgress(val));
}

void GPController::slotErrorMsg(const QString& msg)
{
    error(msg);
}

void GPController::showBusy(bool val)
{
    QApplication::postEvent(parent_, new GPEventBusy(val));
}

void GPController::getInformation(QString& summary, QString& manual,
                                  QString& about)
{
    mutex_.lock();
    camera_->cameraSummary(summary);
    camera_->cameraManual(manual);
    camera_->cameraAbout(about);
    mutex_.unlock();
}

#include "gpcontroller.moc"
