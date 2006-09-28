/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2003-01-21
 * Description : Gphoto2 camera interface
 * 
 * Copyright 2003-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

// C Ansi includes.

extern "C"
{
#include <gphoto2.h>
}

// C++ includes.

#include <cstdio>
#include <iostream>

// QT includes.

#include <qstring.h>
#include <qstringlist.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qdom.h>
#include <qfile.h>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>

// Local includes.

#include "gpcamera.h"

namespace Digikam
{

class GPCameraPrivate
{

public:

    GPCameraPrivate()
    {
        camera = 0;
    }

    bool             cameraInitialized;
    
    bool             thumbnailSupport;
    bool             deleteSupport;
    bool             uploadSupport;
    bool             mkDirSupport;
    bool             delDirSupport;
    
    QString          model;
    QString          port;
    QString          globalPath;

    Camera          *camera;
    CameraAbilities  cameraAbilities;
};

class GPStatus
{

public:

    GPStatus() 
    {
        context = gp_context_new();
        cancel  = false;
        gp_context_set_cancel_func(context, cancel_func, 0);
    }

    ~GPStatus() 
    {
        gp_context_unref(context);
        cancel = false;
    }

    GPContext   *context;
    static bool  cancel;

    static GPContextFeedback cancel_func(GPContext *, void *) 
    {
        return (cancel ? GP_CONTEXT_FEEDBACK_CANCEL :
                GP_CONTEXT_FEEDBACK_OK);
    }            
};

bool GPStatus::cancel = false;

GPCamera::GPCamera(const QString& title, const QString& model, const QString& port, const QString& path)
        : DKCamera(title, model, port, path)
{
    m_status = 0;
    
    d = new GPCameraPrivate;
    d->camera            = 0;
    d->model             = model;
    d->port              = port;
    d->globalPath        = path;
    d->cameraInitialized = false;
    d->thumbnailSupport  = false;
    d->deleteSupport     = false;
    d->uploadSupport     = false;
    d->mkDirSupport      = false;
    d->delDirSupport     = false;
}

GPCamera::~GPCamera()
{
    if (d->camera) 
    {
        gp_camera_unref(d->camera);
        d->camera = 0;
    }

    delete d;
}

QString GPCamera::model() const
{
    return d->model;    
}

QString GPCamera::port() const
{
    return d->port;
}

QString GPCamera::path() const
{
    return d->globalPath;   
}

bool GPCamera::thumbnailSupport()
{
    return d->thumbnailSupport;    
}

bool GPCamera::deleteSupport()
{
    return d->deleteSupport;
}

bool GPCamera::uploadSupport()
{
    return d->uploadSupport;
}

bool GPCamera::mkDirSupport()
{
    return d->mkDirSupport;
}

bool GPCamera::delDirSupport()
{
    return d->delDirSupport;
}

bool GPCamera::doConnect()
{
    int errorCode;
    // -- first step - setup the camera --------------------
    
    if (d->camera) 
    {
        gp_camera_unref(d->camera);
        d->camera = 0;
    }
    
    CameraAbilitiesList *abilList;
    GPPortInfoList      *infoList;
    GPPortInfo           info;

    gp_camera_new(&d->camera);

    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
            
    m_status = new GPStatus();
    
    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, m_status->context);
    gp_port_info_list_new(&infoList);
    gp_port_info_list_load(infoList);

    delete m_status;
    m_status = 0;

    int modelNum = -1, portNum = -1;
    modelNum = gp_abilities_list_lookup_model(abilList, d->model.latin1());
    portNum  = gp_port_info_list_lookup_path (infoList, d->port.latin1());

    gp_abilities_list_get_abilities(abilList, modelNum, &d->cameraAbilities);
    
    errorCode = gp_camera_set_abilities(d->camera, d->cameraAbilities); 
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to set camera Abilities!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_camera_unref(d->camera);
        d->camera = 0;
        gp_abilities_list_free(abilList);
        gp_port_info_list_free(infoList);
        return false;
    }

    if (d->model != "Directory Browse") 
    {
        gp_port_info_list_get_info(infoList, portNum, &info);
        errorCode = gp_camera_set_port_info(d->camera, info);
        if (errorCode != GP_OK) 
        {
            kdDebug() << "Failed to set camera port!" << endl;
            printGphotoErrorDescription(errorCode);
            gp_camera_unref(d->camera);
            d->camera = 0;
            gp_abilities_list_free (abilList);
            gp_port_info_list_free (infoList);
            return false;
        }
    }

    gp_abilities_list_free (abilList);
    gp_port_info_list_free (infoList);

    if (d->cameraAbilities.file_operations &
        GP_FILE_OPERATION_PREVIEW)
        d->thumbnailSupport = true;

    if (d->cameraAbilities.file_operations &
        GP_FILE_OPERATION_DELETE)
        d->deleteSupport = true;

    if (d->cameraAbilities.folder_operations &
        GP_FOLDER_OPERATION_PUT_FILE)
        d->uploadSupport = true;

    if (d->cameraAbilities.folder_operations &
        GP_FOLDER_OPERATION_MAKE_DIR)
        d->mkDirSupport = true;

    if (d->cameraAbilities.folder_operations &
        GP_FOLDER_OPERATION_REMOVE_DIR)
        d->delDirSupport = true;

    // -- Now try to initialize the camera -----------------

    m_status = new GPStatus();

    // Try and initialize the camera to see if its connected
    errorCode = gp_camera_init(d->camera, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to initialize camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_camera_unref(d->camera);
        d->camera = 0;
        delete m_status;
        m_status = 0;
        return false;
    }

    delete m_status;
    m_status = 0;
    
    d->cameraInitialized = true;    
    return true;
}

void GPCamera::cancel()
{
    if (!m_status)
        return;
    m_status->cancel = true;
}

void GPCamera::getAllFolders(const QString& rootFolder,
                             QStringList& folderList)
{
    QStringList subfolders;
    getSubFolders(rootFolder, subfolders);

    for (QStringList::iterator it = subfolders.begin();
         it != subfolders.end(); ++it)
    {
        *it = rootFolder + QString(rootFolder.endsWith("/") ? "" : "/") + (*it);
        folderList.append(*it);
    }

    for (QStringList::iterator it = subfolders.begin();
         it != subfolders.end(); ++it)
    {
        getAllFolders(*it, folderList);
    }
}

bool GPCamera::getSubFolders(const QString& folder, QStringList& subFolderList)
{
    int         errorCode;
    CameraList *clist;
    gp_list_new(&clist);

    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    m_status = new GPStatus();

    errorCode = gp_camera_folder_list_folders(d->camera, QFile::encodeName(folder), clist, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get folders list from camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_list_unref(clist);
        delete m_status;
        m_status = 0;
        return false;
    }

    delete m_status;
    m_status = 0;

    int count = gp_list_count(clist);
    for (int i = 0 ; i < count ; i++) 
    {
        const char* subFolder;
        errorCode = gp_list_get_name(clist, i, &subFolder);
        if (errorCode != GP_OK) 
        {
            kdDebug() << "Failed to get folder name from camera!" << endl;
            printGphotoErrorDescription(errorCode);
            gp_list_unref(clist);
            return false;
        }

        subFolderList.append(QString(subFolder));
    }

    gp_list_unref(clist);
    return true;
}

bool GPCamera::getItemsList(const QString& folder, QStringList& itemsList)
{
    int         errorCode;
    CameraList *clist;
    const char *cname;

    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    m_status = new GPStatus;
    
    gp_list_new(&clist);

    errorCode = gp_camera_folder_list_files(d->camera, QFile::encodeName(folder), clist, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get folder files list from camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_list_unref(clist);
        delete m_status;
        m_status = 0;
        return false;
    }

    int count = gp_list_count(clist);
    for (int i = 0 ; i < count ; i++)
    {
        errorCode = gp_list_get_name(clist, i, &cname);
        if (errorCode != GP_OK)
        {
            kdDebug() << "Failed to get file name from camera!" << endl;
            printGphotoErrorDescription(errorCode);
            gp_list_unref(clist);
            delete m_status;
            m_status = 0;
            return false;
        }

        itemsList.append(cname);
    }

    gp_list_unref(clist);        
    
    delete m_status;
    m_status = 0;

    return true;
}

bool GPCamera::getItemsInfoList(const QString& folder, GPItemInfoList& items, bool /*getImageDimensions*/)
{
    int         errorCode;
    CameraList *clist;
    const char *cname;

    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    m_status = new GPStatus;
    
    gp_list_new(&clist);
    
    errorCode = gp_camera_folder_list_files(d->camera, QFile::encodeName(folder), clist, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get folder files list from camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_list_unref(clist);
        delete m_status;
        m_status = 0;
        return false;
    }

    int count = gp_list_count(clist);
    for (int i = 0 ; i < count ; i++)
    {
        errorCode = gp_list_get_name(clist, i, &cname);
        if (errorCode != GP_OK)
        {
            kdDebug() << "Failed to get file name from camera!" << endl;
            printGphotoErrorDescription(errorCode);
            gp_list_unref(clist);
            delete m_status;
            m_status = 0;
            return false;
        }

        GPItemInfo itemInfo;

        itemInfo.name   = cname;
        itemInfo.folder = folder;

        CameraFileInfo info;
        gp_camera_file_get_info(d->camera, QFile::encodeName(folder),
                                cname, &info, m_status->context);

        itemInfo.mtime            = -1;
        itemInfo.mime             = "";
        itemInfo.size             = -1;
        itemInfo.width            = -1;
        itemInfo.height           = -1;
        itemInfo.downloaded       = GPItemInfo::DownloadUnknow;
        itemInfo.readPermissions  = -1;
        itemInfo.writePermissions = -1;
        
        /* The mime type returned by Gphoto2 is dummy with all RAW files.
        if (info.file.fields & GP_FILE_INFO_TYPE)
            itemInfo.mime = info.file.type;*/

        itemInfo.mime = mimeType(itemInfo.name.section('.', -1).lower());

        if (info.file.fields & GP_FILE_INFO_MTIME)
            itemInfo.mtime = info.file.mtime;      

        if (info.file.fields & GP_FILE_INFO_SIZE)
            itemInfo.size = info.file.size;

        if (info.file.fields & GP_FILE_INFO_WIDTH)
            itemInfo.width = info.file.width;

        if (info.file.fields & GP_FILE_INFO_HEIGHT)
            itemInfo.height = info.file.height;

        if (info.file.fields & GP_FILE_INFO_STATUS) 
        {
            if (info.file.status == GP_FILE_STATUS_DOWNLOADED)
                itemInfo.downloaded = GPItemInfo::DownloadedYes;
            else
                itemInfo.downloaded = GPItemInfo::DownloadedNo;
        }
        
        if (info.file.fields & GP_FILE_INFO_PERMISSIONS) 
        {
            if (info.file.permissions & GP_FILE_PERM_READ)
                itemInfo.readPermissions = 1;
            else
                itemInfo.readPermissions = 0;
            if (info.file.permissions & GP_FILE_PERM_DELETE)
                itemInfo.writePermissions = 1;
            else
                itemInfo.writePermissions = 0;
        }

        items.append(itemInfo);
    }

    gp_list_unref(clist);        
    
    delete m_status;
    m_status = 0;

    return true;
}

bool GPCamera::getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail)
{
    int                errorCode;
    CameraFile        *cfile;
    const char        *data;
    unsigned long int  size;
    
    gp_file_new(&cfile);
    
    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    
    m_status = new GPStatus;

    errorCode = gp_camera_file_get(d->camera, QFile::encodeName(folder),
                                   QFile::encodeName(itemName),
                                   GP_FILE_TYPE_PREVIEW,
                                   cfile, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete m_status;
        m_status = 0;
        return false;
    }

    delete m_status;
    m_status = 0;

    errorCode = gp_file_get_data_and_size(cfile, &data, &size);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get thumbnail from camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    thumbnail.loadFromData((const uchar*) data, (uint) size);

    gp_file_unref(cfile);
    return true;
}

bool GPCamera::getExif(const QString& folder, const QString& itemName,
                       char **edata, int& esize)
{
    int                errorCode;
    CameraFile        *cfile;
    const char        *data;
    unsigned long int  size;
    
    gp_file_new(&cfile);
    
    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    
    m_status = new GPStatus;

    errorCode = gp_camera_file_get(d->camera, QFile::encodeName(folder),
                                   QFile::encodeName(itemName),
                                   GP_FILE_TYPE_EXIF,
                                   cfile, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete m_status;
        m_status = 0;
        return false;
    }

    delete m_status;
    m_status = 0;
    
    errorCode = gp_file_get_data_and_size(cfile, &data, &size);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get Exif data from camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    *edata = new char[size];
    esize  = size;
    memcpy(*edata, data, size);

    gp_file_unref(cfile);
    return true;
}

bool GPCamera::downloadItem(const QString& folder, const QString& itemName,
                            const QString& saveFile)
{
    int         errorCode;
    CameraFile *cfile;

    gp_file_new(&cfile);
    
    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    
    m_status = new GPStatus;
    
    errorCode = gp_camera_file_get(d->camera, QFile::encodeName(folder),
                                   QFile::encodeName(itemName),
                                   GP_FILE_TYPE_NORMAL, cfile,
                                   m_status->context);
    if ( errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete m_status;
        m_status = 0;
        return false;
    }
    
    delete m_status;
    m_status = 0;

    errorCode = gp_file_save(cfile, QFile::encodeName(saveFile));
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to save camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }
    
    gp_file_unref(cfile);
    return true;
}

bool GPCamera::setLockItem(const QString& folder, const QString& itemName, bool lock)
{
    int errorCode;
    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    
    m_status = new GPStatus;

    CameraFileInfo info;
    errorCode = gp_camera_file_get_info(d->camera, QFile::encodeName(folder),
                                QFile::encodeName(itemName), &info, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get camera item properties!" << endl;
        printGphotoErrorDescription(errorCode);
        delete m_status;
        m_status = 0;
        return false;
    }

    if (info.file.fields & GP_FILE_INFO_PERMISSIONS) 
    {
        if (lock)
        {
            // Lock the file to set read only flag
            info.file.permissions = (CameraFilePermissions)GP_FILE_PERM_READ;
        }
        else
        {
            // Unlock the file to set read/write flag
            info.file.permissions = (CameraFilePermissions)(GP_FILE_PERM_READ | GP_FILE_PERM_DELETE);
        }
    }
    
    // Some gphoto2 drivers need to have only the right flag at on to process properties update in camera.
    info.file.fields    = GP_FILE_INFO_PERMISSIONS;
    info.preview.fields = GP_FILE_INFO_NONE; 
    info.audio.fields   = GP_FILE_INFO_NONE;

    errorCode = gp_camera_file_set_info(d->camera, QFile::encodeName(folder),
                                        QFile::encodeName(itemName), info, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to set camera item lock properties!" << endl;
        printGphotoErrorDescription(errorCode);
        delete m_status;
        m_status = 0;
        return false;
    }

    delete m_status;
    m_status = 0;
    return true;
}

bool GPCamera::deleteItem(const QString& folder, const QString& itemName)
{
    int errorCode;
    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    
    m_status = new GPStatus;

    errorCode = gp_camera_file_delete(d->camera, QFile::encodeName(folder),
                                      QFile::encodeName(itemName),
                                      m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to delete camera item!" << endl;
        printGphotoErrorDescription(errorCode);
        delete m_status;
        m_status = 0;
        return false;
    }

    delete m_status;
    m_status = 0;

    return true;
}

// recursively delete all items
bool GPCamera::deleteAllItems(const QString& folder)
{
    int         errorCode;
    QStringList folderList;

    // Get all subfolders in this folder
    getSubFolders(folder, folderList);

    if (folderList.count() > 0) 
    {
        for (unsigned int i = 0 ; i < folderList.count() ; i++) 
        {
            QString subFolder(folder);

            if (!subFolder.endsWith("/"))
                subFolder += '/';

            subFolder += folderList[i];
            deleteAllItems(subFolder);
        }
    }
    
    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    
    m_status = new GPStatus;
    
    errorCode = gp_camera_folder_delete_all(d->camera, QFile::encodeName(folder),
                                            m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to delete camera folder!" << endl;
        printGphotoErrorDescription(errorCode);
        delete m_status;
        m_status = 0;
        return false;
    }
    
    delete m_status;
    m_status = 0;

    return true;
}

bool GPCamera::uploadItem(const QString& folder, const QString& itemName, const QString& localFile,
                          GPItemInfo& itemInfo, bool /*getImageDimensions*/)
{
    int         errorCode;
    CameraFile *cfile;

    errorCode = gp_file_new(&cfile);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to init new camera file instance!" << endl;
        printGphotoErrorDescription(errorCode);
        return false;
    }

    errorCode = gp_file_open(cfile, QFile::encodeName(localFile));
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to open file!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    errorCode = gp_file_set_name(cfile, QFile::encodeName(itemName));
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to rename item from camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }
    
    m_status = new GPStatus;

    errorCode = gp_camera_folder_put_file(d->camera,
                                          QFile::encodeName(folder),
                                          cfile,
                                          m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to upload item to camera!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete m_status;
        m_status = 0;
        return false;
    }

    // Get new camera item informations.

    itemInfo.name   = itemName;
    itemInfo.folder = folder;

    CameraFileInfo info;
    errorCode = gp_camera_file_get_info(d->camera, QFile::encodeName(folder),
                                        QFile::encodeName(itemName), &info, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get camera item informations!" << endl;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        delete m_status;
        m_status = 0;
        return false;
    }

    itemInfo.mtime            = -1;
    itemInfo.mime             = "";
    itemInfo.size             = -1;
    itemInfo.width            = -1;
    itemInfo.height           = -1;
    itemInfo.downloaded       = GPItemInfo::DownloadUnknow;
    itemInfo.readPermissions  = -1;
    itemInfo.writePermissions = -1;
    
    /* The mime type returned by Gphoto2 is dummy with all RAW files.
    if (info.file.fields & GP_FILE_INFO_TYPE)
        itemInfo.mime = info.file.type;*/

    itemInfo.mime = mimeType(itemInfo.name.section('.', -1).lower());

    if (info.file.fields & GP_FILE_INFO_MTIME)
        itemInfo.mtime = info.file.mtime;      

    if (info.file.fields & GP_FILE_INFO_SIZE)
        itemInfo.size = info.file.size;

    if (info.file.fields & GP_FILE_INFO_WIDTH)
        itemInfo.width = info.file.width;

    if (info.file.fields & GP_FILE_INFO_HEIGHT)
        itemInfo.height = info.file.height;

    if (info.file.fields & GP_FILE_INFO_STATUS) 
    {
        if (info.file.status == GP_FILE_STATUS_DOWNLOADED)
            itemInfo.downloaded = GPItemInfo::DownloadedYes;
        else
            itemInfo.downloaded = GPItemInfo::DownloadedNo;
    }
    
    if (info.file.fields & GP_FILE_INFO_PERMISSIONS) 
    {
        if (info.file.permissions & GP_FILE_PERM_READ)
            itemInfo.readPermissions = 1;
        else
            itemInfo.readPermissions = 0;
        if (info.file.permissions & GP_FILE_PERM_DELETE)
            itemInfo.writePermissions = 1;
        else
            itemInfo.writePermissions = 0;
    }

    gp_file_unref(cfile);
    delete m_status;
    m_status = 0;
    return true;
}

bool GPCamera::cameraSummary(QString& summary)
{
    int        errorCode;
    CameraText sum;

    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }

    m_status = new GPStatus;

    errorCode = gp_camera_get_summary(d->camera, &sum, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get camera summary!" << endl;
        printGphotoErrorDescription(errorCode);
        delete m_status;
        m_status = 0;
        return false;
    }

    summary = i18n("Title: %1\n"
                   "Model: %2\n"
                   "Port: %3\n"
                   "Path: %4\n\n"
                   "Thumbnail support: %5\n"
                   "Delete items support: %6\n"
                   "Upload items support: %7\n"
                   "Directory creation support: %8\n"
                   "Directory deletion support: %9\n\n")
                   .arg(title())
                   .arg(model())
                   .arg(port())
                   .arg(path())
                   .arg(thumbnailSupport() ? i18n("yes") : i18n("no"))
                   .arg(deleteSupport() ? i18n("yes") : i18n("no"))
                   .arg(uploadSupport() ? i18n("yes") : i18n("no"))
                   .arg(mkDirSupport() ? i18n("yes") : i18n("no"))
                   .arg(delDirSupport() ? i18n("yes") : i18n("no"));

    summary.append(QString(sum.text));

    delete m_status;
    m_status = 0;
    return true;
}

bool GPCamera::cameraManual(QString& manual)
{
    int        errorCode;
    CameraText man;

    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }

    m_status = new GPStatus;

    errorCode = gp_camera_get_manual(d->camera, &man, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get camera manual!" << endl;
        printGphotoErrorDescription(errorCode);
        delete m_status;
        m_status = 0;
        return false;
    }

    manual = QString(man.text);

    delete m_status;
    m_status = 0;
    return true;
}

bool GPCamera::cameraAbout(QString& about)
{
    int        errorCode;
    CameraText abt;

    if (m_status) 
    {
        delete m_status;
        m_status = 0;
    }

    m_status = new GPStatus;

    errorCode = gp_camera_get_about(d->camera, &abt, m_status->context);
    if (errorCode != GP_OK) 
    {
        kdDebug() << "Failed to get informations about camera!" << endl;
        printGphotoErrorDescription(errorCode);
        delete m_status;
        m_status = 0;
        return false;
    }

    about = QString(abt.text);
    about.append(i18n("\n\nTo report problems about this driver, please contact "
                      "the gphoto2 team at this address:\n\nhttp://gphoto.org/bugs"));

    delete m_status;
    m_status = 0;
    return true;
}

// -- Static methods ---------------------------------------------------------------------

// TODO merge these methods with GPIface implementation.

void GPCamera::printGphotoErrorDescription(int errorCode)
{
    kdDebug() << "Libgphoto2 error: " << gp_result_as_string(errorCode) 
              << " (" << errorCode << ")" << endl;
}

void GPCamera::getSupportedCameras(int& count, QStringList& clist)
{
    clist.clear();
    count = 0;

    CameraAbilitiesList *abilList;
    CameraAbilities      abil;
    GPContext           *context;

    context = gp_context_new();
 
    gp_abilities_list_new( &abilList );
    gp_abilities_list_load( abilList, context );

    count = gp_abilities_list_count( abilList );
    if ( count < 0) 
    {
        kdDebug() << "Failed to get list of cameras!" << endl;
        printGphotoErrorDescription(count);
        gp_context_unref( context );
        return;
    }
    else 
    {
        for (int i = 0 ; i < count ; i++) 
        {
            const char *cname;
            gp_abilities_list_get_abilities( abilList, i, &abil );
            cname = abil.model;
            clist.append( QString( cname ) );
        }
    }

    gp_abilities_list_free( abilList );
    gp_context_unref( context );
}

void GPCamera::getSupportedPorts(QStringList& plist)
{
    GPPortInfoList *list;
    GPPortInfo      info;

    plist.clear();

    gp_port_info_list_new( &list );
    gp_port_info_list_load( list );

    int numPorts = gp_port_info_list_count( list );
    if ( numPorts < 0) 
    {
        kdDebug() << "Failed to get list of port!" << endl;
        printGphotoErrorDescription(numPorts);
        gp_port_info_list_free( list );
        return;
    }
    else 
    {
        for (int i = 0 ; i < numPorts ; i++) 
        {
            gp_port_info_list_get_info( list, i, &info );
            plist.append( info.path );
        }
    }

    gp_port_info_list_free( list );
}

void GPCamera::getCameraSupportedPorts(const QString& model, QStringList& plist)
{
    int i = 0;
    plist.clear();

    CameraAbilities      abilities;
    CameraAbilitiesList *abilList;
    GPContext           *context;

    context = gp_context_new();

    gp_abilities_list_new (&abilList);
    gp_abilities_list_load (abilList, context);
    i = gp_abilities_list_lookup_model (abilList, model.local8Bit().data());
    gp_abilities_list_get_abilities (abilList, i, &abilities);
    gp_abilities_list_free (abilList);

    if (abilities.port & GP_PORT_SERIAL)
        plist.append("serial");
        
    if (abilities.port & GP_PORT_USB)
        plist.append("usb");

    gp_context_unref( context );
}

int GPCamera::autoDetect(QString& model, QString& port)
{
    CameraList          *camList;
    CameraAbilitiesList *abilList;
    GPPortInfoList      *infoList;
    const char          *camModel_, *camPort_;
    GPContext           *context;

    context = gp_context_new();
    gp_list_new(&camList);

    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, context);
    gp_port_info_list_new(&infoList);
    gp_port_info_list_load(infoList);
    gp_abilities_list_detect(abilList, infoList, camList, context);
    gp_abilities_list_free(abilList);
    gp_port_info_list_free(infoList);

    gp_context_unref(context);

    int count = gp_list_count(camList);

    if (count <= 0) 
    {
        kdDebug() << "Failed to autodetect camera!" << endl;
        printGphotoErrorDescription(count);
        gp_list_free(camList);
        return -1;
    }

    for (int i = 0 ; i < count ; i++) 
    {
        gp_list_get_name (camList, i, &camModel_);
        gp_list_get_value(camList, i, &camPort_);
    }

    model = camModel_;
    port  = camPort_;
    gp_list_free(camList);

    return 0;
}

}  // namespace Digikam
