/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-21
 * Description : Gphoto2 camera interface
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "gpcamera.h"

// C ANSI includes

extern "C"
{
#include <utime.h>
#include <unistd.h>
}

// C++ includes

#include <cstdio>
#include <iostream>

// Qt includes

#include <QString>
#include <QStringList>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QDateTime>
#include <QTextDocument>
#include <QCryptographicHash>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dmetadata.h"

//#define GPHOTO2_DEBUG 1

#ifdef HAVE_GPHOTO2

// LibGphoto2 includes

extern "C"
{
#include <gphoto2.h>
}

#endif /* HAVE_GPHOTO2 */

namespace Digikam
{

class GPStatus
{

public:

    GPStatus()
    {
#ifdef HAVE_GPHOTO2
        context = gp_context_new();
        cancel  = false;
        gp_context_set_cancel_func(context, cancel_func, 0);
#ifdef GPHOTO2_DEBUG
        gp_context_set_progress_funcs(context, start_func, update_func, stop_func, 0);
        gp_context_set_error_func(context, error_func, 0);
        gp_context_set_status_func(context, status_func, 0);
#endif /* GPHOTO2_DEBUG */
#endif /* HAVE_GPHOTO2 */
    }

    ~GPStatus()
    {
#ifdef HAVE_GPHOTO2
        gp_context_unref(context);
        cancel = false;
#endif /* HAVE_GPHOTO2 */
    }

    static bool cancel;

#ifdef HAVE_GPHOTO2
    GPContext*  context;

    static GPContextFeedback cancel_func(GPContext*, void*)
    {
        return (cancel ? GP_CONTEXT_FEEDBACK_CANCEL :
                GP_CONTEXT_FEEDBACK_OK);
    }

#ifdef GPHOTO2_DEBUG
    static void error_func(GPContext*, const char* msg, void*) {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "error:" << msg;
    }
    static void status_func(GPContext*, const char* msg, void*) {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "status:" << msg;
    }

    static unsigned int start_func(GPContext*, float target, const char *text, void *data)
    {
        Q_UNUSED(data);
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "start:" << target << "- text:" << text;
        return 0;

    }
    static void update_func(GPContext*, unsigned int id, float target, void *data)
    {
        Q_UNUSED(data);
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "update:" << id << "- target:" << target;
    }
    static void stop_func(GPContext*, unsigned int id, void *data)
    {
        Q_UNUSED(data);
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "stop:" << id;
    }
#endif /* GPHOTO2_DEBUG */
#endif /* HAVE_GPHOTO2 */
};

bool GPStatus::cancel = false;

// ---------------------------------------------------------------------------

class GPCamera::Private
{

public:

    Private()
#ifdef HAVE_GPHOTO2
        : cameraInitialized(false),
          camera(0),
          status(0)
#endif /* HAVE_GPHOTO2 */
    {
    }

#ifdef HAVE_GPHOTO2
    bool            cameraInitialized;

    Camera*         camera;

    CameraAbilities cameraAbilities;

    GPStatus*       status;
#endif /* HAVE_GPHOTO2 */
};

// ---------------------------------------------------------------------------

GPCamera::GPCamera(const QString& title, const QString& model,
                   const QString& port, const QString& path)
    : DKCamera(title, model, port, path),
      d(new Private)
{
}

GPCamera::~GPCamera()
{
#ifdef HAVE_GPHOTO2
    if (d->status)
    {
        gp_context_unref(d->status->context);
        d->status = 0;
    }

    if (d->camera)
    {
        gp_camera_unref(d->camera);
        d->camera = 0;
    }

#endif /* HAVE_GPHOTO2 */

    delete d;
}

DKCamera::CameraDriverType GPCamera::cameraDriverType()
{
    return DKCamera::GPhotoDriver;
}

QByteArray GPCamera::cameraMD5ID()
{
    QByteArray md5data;

#ifdef HAVE_GPHOTO2
    QString    camData;
    // We don't use camera title from digiKam settings panel to compute MD5 fingerprint,
    // because it can be changed by users between session.
    camData.append(model());
    // TODO is it really necessary to have a path here? I think model+filename+size+ctime should be enough to give unique fingerprint
    // while still allowing you to move files around in the camera if needed
    camData.append(path());
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(camData.toUtf8());
    md5data = md5.result().toHex();
#endif /* HAVE_GPHOTO2 */

    return md5data;
}

bool GPCamera::doConnect()
{
#ifdef HAVE_GPHOTO2
    int errorCode;

    // -- first step - setup the camera --------------------

    if (d->camera)
    {
        gp_camera_unref(d->camera);
        d->camera = 0;
    }

    CameraAbilitiesList* abilList = 0;
    GPPortInfoList*      infoList = 0;
    GPPortInfo           info;

    gp_camera_new(&d->camera);

    delete d->status;
    d->status = 0;
    d->status = new GPStatus();

    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, d->status->context);
    gp_port_info_list_new(&infoList);
    gp_port_info_list_load(infoList);

    int modelNum     = gp_abilities_list_lookup_model(abilList, m_model.toLatin1().constData());
    int portNum      = gp_port_info_list_lookup_path(infoList, m_port.toLatin1().constData());

    gp_abilities_list_get_abilities(abilList, modelNum, &d->cameraAbilities);

    errorCode    = gp_camera_set_abilities(d->camera, d->cameraAbilities);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to set camera Abilities!";
        printGphotoErrorDescription(errorCode);
        gp_camera_unref(d->camera);
        d->camera = 0;
        gp_abilities_list_free(abilList);
        gp_port_info_list_free(infoList);
        return false;
    }

    if (m_model != QLatin1String("Directory Browse"))
    {
        gp_port_info_list_get_info(infoList, portNum, &info);
        errorCode = gp_camera_set_port_info(d->camera, info);

        if (errorCode != GP_OK)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to set camera port!";
            printGphotoErrorDescription(errorCode);
            gp_camera_unref(d->camera);
            d->camera = 0;
            gp_abilities_list_free(abilList);
            gp_port_info_list_free(infoList);
            return false;
        }
    }

    gp_abilities_list_free(abilList);
    gp_port_info_list_free(infoList);

    if (d->cameraAbilities.file_operations &
        GP_FILE_OPERATION_PREVIEW)
    {
        m_thumbnailSupport = true;
    }

    if (d->cameraAbilities.file_operations &
        GP_FILE_OPERATION_DELETE)
    {
        m_deleteSupport = true;
    }

    if (d->cameraAbilities.folder_operations &
        GP_FOLDER_OPERATION_PUT_FILE)
    {
        m_uploadSupport = true;
    }

    if (d->cameraAbilities.folder_operations &
        GP_FOLDER_OPERATION_MAKE_DIR)
    {
        m_mkDirSupport = true;
    }

    if (d->cameraAbilities.folder_operations &
        GP_FOLDER_OPERATION_REMOVE_DIR)
    {
        m_delDirSupport = true;
    }

    if (d->cameraAbilities.operations &
        GP_OPERATION_CAPTURE_IMAGE)
    {
        m_captureImageSupport = true;
    }

    if (d->cameraAbilities.operations &
        GP_OPERATION_CAPTURE_PREVIEW)
    {
        m_captureImagePreviewSupport = true;
    }

    // -- Try and initialize the camera to see if its connected -----------------

    errorCode = gp_camera_init(d->camera, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to initialize camera!";
        printGphotoErrorDescription(errorCode);
        gp_camera_unref(d->camera);
        d->camera = 0;
        return false;
    }

    d->cameraInitialized = true;

    return true;
#else
    return false;
#endif /* HAVE_GPHOTO2 */
}

void GPCamera::cancel()
{
#ifdef HAVE_GPHOTO2
    /* TODO what to do on cancel, if there's nothing to cancel?
    if (!d->status)
    {
        return;
    }*/

    d->status->cancel = true;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::getFreeSpace(unsigned long& kBSize, unsigned long& kBAvail)
{
#ifdef HAVE_GPHOTO2
    // NOTE: This method depends of libgphoto2 2.4.0

    int                       nrofsinfos;
    CameraStorageInformation* sinfos = 0;

    d->status->cancel = false;
    int errorCode = gp_camera_get_storageinfo(d->camera, &sinfos, &nrofsinfos, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Getting storage information not supported for this camera!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    for (int i = 0 ; i < nrofsinfos ; ++i)
    {
        if (sinfos[i].fields & GP_STORAGEINFO_FILESYSTEMTYPE)
        {
            switch(sinfos[i].fstype)
            {
                case GP_STORAGEINFO_FST_UNDEFINED:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage fstype: undefined";
                    break;
                case GP_STORAGEINFO_FST_GENERICFLAT:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage fstype: flat, all in one directory";
                    break;
                case GP_STORAGEINFO_FST_GENERICHIERARCHICAL:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage fstype: generic tree hierarchy";
                    break;
                case GP_STORAGEINFO_FST_DCF:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "DCIM style storage";
                    break;
            }
        }

        if (sinfos[i].fields & GP_STORAGEINFO_LABEL)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage label: " << QString::fromUtf8(sinfos[i].label);
        }

        if (sinfos[i].fields & GP_STORAGEINFO_DESCRIPTION)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage description: " << QString::fromUtf8(sinfos[i].description);
        }

        if (sinfos[i].fields & GP_STORAGEINFO_BASE)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage base-dir: " << QString::fromUtf8(sinfos[i].basedir);

            // TODO in order for this to work, the upload dialog needs to be fixed.
/*
            if(nrofsinfos == 1)
            {
                qCDebug(DIGIKAM_IMPORTUI_LOG) << "Only one storage, so setting storage directory to" << sinfos[i].basedir;
                m_path = QString(sinfos[i].basedir);
            }
*/
        }

        if (sinfos[i].fields & GP_STORAGEINFO_ACCESS)
        {
            switch (sinfos[i].access)
            {
                case GP_STORAGEINFO_AC_READWRITE:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage access: R/W";
                    break;

                case GP_STORAGEINFO_AC_READONLY:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage access: RO";
                    break;

                case GP_STORAGEINFO_AC_READONLY_WITH_DELETE:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage access: RO + Del";
                    break;

                default:
                    break;
            }
        }

        if (sinfos[i].fields & GP_STORAGEINFO_STORAGETYPE)
        {
            switch (sinfos[i].type)
            {
                case GP_STORAGEINFO_ST_FIXED_ROM:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage type: fixed ROM";
                    break;

                case GP_STORAGEINFO_ST_REMOVABLE_ROM:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage type: removable ROM";
                    break;

                case GP_STORAGEINFO_ST_FIXED_RAM:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage type: fixed RAM";
                    break;

                case GP_STORAGEINFO_ST_REMOVABLE_RAM:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage type: removable RAM";
                    break;

                case GP_STORAGEINFO_ST_UNKNOWN:
                default:
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage type: unknown";
                    break;
            }
        }

        if (sinfos[i].fields & GP_STORAGEINFO_MAXCAPACITY)
        {
            kBSize += sinfos[i].capacitykbytes;
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage capacity: " << kBSize;
        }

        if (sinfos[i].fields & GP_STORAGEINFO_FREESPACEKBYTES)
        {
            kBAvail += sinfos[i].freekbytes;
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Storage free-space: " << kBAvail;
        }
     }


    return true;
#else
    Q_UNUSED(kBSize);
    Q_UNUSED(kBAvail);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::getPreview(QImage& preview)
{
#ifdef HAVE_GPHOTO2
    int               errorCode;
    CameraFile*       cfile = 0;
    const char*       data  = 0;
    unsigned long int size;

    d->status->cancel = false;
    gp_file_new(&cfile);

    errorCode = gp_camera_capture_preview(d->camera, cfile, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to initialize camera preview mode!";
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    errorCode = gp_file_get_data_and_size(cfile, &data, &size);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get preview from camera!";
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    preview.loadFromData((const uchar*) data, (uint) size);

    gp_file_unref(cfile);
    return true;
#else
    Q_UNUSED(preview);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::capture(CamItemInfo& itemInfo)
{
#ifdef HAVE_GPHOTO2
    int            errorCode;
    CameraFilePath path;

    d->status->cancel = false;
    errorCode         = gp_camera_capture(d->camera, GP_CAPTURE_IMAGE, &path, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to take camera capture!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    // Get new camera item information.

    itemInfo.folder = QString::fromUtf8(path.folder);
    itemInfo.name   = QString::fromUtf8(path.name);

    CameraFileInfo info;
    errorCode       = gp_camera_file_get_info(d->camera, QFile::encodeName(itemInfo.folder).constData(),
                                              QFile::encodeName(itemInfo.name).constData(), &info,
                                              d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get camera item information!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    itemInfo.ctime            = QDateTime();
    itemInfo.mime             = QString();
    itemInfo.size             = -1;
    itemInfo.width            = -1;
    itemInfo.height           = -1;
    itemInfo.downloaded       = CamItemInfo::DownloadUnknown;
    itemInfo.readPermissions  = -1;
    itemInfo.writePermissions = -1;

    /* The mime type returned by Gphoto2 is dummy with all RAW files.
    if (info.file.fields & GP_FILE_INFO_TYPE)
        itemInfo.mime = info.file.type;*/

    itemInfo.mime = mimeType(itemInfo.name.section(QLatin1Char('.'), -1).toLower());

    if (info.file.fields & GP_FILE_INFO_MTIME)
    {
        itemInfo.ctime = QDateTime::fromTime_t(info.file.mtime);
    }

    if (info.file.fields & GP_FILE_INFO_SIZE)
    {
        itemInfo.size = info.file.size;
    }

    if (info.file.fields & GP_FILE_INFO_WIDTH)
    {
        itemInfo.width = info.file.width;
    }

    if (info.file.fields & GP_FILE_INFO_HEIGHT)
    {
        itemInfo.height = info.file.height;
    }

    if (info.file.fields & GP_FILE_INFO_STATUS)
    {
        if (info.file.status == GP_FILE_STATUS_DOWNLOADED)
        {
            itemInfo.downloaded = CamItemInfo::DownloadedYes;
        }
        else
        {
            itemInfo.downloaded = CamItemInfo::DownloadedNo;
        }
    }

    if (info.file.fields & GP_FILE_INFO_PERMISSIONS)
    {
        if (info.file.permissions & GP_FILE_PERM_READ)
        {
            itemInfo.readPermissions = 1;
        }
        else
        {
            itemInfo.readPermissions = 0;
        }

        if (info.file.permissions & GP_FILE_PERM_DELETE)
        {
            itemInfo.writePermissions = 1;
        }
        else
        {
            itemInfo.writePermissions = 0;
        }
    }

    return true;
#else
    Q_UNUSED(itemInfo);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::getFolders(const QString& folder)
{
#ifdef HAVE_GPHOTO2
    int         errorCode;
    CameraList* clist = 0;
    gp_list_new(&clist);

    d->status->cancel = false;
    errorCode         = gp_camera_folder_list_folders(d->camera, QFile::encodeName(folder).constData(), clist, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get folders list from camera!";
        printGphotoErrorDescription(errorCode);
        gp_list_unref(clist);
        return false;
    }

    QStringList subFolderList;
    int count = gp_list_count(clist);

    if (count < 1)
    {
        return true;
    }

    for (int i = 0 ; i < count ; ++i)
    {
        const char* subFolder = 0;
        errorCode             = gp_list_get_name(clist, i, &subFolder);

        if (errorCode != GP_OK)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get folder name from camera!";
            printGphotoErrorDescription(errorCode);
            gp_list_unref(clist);
            return false;
        }

        subFolderList.append(folder + QFile::decodeName(subFolder) + QLatin1Char('/'));
    }

    gp_list_unref(clist);

    emit signalFolderList(subFolderList);

    return true;
#else
    Q_UNUSED(folder);
    return false;
#endif /* HAVE_GPHOTO2 */
}

// TODO unused, remove?
bool GPCamera::getItemsList(const QString& folder, QStringList& itemsList)
{
#ifdef HAVE_GPHOTO2
    int         errorCode;
    CameraList* clist = 0;
    const char* cname = 0;

    gp_list_new(&clist);

    d->status->cancel = false;
    errorCode         = gp_camera_folder_list_files(d->camera, QFile::encodeName(folder).constData(), clist, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get folder files list from camera!";
        printGphotoErrorDescription(errorCode);
        gp_list_unref(clist);
        return false;
    }

    int count = gp_list_count(clist);

    for (int i = 0 ; i < count ; ++i)
    {
        errorCode = gp_list_get_name(clist, i, &cname);

        if (errorCode != GP_OK)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get file name from camera!";
            printGphotoErrorDescription(errorCode);
            gp_list_unref(clist);
            return false;
        }

        itemsList.append(QFile::decodeName(cname));
    }

    gp_list_unref(clist);

    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemsList);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::getItemsInfoList(const QString& folder, bool useMetadata, CamItemInfoList& items)
{
#ifdef HAVE_GPHOTO2
    int         errorCode;
    CameraList* clist = 0;
    const char* cname = 0;

    gp_list_new(&clist);

    d->status->cancel = false;
    errorCode         = gp_camera_folder_list_files(d->camera, QFile::encodeName(folder).constData(), clist, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get folder files list from camera!";
        printGphotoErrorDescription(errorCode);
        gp_list_unref(clist);
        return false;
    }

    int count = gp_list_count(clist);

    for (int i = 0 ; i < count ; ++i)
    {
        errorCode = gp_list_get_name(clist, i, &cname);

        if (errorCode != GP_OK)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get file name from camera!";
            printGphotoErrorDescription(errorCode);
            gp_list_unref(clist);
            return false;
        }

        // TODO for further speed-up, getItemInfoInternal call could be called separately when needed
        CamItemInfo info;
        getItemInfoInternal(folder, QFile::decodeName(cname), info, useMetadata);
        items.append(info);
    }

    gp_list_unref(clist);

    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(useMetadata);
    Q_UNUSED(items);
    return false;
#endif /* HAVE_GPHOTO2 */
}

void GPCamera::getItemInfo(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata)
{
#ifdef HAVE_GPHOTO2
    getItemInfoInternal(folder, itemName, info, useMetadata);
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(info);
    Q_UNUSED(useMetadata);
#endif /* HAVE_GPHOTO2 */
}

void GPCamera::getItemInfoInternal(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata)
{
#ifdef HAVE_GPHOTO2
    info.folder          = folder;
    info.name            = itemName;
    d->status->cancel    = false;
    info.previewPossible = m_captureImagePreviewSupport;

    CameraFileInfo cfinfo;
    gp_camera_file_get_info(d->camera, QFile::encodeName(info.folder).constData(),
                            QFile::encodeName(info.name).constData(), &cfinfo, d->status->context);

    // if preview has size field, it's a valid preview most likely, otherwise we'll skip it later on
    if (cfinfo.preview.fields & GP_FILE_INFO_SIZE)
    {
        info.previewPossible = true;
    }

    if (cfinfo.file.fields & GP_FILE_INFO_STATUS)
    {
        if (cfinfo.file.status == GP_FILE_STATUS_DOWNLOADED)
        {
            info.downloaded = CamItemInfo::DownloadedYes;
        }
    }

    if (cfinfo.file.fields & GP_FILE_INFO_SIZE)
    {
        info.size = cfinfo.file.size;
    }

    if (cfinfo.file.fields & GP_FILE_INFO_PERMISSIONS)
    {
        if (cfinfo.file.permissions & GP_FILE_PERM_READ)
        {
            info.readPermissions = 1;
        }
        else
        {
            info.readPermissions = 0;
        }

        if (cfinfo.file.permissions & GP_FILE_PERM_DELETE)
        {
            info.writePermissions = 1;
        }
        else
        {
            info.writePermissions = 0;
        }
    }

    /* The mime type returned by Gphoto2 is dummy with all RAW files.
        if (cfinfo.file.fields & GP_FILE_INFO_TYPE)
            info.mime = cfinfo.file.type;
    */

    info.mime = mimeType(info.name.section(QLatin1Char('.'), -1).toLower());

    if (!info.mime.isEmpty())
    {
        if (useMetadata)
        {
            // Try to use file metadata
            DMetadata meta;
            getMetadata(folder, itemName, meta);
            fillItemInfoFromMetadata(info, meta);

            // Fall back to camera file system info
            if (info.ctime.isNull())
            {
                if (cfinfo.file.fields & GP_FILE_INFO_MTIME)
                {
                    info.ctime = QDateTime::fromTime_t(cfinfo.file.mtime);
                }
                else
                {
                    info.ctime = QDateTime::currentDateTime();
                }
            }
        }
        else
        {
            // Only use properties provided by camera.
            if (cfinfo.file.fields & GP_FILE_INFO_MTIME)
            {
                info.ctime = QDateTime::fromTime_t(cfinfo.file.mtime);
            }
            else
            {
                info.ctime = QDateTime::currentDateTime();
            }

            if (cfinfo.file.fields & GP_FILE_INFO_WIDTH)
            {
                info.width = cfinfo.file.width;
            }

            if (cfinfo.file.fields & GP_FILE_INFO_HEIGHT)
            {
                info.height = cfinfo.file.height;
            }
        }
    }

#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(info);
    Q_UNUSED(useMetadata);
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail)
{
#ifdef HAVE_GPHOTO2
    int                errorCode;
    CameraFile*        cfile = 0;
    const char*        data  = 0;
    unsigned long int  size;

    gp_file_new(&cfile);

    d->status->cancel = false;
    errorCode         = gp_camera_file_get(d->camera, QFile::encodeName(folder).constData(),
                                           QFile::encodeName(itemName).constData(),
                                           GP_FILE_TYPE_PREVIEW,
                                           cfile, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get camera item!" << folder << itemName;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    errorCode = gp_file_get_data_and_size(cfile, &data, &size);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get thumbnail from camera item!" << folder << itemName;
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    thumbnail.loadFromData((const uchar*) data, (uint) size);

    gp_file_unref(cfile);
    return !thumbnail.isNull();
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(thumbnail);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::getMetadata(const QString& folder, const QString& itemName, DMetadata& meta)
{
#ifdef HAVE_GPHOTO2
    int               errorCode;
    CameraFile*       cfile = 0;
    const char*       data  = 0;
    unsigned long int size;

    gp_file_new(&cfile);

    d->status->cancel = false;
    errorCode         = gp_camera_file_get(d->camera, QFile::encodeName(folder).constData(),
                                           QFile::encodeName(itemName).constData(),
                                           GP_FILE_TYPE_EXIF,
                                           cfile, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get camera item!";
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    errorCode = gp_file_get_data_and_size(cfile, &data, &size);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get Exif data from camera item!";
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    QByteArray exifData(data, size);

    gp_file_unref(cfile);

    // Sometimes, GPhoto2 drivers return complete APP1 JFIF section. Exiv2 cannot
    // decode (yet) exif metadata from APP1. We will find Exif header to get data at this place
    // to please with Exiv2...

    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Size of Exif metadata from camera = " << exifData.size();

    if (!exifData.isEmpty())
    {
        char exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
        int i             = exifData.indexOf(*exifHeader);

        if (i != -1)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Exif header found at position " << i;
            i = i + sizeof(exifHeader);
            QByteArray data;
            data.resize(exifData.size() - i);
            memcpy(data.data(), exifData.data() + i, data.size());
            meta.setExif(data);
            return true;
        }
    }

    return false;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(meta);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::downloadItem(const QString& folder, const QString& itemName,
                            const QString& saveFile)
{
#ifdef HAVE_GPHOTO2
    int         errorCode;
    CameraFile* cfile = 0;

    d->status->cancel = false;
    QFile file(saveFile);

    if (!file.open(QIODevice::ReadWrite))
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to open file" << file.fileName() << file.errorString();
        return false;
    }

    // dup fd, passing fd control to gphoto2 later
    int handle = dup(file.handle());

    if (handle == -1)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to dup file descriptor";
        return false;
    }

    errorCode = gp_file_new_from_fd(&cfile, handle);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get camera item!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    errorCode = gp_camera_file_get(d->camera, QFile::encodeName(folder).constData(),
                                   QFile::encodeName(itemName).constData(),
                                   GP_FILE_TYPE_NORMAL, cfile,
                                   d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get camera item!";
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    time_t mtime;
    errorCode = gp_file_get_mtime(cfile, &mtime);

    if (errorCode == GP_OK && mtime)
    {
        struct utimbuf ut;
        ut.modtime = mtime;
        ut.actime  = mtime;
        ::utime(QFile::encodeName(saveFile).constData(), &ut);
    }

    file.close();

    gp_file_unref(cfile);
    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(saveFile);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::setLockItem(const QString& folder, const QString& itemName, bool lock)
{
#ifdef HAVE_GPHOTO2
    int errorCode;

    d->status->cancel = false;
    CameraFileInfo info;
    errorCode         = gp_camera_file_get_info(d->camera, QFile::encodeName(folder).constData(),
                                                QFile::encodeName(itemName).constData(), &info, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get camera item properties!";
        printGphotoErrorDescription(errorCode);
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

    errorCode = gp_camera_file_set_info(d->camera, QFile::encodeName(folder).constData(),
                                        QFile::encodeName(itemName).constData(), info, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to set camera item lock properties!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(lock);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::deleteItem(const QString& folder, const QString& itemName)
{
#ifdef HAVE_GPHOTO2
    d->status->cancel = false;
    int errorCode     = gp_camera_file_delete(d->camera, QFile::encodeName(folder).constData(),
                                              QFile::encodeName(itemName).constData(),
                                              d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to delete camera item!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    return false;
#endif /* HAVE_GPHOTO2 */
}

// TODO fix to go through all folders
// TODO this was never even used..
bool GPCamera::deleteAllItems(const QString& folder)
{
#ifdef HAVE_GPHOTO2
    int         errorCode;
    QStringList folderList;

    d->status->cancel = false;
    errorCode         = gp_camera_folder_delete_all(d->camera, QFile::encodeName(folder).constData(),
                                                    d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to delete camera folder!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    return true;
#else
    Q_UNUSED(folder);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::uploadItem(const QString& folder, const QString& itemName, const QString& localFile, CamItemInfo& itemInfo)
{
#ifdef HAVE_GPHOTO2
    int         errorCode;
    CameraFile* cfile = 0;
    errorCode         = gp_file_new(&cfile);
    d->status->cancel = false;

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to init new camera file instance!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    errorCode = gp_file_open(cfile, QFile::encodeName(localFile).constData());

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to open file!";
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    errorCode = gp_file_set_name(cfile, QFile::encodeName(itemName).constData());

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to rename item from camera!";
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

#ifdef HAVE_GPHOTO25
    errorCode = gp_camera_folder_put_file(d->camera,
                                          QFile::encodeName(folder).constData(),
                                          QFile::encodeName(itemName).constData(),
                                          GP_FILE_TYPE_NORMAL,
                                          cfile,
                                          d->status->context);
#else
    errorCode = gp_camera_folder_put_file(d->camera,
                                          QFile::encodeName(folder).constData(),
                                          cfile,
                                          d->status->context);
#endif

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to upload item to camera!";
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    // Get new camera item information.

    itemInfo.name   = itemName;
    itemInfo.folder = folder;

    CameraFileInfo info;
    errorCode       = gp_camera_file_get_info(d->camera, QFile::encodeName(folder).constData(),
                                              QFile::encodeName(itemName).constData(), &info, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get camera item information!";
        printGphotoErrorDescription(errorCode);
        gp_file_unref(cfile);
        return false;
    }

    itemInfo.ctime            = QDateTime();
    itemInfo.mime             = QString();
    itemInfo.size             = -1;
    itemInfo.width            = -1;
    itemInfo.height           = -1;
    itemInfo.downloaded       = CamItemInfo::DownloadUnknown;
    itemInfo.readPermissions  = -1;
    itemInfo.writePermissions = -1;

    /* The mime type returned by Gphoto2 is dummy with all RAW files.
    if (info.file.fields & GP_FILE_INFO_TYPE)
        itemInfo.mime = info.file.type;
    */

    itemInfo.mime = mimeType(itemInfo.name.section(QLatin1Char('.'), -1).toLower());

    if (info.file.fields & GP_FILE_INFO_MTIME)
    {
        itemInfo.ctime = QDateTime::fromTime_t(info.file.mtime);
    }

    if (info.file.fields & GP_FILE_INFO_SIZE)
    {
        itemInfo.size = info.file.size;
    }

    if (info.file.fields & GP_FILE_INFO_WIDTH)
    {
        itemInfo.width = info.file.width;
    }

    if (info.file.fields & GP_FILE_INFO_HEIGHT)
    {
        itemInfo.height = info.file.height;
    }

    if (info.file.fields & GP_FILE_INFO_STATUS)
    {
        if (info.file.status == GP_FILE_STATUS_DOWNLOADED)
        {
            itemInfo.downloaded = CamItemInfo::DownloadedYes;
        }
        else
        {
            itemInfo.downloaded = CamItemInfo::DownloadedNo;
        }
    }

    if (info.file.fields & GP_FILE_INFO_PERMISSIONS)
    {
        if (info.file.permissions & GP_FILE_PERM_READ)
        {
            itemInfo.readPermissions = 1;
        }
        else
        {
            itemInfo.readPermissions = 0;
        }

        if (info.file.permissions & GP_FILE_PERM_DELETE)
        {
            itemInfo.writePermissions = 1;
        }
        else
        {
            itemInfo.writePermissions = 0;
        }
    }

    gp_file_unref(cfile);
    return true;
#else
    Q_UNUSED(folder);
    Q_UNUSED(itemName);
    Q_UNUSED(localFile);
    Q_UNUSED(itemInfo);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::cameraSummary(QString& summary)
{
#ifdef HAVE_GPHOTO2
    int        errorCode;
    CameraText sum;

    d->status->cancel = false;
    errorCode         = gp_camera_get_summary(d->camera, &sum, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get camera summary!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    // we do not expect titel/model/etc. to contain newlines,
    // so we just escape HTML characters
    summary =  i18nc("@info List of device properties",
                     "Title: <b>%1</b><br/>"
                     "Model: <b>%2</b><br/>"
                     "Port: <b>%3</b><br/>"
                     "Path: <b>%4</b><br/><br/>",
                     title().toHtmlEscaped(),
                     model().toHtmlEscaped(),
                     port().toHtmlEscaped(),
                     path().toHtmlEscaped());

    summary += i18nc("@info List of supported device operations",
                     "Thumbnails: <b>%1</b><br/>"
                     "Capture image: <b>%2</b><br/>"
                     "Delete items: <b>%3</b><br/>"
                     "Upload items: <b>%4</b><br/>"
                     "Create directories: <b>%5</b><br/>"
                     "Delete Directories: <b>%6</b><br/><br/>",
                     thumbnailSupport()    ? i18n("yes") : i18n("no"),
                     captureImageSupport() ? i18n("yes") : i18n("no"),
                     deleteSupport()       ? i18n("yes") : i18n("no"),
                     uploadSupport()       ? i18n("yes") : i18n("no"),
                     mkDirSupport()        ? i18n("yes") : i18n("no"),
                     delDirSupport()       ? i18n("yes") : i18n("no"));

    // here we need to make sure whitespace and newlines
    // are converted to HTML properly
    summary.append(Qt::convertFromPlainText(QString::fromLocal8Bit(sum.text), Qt::WhiteSpacePre));

    return true;
#else
    Q_UNUSED(summary);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::cameraManual(QString& manual)
{
#ifdef HAVE_GPHOTO2
    int        errorCode;
    CameraText man;

    d->status->cancel = false;
    errorCode         = gp_camera_get_manual(d->camera, &man, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get camera manual!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    // I guess manual is plain text and not HTML?
    // Can't test it. (Michael G. Hansen)
    manual = Qt::convertFromPlainText(QString::fromLocal8Bit(man.text), Qt::WhiteSpacePre);

    return true;
#else
    Q_UNUSED(manual);
    return false;
#endif /* HAVE_GPHOTO2 */
}

bool GPCamera::cameraAbout(QString& about)
{
#ifdef HAVE_GPHOTO2
    int        errorCode;
    CameraText abt;

    d->status->cancel = false;
    errorCode         = gp_camera_get_about(d->camera, &abt, d->status->context);

    if (errorCode != GP_OK)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get information about camera!";
        printGphotoErrorDescription(errorCode);
        return false;
    }

    // here we need to make sure whitespace and newlines
    // are converted to HTML properly
    about = Qt::convertFromPlainText(QString::fromLocal8Bit(abt.text), Qt::WhiteSpacePre);
    about.append(QString::fromUtf8("<br/><br/>To report problems about this driver, please contact "
                 "the gphoto2 team at:<br/><br/>http://gphoto.org/bugs"));

    return true;
#else
    Q_UNUSED(about);
    return false;
#endif /* HAVE_GPHOTO2 */
}

// -- Static methods ---------------------------------------------------------------------

void GPCamera::printGphotoErrorDescription(int errorCode)
{
#ifdef HAVE_GPHOTO2
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Libgphoto2 error: " << gp_result_as_string(errorCode)
                          << " (" << errorCode << ")";
#else
    Q_UNUSED(errorCode);
#endif /* HAVE_GPHOTO2 */
}

void GPCamera::getSupportedCameras(int& count, QStringList& clist)
{
#ifdef HAVE_GPHOTO2
    clist.clear();
    count                         = 0;

    CameraAbilities      abil;
    CameraAbilitiesList* abilList = 0;
    GPContext*           context  = 0;
    context                       = gp_context_new();

    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, context);

    count                         = gp_abilities_list_count(abilList);

    if (count < 0)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get list of cameras!";
        printGphotoErrorDescription(count);
        gp_context_unref(context);
        return;
    }
    else
    {
        for (int i = 0 ; i < count ; i++)
        {
            gp_abilities_list_get_abilities(abilList, i, &abil);
            const char* cname = abil.model;
            clist.append(QString::fromLocal8Bit(cname));
        }
    }

    gp_abilities_list_free(abilList);
    gp_context_unref(context);
#else
    Q_UNUSED(count);
    Q_UNUSED(clist);
#endif /* HAVE_GPHOTO2 */
}

void GPCamera::getSupportedPorts(QStringList& plist)
{
#ifdef HAVE_GPHOTO2
    GPPortInfoList* list = 0;
    GPPortInfo      info;

    plist.clear();

    gp_port_info_list_new(&list);
    gp_port_info_list_load(list);

    int numPorts = gp_port_info_list_count(list);

    if (numPorts < 0)
    {
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get list of port!";
        printGphotoErrorDescription(numPorts);
        gp_port_info_list_free(list);
        return;
    }
    else
    {
        for (int i = 0 ; i < numPorts ; i++)
        {
            gp_port_info_list_get_info(list, i, &info);
#ifdef HAVE_GPHOTO25
            char* xpath = 0;
            gp_port_info_get_name (info, &xpath);
            plist.append(QString::fromUtf8(xpath));
#else
            plist.append(QString::fromUtf8(info.path));
#endif
        }
    }

    gp_port_info_list_free(list);
#else
    Q_UNUSED(plist);
#endif /* HAVE_GPHOTO2 */
}

void GPCamera::getCameraSupportedPorts(const QString& model, QStringList& plist)
{
#ifdef HAVE_GPHOTO2
    int i                         = 0;
    plist.clear();

    CameraAbilities      abilities;
    CameraAbilitiesList* abilList = 0;
    GPContext*           context  = 0;
    context                       = gp_context_new();

    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, context);
    i = gp_abilities_list_lookup_model(abilList, model.toLocal8Bit().data());
    gp_abilities_list_get_abilities(abilList, i, &abilities);
    gp_abilities_list_free(abilList);

    if (abilities.port & GP_PORT_SERIAL)
    {
        plist.append(QLatin1String("serial"));
    }

    if (abilities.port & GP_PORT_PTPIP)
    {
        plist.append(QLatin1String("ptpip"));
    }

    if (abilities.port & GP_PORT_USB)
    {
        plist.append(QLatin1String("usb"));
    }

    gp_context_unref(context);
#else
    Q_UNUSED(model);
    Q_UNUSED(plist);
#endif /* HAVE_GPHOTO2 */
}

int GPCamera::autoDetect(QString& model, QString& port)
{
#ifdef HAVE_GPHOTO2
    CameraList*          camList   = 0;
    CameraAbilitiesList* abilList  = 0;
    GPPortInfoList*      infoList  = 0;
    const char*          camModel_ = 0, *camPort_ = 0;
    GPContext*           context   = 0;
    context                        = gp_context_new();

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
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to autodetect camera!";
        printGphotoErrorDescription(count);
        gp_list_free(camList);
        return -1;
    }

    camModel_ = 0;
    camPort_  = 0;

    for (int i = 0; i < count; i++)
    {
        if (gp_list_get_name(camList, i, &camModel_) != GP_OK)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to autodetect camera!";
            gp_list_free(camList);
            return -1;
        }

        if (gp_list_get_value(camList, i, &camPort_) != GP_OK)
        {
            qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to autodetect camera!";
            gp_list_free(camList);
            return -1;
        }

        if (camModel_ && camPort_)
        {
            model = QString::fromLatin1(camModel_);
            port  = QString::fromLatin1(camPort_);
            gp_list_free(camList);
            return 0;
        }
    }

    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to autodetect camera!";
    gp_list_free(camList);
#else
    Q_UNUSED(model);
    Q_UNUSED(port);
#endif /* HAVE_GPHOTO2 */
    return -1;
}

bool GPCamera::findConnectedUsbCamera(int vendorId, int productId, QString& model, QString& port)
{
#ifdef HAVE_GPHOTO2
    CameraAbilitiesList* abilList = 0;
    GPPortInfoList*      list     = 0;
    GPContext*           context  = 0;
    CameraList*          camList  = 0;
    bool                 success  = false;
    // get name and port of detected camera
    const char* model_str         = 0;
    const char* port_str          = 0;
    context                       = gp_context_new();

    // get list of all ports
    gp_port_info_list_new(&list);
    gp_port_info_list_load(list);

    gp_abilities_list_new(&abilList);
    // get list of all supported cameras
    gp_abilities_list_load(abilList, context);

    // autodetect all cameras, then match the list to the passed in USB ids
    gp_list_new (&camList);
    gp_abilities_list_detect(abilList, list, camList, context);
    gp_context_unref(context);

    int count = gp_list_count(camList);
    int cnt   = 0;

    for (int i = 0 ; i < count ; i++)
    {
        const char* xmodel = 0;
        gp_list_get_name(camList, i, &xmodel);
        int model          = gp_abilities_list_lookup_model (abilList, xmodel);
        CameraAbilities ab;
        gp_abilities_list_get_abilities(abilList, model, &ab);

        if (ab.port != GP_PORT_USB)
            continue;

        /* KDE provides us USB Vendor and Product, but we might just
         * have covered this via a class match. Check class matched
         * cameras also for matchingo USB vendor/product id
         */
        if (ab.usb_vendor == 0)
        {
            int ret;
            GPPortInfo info;
            const char* xport = 0;
            GPPort* gpport    = 0;

            /* get the port path so we only look at this bus position */
            gp_list_get_value(camList, i, &xport);
            ret = gp_port_info_list_lookup_path (list, xport);

            if (ret < GP_OK) /* should not happen */
                continue;

            /* get the lowlevel port info  for the path
             */
            gp_port_info_list_get_info(list, ret, &info);

            /* open lowlevel driver interface briefly to search */
            gp_port_new(&gpport);
            gp_port_set_info(gpport, info);

            /* And now call into the lowlevel usb driver to see if the bus position
             * has that specific vendor/product id
             */
            if (gp_port_usb_find_device(gpport, vendorId, productId) == GP_OK)
            {
                ab.usb_vendor  = vendorId;
                ab.usb_product = productId;
            }

            gp_port_free (gpport);
        }

        if (ab.usb_vendor != vendorId)
            continue;

        if (ab.usb_product != productId)
            continue;

        /* keep it, and continue iterating, in case we find another one
         */
        gp_list_get_name (camList, i, &model_str);
        gp_list_get_value(camList, i, &port_str);

        cnt++;
    }

    gp_port_info_list_free(list);
    gp_abilities_list_free(abilList);

    if (cnt > 0)
    {
       if (cnt > 1)
       {
          qCWarning(DIGIKAM_IMPORTUI_LOG) << "More than one camera detected on port " << port
                                  << ". Due to restrictions in the GPhoto2 API, "
                                  << "only the first camera is used.";
       }

       model   = QString::fromLatin1(model_str);
       port    = QString::fromLatin1(port_str);
       success = true;
    }
    else
    {
       qCDebug(DIGIKAM_IMPORTUI_LOG) << "Failed to get information for the listed camera";
    }

    gp_list_free(camList);
    return success;
#else
    Q_UNUSED(vendorId);
    Q_UNUSED(productId);
    Q_UNUSED(model);
    Q_UNUSED(port);
    return false;
#endif /* HAVE_GPHOTO2 */
}

}  // namespace Digikam
