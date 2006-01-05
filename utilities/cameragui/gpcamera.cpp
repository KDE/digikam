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

// C Ansi includes.

extern "C"
{
#include <stdio.h>
#include <gphoto2.h>
}

// C++ includes.

#include <iostream>

// QT includes.

#include <qstring.h>
#include <qstringlist.h>
#include <qimage.h>
#include <qdom.h>
#include <qfile.h>

// Local includes.

#include "gpcamera.h"

namespace Digikam
{

class GPCameraPrivate
{
public:

    Camera *camera;
    CameraAbilities cameraAbilities;

    QString model;
    QString port;
    QString globalPath;

    bool cameraInitialized;
    
    bool thumbnailSupport;
    bool deleteSupport;
    bool uploadSupport;
    bool mkDirSupport;
    bool delDirSupport;
    
};

class GPStatus
{
public:

    GPStatus() {
        context = gp_context_new();
        cancel  = false;
        gp_context_set_cancel_func(context, cancel_func, 0);
    }

    ~GPStatus() {
        gp_context_unref(context);
        cancel = false;
    }

    GPContext*  context;
    static bool cancel;

    static GPContextFeedback cancel_func(GPContext *,
                                         void *) {
        return (cancel ? GP_CONTEXT_FEEDBACK_CANCEL :
                GP_CONTEXT_FEEDBACK_OK);
    }            
};

bool GPStatus::cancel = false;

GPCamera::GPCamera(const QString& model,
                   const QString& port,
                   const QString& path)
    : DKCamera(model, port, path)
{
    status = 0;
    
    d = new GPCameraPrivate;
    d->camera = 0;

    d->model = model;
    d->port  = port;
    d->globalPath = path;
               
    d->cameraInitialized = false;

    d->thumbnailSupport = false;
    d->deleteSupport    = false;
    d->uploadSupport    = false;
    d->mkDirSupport     = false;
    d->delDirSupport    = false;
}

GPCamera::~GPCamera()
{
    if (d->camera) {
        gp_camera_unref(d->camera);
        d->camera = 0;
    }

    delete d;
}

bool GPCamera::connect()
{
    // -- first step - setup the camera --------------------
    
    if (d->camera) {
        gp_camera_unref(d->camera);
        d->camera = 0;
    }
    
    CameraAbilitiesList *abilList;
    GPPortInfoList      *infoList;
    GPPortInfo           info;

    gp_camera_new(&d->camera);

    if (status) {
        delete status;
        status = 0;
    }
            
    status = new GPStatus();
    
    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, status->context);
    gp_port_info_list_new(&infoList);
    gp_port_info_list_load(infoList);

    delete status;
    status = 0;

    int modelNum = -1, portNum = -1;
    modelNum = gp_abilities_list_lookup_model(abilList,
                                              d->model.latin1());
    portNum = gp_port_info_list_lookup_path (infoList,
                                             d->port.latin1());

    gp_abilities_list_get_abilities(abilList, modelNum,
                                    &d->cameraAbilities);
    
    if (gp_camera_set_abilities(d->camera, d->cameraAbilities)
        != GP_OK) {
        gp_camera_unref(d->camera);
        d->camera = 0;
        gp_abilities_list_free(abilList);
        gp_port_info_list_free(infoList);
        return false;
    }

    if (d->model != "Directory Browse") {
        gp_port_info_list_get_info(infoList, portNum, &info);
        if (gp_camera_set_port_info(d->camera, info) != GP_OK) {
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


    status = new GPStatus();

    // Try and initialize the camera to see if its connected
    if (gp_camera_init(d->camera, status->context) != GP_OK) {
        gp_camera_unref(d->camera);
        d->camera = 0;
        delete status;
        status = 0;
        return false;
    }

    delete status;
    status = 0;
    
    d->cameraInitialized = true;    
    return true;
}

void GPCamera::cancel()
{
    if (!status)
        return;
    status->cancel = true;
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

void GPCamera::getAllFolders(const QString& rootFolder,
                             QStringList& folderList)
{
    QStringList subfolders;
    getSubFolders(rootFolder, subfolders);

    for (QStringList::iterator it = subfolders.begin();
         it != subfolders.end(); ++it)
    {
        *it = rootFolder + QString(rootFolder.endsWith("/") ? "" : "/")
              + (*it);
        folderList.append(*it);
    }

    for (QStringList::iterator it = subfolders.begin();
         it != subfolders.end(); ++it)
    {
        getAllFolders(*it, folderList);
    }
}

bool GPCamera::getSubFolders(const QString& folder,
                             QStringList& subFolderList)
{
    CameraList *clist;
    gp_list_new(&clist);

    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus();

    if (gp_camera_folder_list_folders(d->camera,
                                      QFile::encodeName(folder),
                                      clist, status->context)
        != GP_OK) {

        gp_list_unref(clist);
        delete status;
        status = 0;
        return false;
    }

    delete status;
    status = 0;

    int count = gp_list_count(clist);
    for (int i=0; i<count; i++) {

        const char* subFolder;

        if (gp_list_get_name(clist, i, &subFolder) != GP_OK) {
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
    CameraList *clist;
    const char *cname;

    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus;
    
    gp_list_new(&clist);
    if (gp_camera_folder_list_files(d->camera, QFile::encodeName(folder),
                                    clist,
                                    status->context) != GP_OK) {
        gp_list_unref(clist);
        delete status;
        status = 0;
        return false;
    }

    int count = gp_list_count(clist);

    for (int i=0; i<count; i++)
    {
        if (gp_list_get_name(clist, i, &cname) != GP_OK)
        {
            gp_list_unref(clist);
            delete status;
            status = 0;
            return false;
        }

        itemsList.append(cname);
    }

    gp_list_unref(clist);        
    
    delete status;
    status = 0;

    return true;
}

bool GPCamera::getItemsInfoList(const QString& folder, GPItemInfoList& items)
{
    CameraList *clist;
    const char *cname;

    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus;
    
    gp_list_new(&clist);
    if (gp_camera_folder_list_files(d->camera, QFile::encodeName(folder),
                                    clist,
                                    status->context) != GP_OK) {
        gp_list_unref(clist);
        delete status;
        status = 0;
        return false;
    }

    int count = gp_list_count(clist);

    for (int i=0; i<count; i++)
    {
        if (gp_list_get_name(clist, i, &cname) != GP_OK)
        {
            gp_list_unref(clist);
            delete status;
            status = 0;
            return false;
        }

        GPItemInfo itemInfo;

        itemInfo.name   = cname;
        itemInfo.folder = folder;

        CameraFileInfo info;
        gp_camera_file_get_info(d->camera, QFile::encodeName(folder),
                                cname, &info, status->context);

        itemInfo.mtime            = -1;
        itemInfo.mime             = "";
        itemInfo.size             = -1;
        itemInfo.width            = -1;
        itemInfo.height           = -1;
        itemInfo.downloaded       = -1;
        itemInfo.readPermissions  = -1;
        itemInfo.writePermissions = -1;
        
        if (info.file.fields & GP_FILE_INFO_MTIME)
            itemInfo.mtime = info.file.mtime;
        
        if (info.file.fields & GP_FILE_INFO_TYPE)
            itemInfo.mime = info.file.type;

        if (info.file.fields & GP_FILE_INFO_SIZE)
            itemInfo.size = info.file.size;

        if (info.file.fields & GP_FILE_INFO_WIDTH)
            itemInfo.width = info.file.width;

        if (info.file.fields & GP_FILE_INFO_HEIGHT)
            itemInfo.height = info.file.height;

        if (info.file.fields & GP_FILE_INFO_STATUS) {
            if (info.file.status == GP_FILE_STATUS_DOWNLOADED)
                itemInfo.downloaded = 1;
            else
                itemInfo.downloaded = 0;
        }
        
        if (info.file.fields & GP_FILE_INFO_PERMISSIONS) {
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
    
    delete status;
    status = 0;

    return true;
}

bool GPCamera::getThumbnail(const QString& folder,
                            const QString& itemName,
                            QImage& thumbnail)
{
    CameraFile*       cfile;
    const char*       data;
    unsigned long int size;

    gp_file_new(&cfile);

    if (status) {
        delete status;
        status = 0;
    }
    
    status = new GPStatus;

    if (gp_camera_file_get(d->camera, QFile::encodeName(folder),
                           QFile::encodeName(itemName),
                           GP_FILE_TYPE_PREVIEW,
                           cfile, status->context) != GP_OK) {
        gp_file_unref(cfile);
        delete status;
        status = 0;
        return false;
    }

    delete status;
    status = 0;

    gp_file_get_data_and_size(cfile, &data, &size);
    thumbnail.loadFromData((const uchar*) data, (uint) size);

    gp_file_unref (cfile);
    return true;
}

bool GPCamera::getExif(const QString& folder,
                       const QString& itemName,
                       char **edata, int& esize)
{
    CameraFile *cfile;
    const char* data;
    unsigned long int size;
    
    gp_file_new(&cfile);
    
    if (status) {
        delete status;
        status = 0;
    }
    
    status = new GPStatus;

    if (gp_camera_file_get(d->camera, QFile::encodeName(folder),
                           QFile::encodeName(itemName),
                           GP_FILE_TYPE_EXIF,
                           cfile, status->context) != GP_OK) {
        gp_file_unref(cfile);
        delete status;
        status = 0;
        return false;
    }

    delete status;
    status = 0;
    
    gp_file_get_data_and_size(cfile, &data, &size);

    *edata = new char[size];
    esize = size;

    memcpy(*edata, data, size);

    gp_file_unref(cfile);

    return true;
}

bool GPCamera::downloadItem(const QString& folder,
                            const QString& itemName,
                            const QString& saveFile)
{
    CameraFile *cfile;
    gp_file_new(&cfile);

    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus;
    
    if (gp_camera_file_get(d->camera, QFile::encodeName(folder),
                           QFile::encodeName(itemName),
                           GP_FILE_TYPE_NORMAL, cfile,
                           status->context) != GP_OK) {
        gp_file_unref(cfile);
        delete status;
        status = 0;
        return false;
    }
    delete status;
    status = 0;

    if (gp_file_save(cfile, QFile::encodeName(saveFile)) != GP_OK) {
        gp_file_unref(cfile);
        return false;
    }
    gp_file_unref(cfile);

    return true;
}

bool GPCamera::deleteItem(const QString& folder,
                          const QString& itemName)
{
    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus;

    if (gp_camera_file_delete(d->camera, QFile::encodeName(folder),
                              QFile::encodeName(itemName),
                              status->context) != GP_OK) {
        delete status;
        status = 0;
        return false;
    }

    delete status;
    status = 0;

    return true;
}

// recursively delete all items
bool GPCamera::deleteAllItems(const QString& folder)
{
    QStringList folderList;

    // Get all subfolders in this folder
    getSubFolders(folder, folderList);

    if (folderList.count() > 0) {
        for (unsigned int i=0; i<folderList.count(); i++) {
            QString subFolder(folder);
            if (!subFolder.endsWith("/"))
                subFolder += "/";
            subFolder += folderList[i];

            deleteAllItems(subFolder);
        }
    }
    
   if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus;
    
    if (gp_camera_folder_delete_all(d->camera, QFile::encodeName(folder),
                                    status->context) != GP_OK) {
        delete status;
        status = 0;
        return false;
    }
    delete status;
    status = 0;

    return true;
}

bool GPCamera::uploadItem(const QString& folder,
                          const QString& itemName,
                          const QString& localFile)
{
    CameraFile *cfile;
    gp_file_new(&cfile);

    if (gp_file_open(cfile, QFile::encodeName(localFile)) != GP_OK) {
        gp_file_unref(cfile);
        return false;
    }

    gp_file_set_name(cfile, QFile::encodeName(itemName));

    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus;

    if (gp_camera_folder_put_file(d->camera,
                                  QFile::encodeName(folder),
                                  cfile,
                                  status->context) != GP_OK) {
        gp_file_unref(cfile);
        delete status;
        status = 0;
        return false;
    }

    gp_file_unref(cfile);
    delete status;
    status = 0;

    return true;
}

void GPCamera::cameraSummary(QString& summary)
{
    CameraText sum;

    if (status) {
        delete status;
        status = 0;
    }

    status = new GPStatus;
    gp_camera_get_summary(d->camera, &sum, status->context);
    summary = QString(sum.text);

    delete status;
    status = 0;
}

void GPCamera::cameraManual(QString& manual)
{
    CameraText man;

    if (status) {
        delete status;
        status = 0;
    }

    status = new GPStatus;
    gp_camera_get_manual(d->camera, &man, status->context);
    manual = QString(man.text);

    delete status;
    status = 0;
}

void GPCamera::cameraAbout(QString& about)
{
    CameraText abt;

    if (status) {
        delete status;
        status = 0;
    }

    status = new GPStatus;
    gp_camera_get_about(d->camera, &abt, status->context);
    about = QString(abt.text);

    delete status;
    status = 0;

}

// Static functions

void GPCamera::getSupportedCameras(int& count, QStringList& clist)
{
    clist.clear();
    count = 0;

    CameraAbilitiesList *abilList;
    CameraAbilities abil;
    GPContext *context;

    context = gp_context_new ();
 
    gp_abilities_list_new( &abilList );
    gp_abilities_list_load( abilList, context );

    count = gp_abilities_list_count( abilList );
    if ( count < 0) {
        gp_context_unref( context );
        qWarning("failed to get list of cameras");
        return;
    }
    else {
        for (int i=0; i<count; i++) {
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
    GPPortInfo info;

    plist.clear();

    gp_port_info_list_new( &list );
    gp_port_info_list_load( list );

    int numPorts = gp_port_info_list_count( list );

    for (int i = 0; i < numPorts; i++) {
        gp_port_info_list_get_info( list, i, &info );
        plist.append( info.path );
    }

    gp_port_info_list_free( list );

}

void GPCamera::getCameraSupportedPorts(const QString& model,
                                       QStringList& plist)
{
    int i = 0;
    plist.clear();

    CameraAbilities abilities;
    CameraAbilitiesList *abilList;
    GPContext *context;

    context = gp_context_new ();

    gp_abilities_list_new (&abilList);
    gp_abilities_list_load (abilList, context);
    i = gp_abilities_list_lookup_model (abilList,
                                        model.local8Bit().data());
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
    CameraList camList;
    CameraAbilitiesList *abilList;
    GPPortInfoList *infoList;
    const char *camModel_, *camPort_;
    GPContext *context;

    context = gp_context_new ();

    gp_abilities_list_new (&abilList);
    gp_abilities_list_load (abilList, context);
    gp_port_info_list_new (&infoList);
    gp_port_info_list_load (infoList);
    gp_abilities_list_detect (abilList, infoList,
                              &camList, context);
    gp_abilities_list_free (abilList);
    gp_port_info_list_free (infoList);

    gp_context_unref( context );

    int count = gp_list_count (&camList);

    if (count<=0) {
        return -1;
    }

    for (int i = 0; i < count; i++) {
        gp_list_get_name  (&camList, i, &camModel_);
        gp_list_get_value (&camList, i, &camPort_);
    }

    model = camModel_;
    port  = camPort_;

    return 0;
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

}  // namespace Digikam
