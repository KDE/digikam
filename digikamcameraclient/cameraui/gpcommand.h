/* ============================================================
 * File  : gpcommand.h
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

#ifndef GPCOMMAND_H
#define GPCOMMAND_H

#include <qstring.h>

#include "thumbnailsize.h"

class GPCommand
{
public:

    enum Type
    {
        Init=0,
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
        ExifInfo
    };    

    GPCommand(Type type) : type_(type) {}
    Type type() const { return type_; }

private:

    Type type_;
};

// --------------------------------------------------------------

class GPCommandGetSubFolders : public GPCommand
{
public:

    GPCommandGetSubFolders(const QString& folder)
        : GPCommand(GetSubFolders),
          folder_(folder)
        {}

    QString folder() const { return folder_; }

private:

    QString folder_;
};

// --------------------------------------------------------------

class GPCommandMakeFolder : public GPCommand
{
public:

    GPCommandMakeFolder(const QString& folder,
                           const QString& newFolder)
        : GPCommand(MakeFolder),
          folder_(folder),
          newFolder_(newFolder)
        {}

    QString folder() const { return folder_; }
    QString newFolder() const { return newFolder_; }

private:

    QString folder_;
    QString newFolder_;
};

// --------------------------------------------------------------

class GPCommandDeleteFolder : public GPCommand
{
public:

    GPCommandDeleteFolder(const QString& folder)
        : GPCommand(DeleteFolder),
          folder_(folder)
        {}

    QString folder() const { return folder_; }

private:

    QString folder_;
};

// --------------------------------------------------------------

class GPCommandGetItemsInfo : public GPCommand
{
public:

    GPCommandGetItemsInfo(const QString& folder)
        : GPCommand(GetItemsInfo),
          folder_(folder)
        {}

    QString folder() const { return folder_; }

private:

    QString folder_;
};

// --------------------------------------------------------------

class GPCommandGetAllItemsInfo : public GPCommand
{
public:

    GPCommandGetAllItemsInfo(const QString& folder)
        : GPCommand(GetAllItemsInfo),
          folder_(folder)
        {}

    QString folder() const { return folder_; }

private:

    QString folder_;
};

// --------------------------------------------------------------

class GPCommandGetThumbnail : public GPCommand
{
public:

    GPCommandGetThumbnail(const QString& folder,
                          const QString& imageName,
                          const ThumbnailSize& thumbSize)
        : GPCommand(GetThumbnail),
          folder_(folder),
          imageName_(imageName),
          thumbSize_(thumbSize)
        {}

    QString folder() const { return folder_; }
    QString imageName() const { return imageName_; }
    ThumbnailSize thumbSize() const { return thumbSize_; }

private:

    QString folder_;
    QString imageName_;
    ThumbnailSize thumbSize_;
};

// --------------------------------------------------------------

class GPCommandDownloadItem : public GPCommand
{
public:

    GPCommandDownloadItem(const QString& folder,
                          const QString& itemName,
                          const QString& saveFile)
        : GPCommand(DownloadItem),
          folder_(folder),
          itemName_(itemName),
          saveFile_(saveFile)
        {}

    QString folder() const { return folder_; }
    QString itemName() const { return itemName_; }
    QString saveFile() const { return saveFile_; }

private:

    QString folder_;
    QString itemName_;
    QString saveFile_;
};

// --------------------------------------------------------------

class GPCommandDeleteItem : public GPCommand
{
public:

    GPCommandDeleteItem(const QString& folder,
                        const QString& itemName)
        : GPCommand(DeleteItem),
          folder_(folder),
          itemName_(itemName)
        {}

    QString folder() const { return folder_; }
    QString itemName() const { return itemName_; }

private:

    QString folder_;
    QString itemName_;
};

// --------------------------------------------------------------

class GPCommandDeleteAllItems : public GPCommand
{
public:

    GPCommandDeleteAllItems(const QString& rootFolder)
        : GPCommand(DeleteAllItems),
          folder_(rootFolder)
        {}

    QString rootFolder() const { return folder_; }

private:

    QString folder_;
};


// --------------------------------------------------------------

class GPCommandUploadItem : public GPCommand
{
public:

    GPCommandUploadItem(const QString& folder,
                        const QString& localFile,
                        const QString& uploadName)
        : GPCommand(UploadItem),
          folder_(folder),
          localFile_(localFile),
          uploadName_(uploadName)
        {}

    QString folder() const { return folder_; }
    QString localFile() const { return localFile_; }
    QString uploadName() const { return uploadName_; }

private:

    QString folder_;
    QString localFile_;
    QString uploadName_;
};

// --------------------------------------------------------------

class GPCommandOpenItem : public GPCommand
{
public:

    GPCommandOpenItem(const QString& folder,
                      const QString& itemName,
                      const QString& saveFile)
        : GPCommand(OpenItem),
          folder_(folder),
          itemName_(itemName),
          saveFile_(saveFile)
        {}

    QString folder() const { return folder_; }
    QString itemName() const { return itemName_; }
    QString saveFile() const { return saveFile_; }

private:

    QString folder_;
    QString itemName_;
    QString saveFile_;
};

// --------------------------------------------------------------

class GPCommandOpenItemWithService : public GPCommand
{
public:

    GPCommandOpenItemWithService(const QString& folder,
                                 const QString& itemName,
                                 const QString& saveFile,
                                 const QString& serviceName)
        : GPCommand(OpenItemWithService),
          folder_(folder),
          itemName_(itemName),
          saveFile_(saveFile),
          serviceName_(serviceName)
        { }

    QString folder() const { return folder_; }
    QString itemName() const { return itemName_; }
    QString saveFile() const { return saveFile_; }
    QString serviceName() const { return serviceName_; }

private:

    QString folder_;
    QString itemName_;
    QString saveFile_;
    QString serviceName_;
};

// --------------------------------------------------------------

class GPCommandExifInfo : public GPCommand
{
public:

    GPCommandExifInfo(const QString& folder,
                      const QString& itemName)
        : GPCommand(ExifInfo),
          folder_(folder),
          itemName_(itemName)
        { }

    QString folder() const { return folder_; }
    QString itemName() const { return itemName_; }

private:

    QString folder_;
    QString itemName_;
};

#endif /* GPCOMMAND_H */
