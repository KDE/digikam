/* ============================================================
 * File  : gpcamera.cpp
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

#include <kdebug.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qimage.h>
#include <qdom.h>

#include <iostream>

extern "C" {
#include <stdio.h>
#include <gphoto2.h>
}

#include "digikamcamera.h"
#include "gpstatus.h"
#include "gpcamera.h"

class GPCameraPrivate
{
public:

    DigikamCamera* parent;
    Camera *camera;
    CameraAbilities cameraAbilities;

    QString model;
    QString port;
    QString globalPath;

    bool cameraSetup;
    bool cameraInitialized;
    
    bool thumbnailSupport;
    bool deleteSupport;
    bool uploadSupport;
    bool mkDirSupport;
    bool delDirSupport;
    
};

GPCamera::GPCamera(DigikamCamera* parent,
                   const QString& model,
                   const QString& port,
                   const QString& path)
{
    status = 0;
    
    d = new GPCameraPrivate;
    d->parent = parent;
    d->camera = 0;

    d->model = model;
    d->port  = port;
    d->globalPath = path;
               

    d->cameraSetup       = false;
    d->cameraInitialized = false;

    d->thumbnailSupport = false;
    d->deleteSupport    = false;
    d->uploadSupport    = false;
    d->mkDirSupport     = false;
    d->delDirSupport    = false;

    setup();
}

GPCamera::~GPCamera()
{
    if (d->camera) {
        gp_camera_unref(d->camera);
        d->camera = 0;
    }

    delete d;
}

int GPCamera::setup()
{
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
        return GPSetup;
    }

    if (d->model != "Directory Browse") {
        gp_port_info_list_get_info(infoList, portNum, &info);
        if (gp_camera_set_port_info(d->camera, info) != GP_OK) {
            gp_camera_unref(d->camera);
            d->camera = 0;
            gp_abilities_list_free (abilList);
            gp_port_info_list_free (infoList);
            return GPSetup;
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


    d->cameraSetup = true;
    return GPSuccess;
    
}

int GPCamera::initialize()
{
    if (!d->cameraSetup || !d->camera) {
        int result = setup();
        if (result != GPSuccess)
            return result;
    }

    if (status) {
        delete status;
        status = 0;
    }

    status = new GPStatus();

    // Try and initialize the camera to see if its connected
    if (gp_camera_init(d->camera, status->context) != GP_OK) {
        gp_camera_unref(d->camera);
        d->camera = 0;
        delete status;
        status = 0;
        return GPInit;
    }

    delete status;
    status = 0;
    
    d->cameraInitialized = true;    
    return GPSuccess;
}

void GPCamera::cancel()
{
    if (!status) return;
    status->cancelOperation();
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

int GPCamera::getSubFolders(const QString& folder,
                            QValueList<QString>& subFolderList)
{
    CameraList *clist;
    gp_list_new(&clist);

    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus();

    if (gp_camera_folder_list_folders(d->camera, folder.latin1(),
                                      clist, status->context)
        != GP_OK) {

        gp_list_unref(clist);
        delete status;
        status = 0;
        return GPError;
    }

    delete status;
    status = 0;

    int count = gp_list_count(clist);
    for (int i=0; i<count; i++) {

        const char* subFolder;

        if (gp_list_get_name(clist, i, &subFolder) != GP_OK) {
            gp_list_unref(clist);
            return GPError;
        }

        subFolderList.append(QString(subFolder));

    }

    gp_list_unref(clist);

    return GPSuccess;
}

void GPCamera::getAllItemsInfo(const QString& folder)
{
    QValueList<QString> folderList;
    folderList.clear();

    // Get all items in this folder first
    getItemsInfo(folder);

    // Get all subfolders in this folder
    getSubFolders(folder, folderList);

    if (folderList.count() > 0) {
        for (unsigned int i=0; i<folderList.count(); i++) {
            QString subFolder(folder);
            if (!subFolder.endsWith("/"))
                subFolder += "/";
            subFolder += folderList[i];

            getAllItemsInfo(subFolder);
        }
    }
    
}

int GPCamera::getItemsInfo(const QString& folder)
{
    CameraList *clist;
    const char *cname;

    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus;
    
    gp_list_new(&clist);
    if (gp_camera_folder_list_files(d->camera, folder.latin1(),
                                    clist,
                                    status->context) != GP_OK) {
        gp_list_unref(clist);
        delete status;
        status = 0;
        return GPError;
    }

    int count = gp_list_count(clist);

    KIO::UDSEntry entry;
    KIO::UDSAtom  atom;
    KURL xurl;
    xurl.setProtocol("digikamcamera");
    
    for (int i=0; i<count; i++)
    {
        if (gp_list_get_name(clist, i, &cname) != GP_OK)
        {
            gp_list_unref(clist);
            delete status;
            status = 0;
            return GPError;
        }

        entry.clear();

        atom.m_uds  = KIO::UDS_FILE_TYPE;
        atom.m_long = S_IFREG;
        entry.append(atom);
        
        atom.m_uds = KIO::UDS_NAME;
        atom.m_str = cname;
        entry.append(atom);

        atom.m_uds = KIO::UDS_URL;
        xurl.setPath(folder);
        xurl.addPath(cname);
        atom.m_str = xurl.url();
        entry.append(atom);
        
        CameraFileInfo info;

        gp_camera_file_get_info(d->camera, folder.latin1(), cname,
                                &info, status->context);
                
        atom.m_uds  = KIO::UDS_MODIFICATION_TIME;
        if (info.file.fields & GP_FILE_INFO_MTIME)
        {
            atom.m_long = info.file.mtime;
        }
        else
        {
            atom.m_long = 0;
        }
        entry.append(atom);

        if (info.file.fields & GP_FILE_INFO_TYPE)
        {
            atom.m_uds = KIO::UDS_MIME_TYPE;
            atom.m_str = info.file.type;
            entry.append(atom);
        }

        atom.m_uds = KIO::UDS_SIZE;
        if (info.file.fields & GP_FILE_INFO_SIZE)
        {
            atom.m_long = info.file.size;
        }
        else
        {
            atom.m_long = 0;
        }
        entry.append(atom);

        // we piggyback UDS_XML_PROPERTIES to send extra info to app
                
        atom.m_uds = KIO::UDS_XML_PROPERTIES;
        if (info.file.fields & GP_FILE_INFO_WIDTH)
            atom.m_str = QString::number(info.file.width) + "/";
        else
            atom.m_str = QString::number(0) + "/";
                    
        if (info.file.fields & GP_FILE_INFO_HEIGHT)
            atom.m_str += QString::number(info.file.height) + "/";
        else
            atom.m_str += QString::number(0) + "/";
                   
        if (info.file.fields & GP_FILE_INFO_STATUS)
        {
            if (info.file.status == GP_FILE_STATUS_DOWNLOADED)
                atom.m_str += QString::number(1) + "/";
            else
                atom.m_str += QString::number(0) + "/";
        }
        else
            atom.m_str += QString::number(-1) + "/";
        entry.append(atom);

        atom.m_uds = KIO::UDS_ACCESS;
        if (info.file.fields & GP_FILE_INFO_PERMISSIONS)
        {
            atom.m_long = 0;
            atom.m_long |= (info.file.permissions & GP_FILE_PERM_READ)
                           ? (S_IRUSR | S_IRGRP | S_IROTH) : 0;
            atom.m_long |= (info.file.permissions & GP_FILE_PERM_DELETE)
                           ? (S_IWUSR | S_IWGRP | S_IWOTH) : 0;
        }
        else
        {
            atom.m_long = S_IRUSR | S_IRGRP | S_IROTH;
        }
        entry.append(atom);

        d->parent->listEntry(entry, false);
        
    }

    gp_list_unref(clist);        
    
    delete status;
    status = 0;

    return GPSuccess;
}

int GPCamera::getThumbnail(const QString& folder,
                           const QString& imageName,
                           QImage& thumbnail)
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

    if (gp_camera_file_get(d->camera, folder.latin1(),
                           imageName.latin1(),
                           GP_FILE_TYPE_PREVIEW,
                           cfile, status->context) != GP_OK) {
        gp_file_unref(cfile);
        delete status;
        status = 0;
        return GPError;
    }

    delete status;
    status = 0;

    gp_file_get_data_and_size(cfile, &data, &size);
    thumbnail.loadFromData((const uchar*) data, (uint) size);

    gp_file_unref (cfile);
    
    return GPSuccess;
}

int GPCamera::getExif(const QString& folder,
                      const QString& imageName,
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

    if (gp_camera_file_get(d->camera, folder.latin1(),
                           imageName.latin1(),
                           GP_FILE_TYPE_EXIF,
                           cfile, status->context) != GP_OK) {
        gp_file_unref(cfile);
        delete status;
        status = 0;
        return GPError;
    }

    delete status;
    status = 0;
    
    gp_file_get_data_and_size(cfile, &data, &size);

    *edata = new char[size];
    esize = size;

    memcpy(*edata, data, size);

    gp_file_unref(cfile);

    return GPSuccess;
    
}

int GPCamera::downloadItem(const QString& folder,
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
    
    if (gp_camera_file_get(d->camera, folder.latin1(),
                           itemName.latin1(),
                           GP_FILE_TYPE_NORMAL, cfile,
                           status->context) != GP_OK) {
        gp_file_unref(cfile);
        delete status;
        status = 0;
        return GPError;
    }
    delete status;
    status = 0;

    if (gp_file_save(cfile, saveFile.latin1()) != GP_OK) {
        gp_file_unref(cfile);
        return GPError;
    }
    gp_file_unref(cfile);

    return GPSuccess;

}

int GPCamera::deleteItem(const QString& folder,
                         const QString& itemName)
{
    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus;

    if (gp_camera_file_delete(d->camera, folder.latin1(),
                              itemName.latin1(),
                              status->context) != GP_OK) {
        delete status;
        status = 0;
        return GPError;
    }

    delete status;
    status = 0;

    return GPSuccess;
}

// recursively delete all items
int GPCamera::deleteAllItems(const QString& folder)
{
    kdDebug() << "Deleting all items from folder: " << folder << endl;

    QValueList<QString> folderList;
    folderList.clear();

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
    
    if (gp_camera_folder_delete_all(d->camera, folder.latin1(),
                                    status->context) != GP_OK) {
        delete status;
        status = 0;
        return GPError;
    }
    delete status;
    status = 0;

    return GPSuccess;
}

int GPCamera::uploadItem(const QString& folder,
                         const QString& itemName,
                         const QString& localFile)
{
    CameraFile *cfile;
    gp_file_new(&cfile);

    if (gp_file_open(cfile, localFile.local8Bit()) != GP_OK) {
        gp_file_unref(cfile);
        return GPError;
    }

    gp_file_set_name(cfile, itemName.local8Bit());

    if (status) {
        delete status;
        status = 0;
    }
    status = new GPStatus;

    if (gp_camera_folder_put_file(d->camera,
                                  folder.latin1(),
                                  cfile,
                                  status->context) != GP_OK) {
        gp_file_unref(cfile);
        delete status;
        status = 0;
        return GPError;
    }

    gp_file_unref(cfile);
    delete status;
    status = 0;

    return GPSuccess;
   
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
