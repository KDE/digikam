/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : USB Mass Storage camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "umscamera.h"

// C ANSI includes

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
}

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMatrix>
#include <QStringList>
#include <QTextDocument>
#include <QtGlobal>

// KDE includes

#include <kdebug.h>
#include <kcodecs.h>
#include <kio/global.h>
#include <klocale.h>
#include <kmimetype.h>
#include <solid/device.h>
#include <solid/storageaccess.h>
#include <solid/storagedrive.h>
#include <solid/storagevolume.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>

// Local includes

#include "config-digikam.h"
#include "dimg.h"
#include "dmetadata.h"
#include "imagescanner.h"

namespace Digikam
{

UMSCamera::UMSCamera(const QString& title, const QString& model,
                     const QString& port, const QString& path)
    : DKCamera(title, model, port, path)
{
    m_cancel = false;
    getUUIDFromSolid();
}

UMSCamera::~UMSCamera()
{
}

// Method not supported by UMS camera.
bool UMSCamera::getPreview(QImage& /*preview*/)
{
    return false;
}

// Method not supported by UMS camera.
bool UMSCamera::capture(CamItemInfo& /*itemInfo*/)
{
    return false;
}

DKCamera::CameraDriverType UMSCamera::cameraDriverType()
{
    return DKCamera::UMSDriver;
}

QByteArray UMSCamera::cameraMD5ID()
{
    QString camData;
    // Camera media UUID is used (if available) to improve fingerprint value
    // registration to database. We want to be sure that MD5 sum is really unique.
    // We don't use camera information here. media UUID is enough because it came be
    // mounted by a card reader or a camera. In this case, "already downloaded" flag will
    // be independent of the device used to mount memory card.
    camData.append(uuid());
    KMD5 md5(camData.toUtf8());
    return md5.hexDigest();
}

bool UMSCamera::getFreeSpace(unsigned long& /*kBSize*/, unsigned long& /*kBAvail*/)
{
    return false; // NOTE: implemented in gui, outside the camera thread.
}

bool UMSCamera::doConnect()
{
    QFileInfo dir(m_path);

    if (!dir.exists() || !dir.isReadable() || !dir.isDir())
    {
        return false;
    }

    if (dir.isWritable())
    {
        m_deleteSupport = true;
        m_uploadSupport = true;
        m_mkDirSupport  = true;
        m_delDirSupport = true;
    }
    else
    {
        m_deleteSupport = false;
        m_uploadSupport = false;
        m_mkDirSupport  = false;
        m_delDirSupport = false;
    }

    m_thumbnailSupport    = true;   // UMS camera always support thumbnails.
    m_captureImageSupport = false;  // UMS camera never support capture mode.

    return true;
}

void UMSCamera::cancel()
{
    // set the cancel flag
    m_cancel = true;
}

bool UMSCamera::getFolders(const QString& folder)
{
    if (m_cancel)
    {
        return false;
    }

    QDir dir(folder);
    dir.setFilter(QDir::Dirs | QDir::Executable);

    const QFileInfoList list = dir.entryInfoList();

    if (list.isEmpty())
    {
        return true;
    }

    QFileInfoList::const_iterator fi;
    QStringList subFolderList;

    for (fi = list.constBegin() ; !m_cancel && (fi != list.constEnd()) ; ++fi)
    {
        if (fi->fileName() == "." || fi->fileName() == "..")
        {
            continue;
        }

        QString subFolder = folder + QString(folder.endsWith('/') ? "" : "/") + fi->fileName();
        subFolderList.append(subFolder);

    }

    if (subFolderList.isEmpty())
    {
        return true;
    }

    emit signalFolderList(subFolderList);

    return true;
}

bool UMSCamera::getItemsInfoList(const QString& folder, bool useMetadata, CamItemInfoList& infoList)
{
    m_cancel = false;
    infoList.clear();

    QDir dir(folder);
    dir.setFilter(QDir::Files);

    if (!dir.exists())
    {
        return false;
    }

    const QFileInfoList list = dir.entryInfoList();

    if (list.isEmpty())
    {
        return true;    // Nothing to do.
    }

    for (QFileInfoList::const_iterator fi = list.constBegin() ; !m_cancel && (fi != list.constEnd()) ; ++fi)
    {
        CamItemInfo info;
        getItemInfo(folder, fi->fileName(), info, useMetadata);
        infoList.append(info);
    }

    return true;
}

void UMSCamera::getItemInfo(const QString& folder, const QString& itemName, CamItemInfo& info, bool useMetadata)
{
    info.folder           = !folder.endsWith('/') ? folder + QString('/') : folder;
    info.name             = itemName;

    QFileInfo fi(info.folder + info.name);
    info.size             = fi.size();
    info.readPermissions  = fi.isReadable();
    info.writePermissions = fi.isWritable();
    info.mime             = mimeType(fi.suffix().toLower());

    if (!info.mime.isEmpty())
    {
        if (useMetadata)
        {
            // Try to use file metadata
            DMetadata meta;
            getMetadata(folder, itemName, meta);
            fillItemInfoFromMetadata(info, meta);

            // Fall back to file system info
            if (info.ctime.isNull())
            {
                info.ctime = ImageScanner::creationDateFromFilesystem(fi);
            }
        }
        else
        {
            // Only use file system date
            info.ctime = ImageScanner::creationDateFromFilesystem(fi);
        }
    }

    // if we have an image, allow previews
    // TODO allow video previews at some point?
    if(info.mime.startsWith("image/"))
    {
        info.previewPossible = true;
    }
}

bool UMSCamera::getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail)
{
    m_cancel     = false;
    QString path = folder + QString("/") + itemName;

    // Try to get preview from Exif data (good quality). Can work with Raw files

    DMetadata metadata(path);
    metadata.getImagePreview(thumbnail);

    if (!thumbnail.isNull())
    {
        return true;
    }

    // RAW files : try to extract embedded thumbnail using libkdcraw

    KDcrawIface::KDcraw::loadRawPreview(thumbnail, path);

    if (!thumbnail.isNull())
    {
        return true;
    }

    KSharedConfig::Ptr config  = KGlobal::config();
    KConfigGroup group         = config->group("Camera Settings");
    bool turnHighQualityThumbs = group.readEntry("TurnHighQualityThumbs", false);

    // Try to get thumbnail from Exif data (poor quality).
    if(!turnHighQualityThumbs)
    {
        thumbnail = metadata.getExifThumbnail(true);

        if (!thumbnail.isNull())
        {
            return true;
        }
    }

    // THM files: try to get thumbnail from '.thm' files if we didn't manage to get
    // thumbnail from Exif. Any cameras provides *.thm files like JPEG files with RAW files.
    // Using this way is always speed up than ultimate loading using DImg.
    // Note: the thumbnail extracted with this method can be in poor quality.
    // 2006/27/01 - Gilles - Tested with my Minolta Dynax 5D USM camera.

    QFileInfo fi(path);

    if (thumbnail.load(folder + QString("/") + fi.baseName() + QString(".thm")))        // Lowercase
    {
        if (!thumbnail.isNull())
        {
            return true;
        }
    }
    else if (thumbnail.load(folder + QString("/") + fi.baseName() + QString(".THM")))   // Uppercase
    {
        if (!thumbnail.isNull())
        {
            return true;
        }
    }

    // Finally, we trying to get thumbnail using DImg API (slow).

    kDebug() << "Use DImg loader to get thumbnail from : " << path;

    DImg dimgThumb;
    // skip loading the data we don't need to speed it up.
    dimgThumb.load(path, false /*loadMetadata*/, false /*loadICCData*/, false /*loadUniqueHash*/, false /*loadHistory*/);

    if (!dimgThumb.isNull())
    {
        thumbnail = dimgThumb.copyQImage();
        return true;
    }

    return false;
}

bool UMSCamera::getMetadata(const QString& folder, const QString& itemName, DMetadata& meta)
{
    QFileInfo fi, thmlo, thmup;
    bool ret = false;

    fi.setFile(folder    + QString("/") + itemName);
    thmlo.setFile(folder + QString("/") + fi.baseName() + QString(".thm"));
    thmup.setFile(folder + QString("/") + fi.baseName() + QString(".THM"));

    if (thmlo.exists())
    {
        // Try thumbnail sidecar files with lowercase extension.
        ret = meta.load(thmlo.filePath());
    }
    else if (thmup.exists())
    {
        // Try thumbnail sidecar files with uppercase extension.
        ret = meta.load(thmup.filePath());
    }
    else
    {
        // If no thumbnail sidecar file available, try to load image metadata for files.
        ret = meta.load(fi.filePath());
    }

    return ret;
}

bool UMSCamera::downloadItem(const QString& folder, const QString& itemName, const QString& saveFile)
{
    m_cancel     = false;
    QString src  = folder + QString("/") + itemName;
    QString dest = saveFile;

    QFile sFile(src);
    QFile dFile(dest);

    if (!sFile.open(QIODevice::ReadOnly))
    {
        kWarning() << "Failed to open source file for reading: " << src;
        return false;
    }

    if (!dFile.open(QIODevice::WriteOnly))
    {
        sFile.close();
        kWarning() << "Failed to open destination file for writing: " << dest;
        return false;
    }

    const int MAX_IPC_SIZE = (1024 * 32);
    char      buffer[MAX_IPC_SIZE];
    qint64    len;

    while (((len = sFile.read(buffer, MAX_IPC_SIZE)) != 0) && !m_cancel)
    {
        if ((len == -1) || (dFile.write(buffer, (quint64)len) != len))
        {
            sFile.close();
            dFile.close();
            return false;
        }
    }

    sFile.close();
    dFile.close();

    // Set the file modification time of the downloaded file to the original file.
    // NOTE: this behavior don't need to be managed through Setup/Metadata settings.
    struct stat st;

    if (::stat(QFile::encodeName(src), &st) == 0)
    {
        struct utimbuf ut;
        ut.modtime = st.st_mtime;
        ut.actime  = st.st_atime;

        ::utime(QFile::encodeName(dest), &ut);
    }

    return true;
}

bool UMSCamera::setLockItem(const QString& folder, const QString& itemName, bool lock)
{
    QString src = folder + QString("/") + itemName;

    if (lock)
    {
        // Lock the file to set read only flag
        if (::chmod(QFile::encodeName(src), S_IREAD) == -1)
        {
            return false;
        }
    }
    else
    {
        // Unlock the file to set read/write flag
        if (::chmod(QFile::encodeName(src), S_IREAD | S_IWRITE) == -1)
        {
            return false;
        }
    }

    return true;
}

bool UMSCamera::deleteItem(const QString& folder, const QString& itemName)
{
    m_cancel = false;

    // Any camera provide THM (thumbnail) file with real image. We need to remove it also.

    QFileInfo fi(folder + QString("/") + itemName);

    QFileInfo thmLo(folder + QString("/") + fi.baseName() + ".thm");          // Lowercase

    if (thmLo.exists())
    {
        ::unlink(QFile::encodeName(thmLo.filePath()));
    }

    QFileInfo thmUp(folder + QString("/") + fi.baseName() + ".THM");          // Uppercase

    if (thmUp.exists())
    {
        ::unlink(QFile::encodeName(thmUp.filePath()));
    }

    // Remove the real image.
    return (::unlink(QFile::encodeName(folder + QString("/") + itemName)) == 0);
}

bool UMSCamera::uploadItem(const QString& folder, const QString& itemName, const QString& localFile, CamItemInfo& info)
{
    m_cancel     = false;
    QString dest = folder + QString("/") + itemName;
    QString src  = localFile;

    QFile sFile(src);
    QFile dFile(dest);

    if (!sFile.open(QIODevice::ReadOnly))
    {
        kWarning() << "Failed to open source file for reading: " << src;
        return false;
    }

    if (!dFile.open(QIODevice::WriteOnly))
    {
        sFile.close();
        kWarning() << "Failed to open destination file for writing: " << dest;
        return false;
    }

    const int MAX_IPC_SIZE = (1024 * 32);

    char buffer[MAX_IPC_SIZE];

    qint64 len;

    while (((len = sFile.read(buffer, MAX_IPC_SIZE)) != 0) && !m_cancel)
    {
        if ((len == -1) || (dFile.write(buffer, (quint64)len) == -1))
        {
            sFile.close();
            dFile.close();
            return false;
        }
    }

    sFile.close();
    dFile.close();

    // Set the file modification time of the uploaded file to original file.
    // NOTE: this behavior don't need to be managed through Setup/Metadata settings.
    struct stat st;

    if (::stat(QFile::encodeName(src), &st) == 0)
    {
        struct utimbuf ut;
        ut.modtime = st.st_mtime;
        ut.actime  = st.st_atime;

        ::utime(QFile::encodeName(dest), &ut);
    }

    // Get new camera item information.

    PhotoInfoContainer pInfo;
    DMetadata          meta;
    QFileInfo          fi(dest);
    QString            mime = mimeType(fi.suffix().toLower());

    if (!mime.isEmpty())
    {
        QSize     dims;
        QDateTime dt;

        // Try to load image metadata.
        meta.load(fi.filePath());
        dt    = meta.getImageDateTime();
        dims  = meta.getImageDimensions();
        pInfo = meta.getPhotographInformation();

        if (dt.isNull()) // fall back to file system info
        {
            ImageScanner::creationDateFromFilesystem(fi);
        }

        info.name             = fi.fileName();
        info.folder           = !folder.endsWith('/') ? folder + QString("/") : folder;
        info.mime             = mime;
        info.ctime            = dt;
        info.size             = fi.size();
        info.width            = dims.width();
        info.height           = dims.height();
        info.downloaded       = CamItemInfo::DownloadUnknown;
        info.readPermissions  = fi.isReadable();
        info.writePermissions = fi.isWritable();
        info.photoInfo        = pInfo;
    }

    return true;
}

bool UMSCamera::cameraSummary(QString& summary)
{
    summary =  QString(i18n("<b>Mounted Camera</b> driver for USB/IEEE1394 mass storage cameras and "
                            "Flash disk card readers.<br/><br/>"));

    // we do not expect titel/model/etc. to contain newlines,
    // so we just escape HTML characters
    summary += i18nc("@info List of device properties",
                     "Title: <b>%1</b><br/>"
                     "Model: <b>%2</b><br/>"
                     "Port: <b>%3</b><br/>"
                     "Path: <b>%4</b><br/>"
                     "UUID: <b>%5</b><br/><br/>",
                     Qt::escape(title()),
                     Qt::escape(model()),
                     Qt::escape(port()),
                     Qt::escape(path()),
                     Qt::escape(uuid()));

    summary += i18nc("@info List of supported device operations",
                     "Thumbnails: <b>%1</b><br/>"
                     "Capture image: <b>%2</b><br/>"
                     "Delete items: <b>%3</b><br/>"
                     "Upload items: <b>%4</b><br/>"
                     "Create directories: <b>%5</b><br/>"
                     "Delete directories: <b>%6</b><br/><br/>",
                     thumbnailSupport()    ? i18n("yes") : i18n("no"),
                     captureImageSupport() ? i18n("yes") : i18n("no"),
                     deleteSupport()       ? i18n("yes") : i18n("no"),
                     uploadSupport()       ? i18n("yes") : i18n("no"),
                     mkDirSupport()        ? i18n("yes") : i18n("no"),
                     delDirSupport()       ? i18n("yes") : i18n("no"));
    return true;
}

bool UMSCamera::cameraManual(QString& manual)
{
    manual = QString(i18n("For more information about the <b>Mounted Camera</b> driver, "
                          "please read the <b>Supported Digital Still "
                          "Cameras</b> section in the digiKam manual."));
    return true;
}

bool UMSCamera::cameraAbout(QString& about)
{
    about = QString(i18n("The <b>Mounted Camera</b> driver is a simple interface to a camera disk "
                         "mounted locally on your system.<br/><br/>"
                         "It does not use libgphoto2 drivers.<br/><br/>"
                         "To report any problems with this driver, please contact the digiKam team at:<br/><br/>"
                         "http://www.digikam.org/?q=contact"));
    return true;
}

void UMSCamera::getUUIDFromSolid()
{
    QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    foreach(const Solid::Device& accessDevice, devices)
    {
        // check for StorageAccess
        if (!accessDevice.is<Solid::StorageAccess>())
        {
            continue;
        }

        const Solid::StorageAccess* const access = accessDevice.as<Solid::StorageAccess>();

        if (!access->isAccessible())
        {
            continue;
        }

        // check for StorageDrive
        Solid::Device driveDevice;

        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid();
             currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageDrive>())
            {
                driveDevice = currentDevice;
                break;
            }
        }

        if (!driveDevice.isValid())
        {
            continue;
        }

        // check for StorageVolume
        Solid::Device volumeDevice;

        for (Solid::Device currentDevice = accessDevice; currentDevice.isValid(); currentDevice = currentDevice.parent())
        {
            if (currentDevice.is<Solid::StorageVolume>())
            {
                volumeDevice = currentDevice;
                break;
            }
        }

        if (!volumeDevice.isValid())
        {
            continue;
        }

        Solid::StorageVolume* const volume = volumeDevice.as<Solid::StorageVolume>();

        if (m_path.startsWith(QDir::fromNativeSeparators(access->filePath())))
        {
            m_uuid = volume->uuid();
        }
    }
}

}  // namespace Digikam
