/* ============================================================
 * File  : gpevents.h
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

#ifndef GPEVENTS_H
#define GPEVENTS_H

#include <qevent.h>
#include <qstring.h>
#include <qimage.h>
#include <qvaluelist.h>

#include "mtlist.h"
#include "gpfileiteminfo.h"

class GPEvent : public QCustomEvent
{
public:

    enum Type
    {
        Init = QCustomEvent::User,
        GetSubFolders,
        MakeFolder,
        DeleteFolder,
        GetItemsInfo,
        GetAllItemsInfo,
        GetThumbnail,
        DownloadItem,
        DeleteItem,
        DeleteAllItems,
        OpenItem,
        OpenItemWithService,
        UploadItem,
        ExifInfo,
        Information,
        StatusMsg,
        Progress,
        Error,
        Busy
    };    

    GPEvent(Type type) : QCustomEvent(type) {}
};

class GPEventError : public GPEvent
{
public:

    GPEventError(const QString errorMsg)
        : GPEvent(Error),
          errorMsg_(errorMsg)
        {}

    QString errorMsg() const { return errorMsg_; }

private:

    QString errorMsg_;

};


class GPEventGetSubFolders : public GPEvent
{
public:

    GPEventGetSubFolders(const QString& folder,
                         const QValueList<QString>& subFolderList)
        : GPEvent(GetSubFolders),
          folder_(folder),
          subFolderList_(subFolderList)
        {}

    QString folder() const { return folder_; }
    const MTList<QString>& subFolderList() const { return subFolderList_; }

private:

    QString folder_;
    MTList<QString> subFolderList_;
};


class GPEventGetItemsInfo : public GPEvent
{
public:

    GPEventGetItemsInfo(const QString& folder,
                        const GPFileItemInfoList& infoList)
        : GPEvent(GetItemsInfo),
          folder_(folder),
          infoList_(infoList)
        {}

    QString folder() const { return folder_; }
    const MTList<GPFileItemInfo>& infoList() const { return infoList_; }

private:

    QString folder_;
    MTList<GPFileItemInfo> infoList_;
};

class GPEventGetAllItemsInfo : public GPEvent
{
public:

    GPEventGetAllItemsInfo(const GPFileItemInfoList& infoList)
        : GPEvent(GetAllItemsInfo),
          infoList_(infoList)
        {}

    const MTList<GPFileItemInfo>& infoList() const { return infoList_; }

private:

    MTList<GPFileItemInfo> infoList_;
};

class GPEventGetThumbnail : public GPEvent
{
public:

    GPEventGetThumbnail(const QString& folder,
                        const QString& imageName,
                        const QImage& thumbnail)
        : GPEvent(GetThumbnail),
          folder_(folder),
          imageName_(imageName),
          thumbnail_(thumbnail)
        {}

    QString folder() const { return folder_; }
    QString imageName() const { return imageName_; }
    QImage  thumbnail() const { return thumbnail_; }


private:

    QString folder_;
    QString imageName_;
    QImage  thumbnail_;
};

class GPEventDownloadItem : public GPEvent
{
public:

    GPEventDownloadItem(const QString& folder,
                        const QString& itemName)
        : GPEvent(DownloadItem),
          folder_(folder),
          itemName_(itemName)
        {}

    QString folder() const { return folder_; }
    QString itemName() const { return itemName_; }

private:

    QString folder_;
    QString itemName_;
};

class GPEventDeleteItem : public GPEvent
{
public:

    GPEventDeleteItem(const QString& folder,
                      const QString& itemName)
        : GPEvent(DeleteItem),
          folder_(folder),
          itemName_(itemName)
        {}

    QString folder() const { return folder_; }
    QString itemName() const { return itemName_; }

private:

    QString folder_;
    QString itemName_;
};


class GPEventOpenItem : public GPEvent
{
public:

    GPEventOpenItem(const QString& openFile)
        : GPEvent(OpenItem),
          openFile_(openFile)
        {}

    QString openFile() const { return openFile_; }

private:

    QString openFile_;
};

class GPEventOpenItemWithService : public GPEvent
{
public:

    GPEventOpenItemWithService(const QString& openFile,
                               const QString& serviceName)
        : GPEvent(OpenItemWithService),
          openFile_(openFile),
          serviceName_(serviceName)
        { }

    QString openFile() const { return openFile_; }
    QString serviceName() const { return serviceName_; }


private:

    QString openFile_;
    QString serviceName_;
};

class GPEventExifInfo : public GPEvent
{
public:
    
    GPEventExifInfo(const QString& folder,
                    const QString& itemName,
                    char *data, int size)
        : GPEvent(ExifInfo),
          folder_(folder),
          itemName_(itemName),
          data_(data),
          size_(size)
        {}

    QString folder() const { return folder_; }
    QString itemName() const { return itemName_; }
    char*   data() { return data_; }
    int     size() const { return size_; }
    
private:

    QString folder_;
    QString itemName_;
    char *data_;
    int   size_;

};

class GPEventStatusMsg : public GPEvent
{
public:
    GPEventStatusMsg(const QString& msg)
        : GPEvent(StatusMsg),
          msg_(msg)
        {}

    QString msg() const { return msg_; }

private:

    QString msg_;
};

class GPEventProgress : public GPEvent
{
public:

    GPEventProgress(int val)
        : GPEvent(Progress),
          val_(val)
        {}

    int val() { return val_; }

private:

    int val_;
};

class GPEventBusy : public GPEvent
{
public:

    GPEventBusy(bool busy)
        : GPEvent(Busy),
          busy_(busy)
        {}

    bool busy() { return busy_; }

private:

    bool busy_;
};

#endif /* GPEVENTS_H */
