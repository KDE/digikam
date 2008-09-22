/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-21
 * Description : USB Mass Storage camera interface
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
}

// Qt includes.

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QMatrix>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kfilemetainfo.h>

// LibKDcraw includes.

#include <libkdcraw/kdcraw.h>

// Local includes.

#include "dimg.h"
#include "dmetadata.h"
#include "umscamera.h"

namespace Digikam
{

UMSCamera::UMSCamera(const QString& title, const QString& model,
                     const QString& port, const QString& path)
         : DKCamera(title, model, port, path)
{
    m_cancel = false;
}

UMSCamera::~UMSCamera()
{
}

bool UMSCamera::getFreeSpace(unsigned long& /*kBSize*/, unsigned long& /*kBAvail*/)
{
    return false; // NOTE: implemented in gui, outside the camera thread.
}

bool UMSCamera::doConnect()
{
    QFileInfo dir(m_path);
    if (!dir.exists() || !dir.isReadable() || !dir.isDir())
        return false;

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

void UMSCamera::getAllFolders(const QString& folder, QStringList& subFolderList)
{
    m_cancel = false;
    subFolderList.clear();
    subFolderList.append(folder);
    listFolders(folder, subFolderList);
}

bool UMSCamera::getItemsInfoList(const QString& folder, GPItemInfoList& infoList, bool getImageDimensions)
{
    m_cancel = false;
    infoList.clear();

    QDir dir(folder);
    dir.setFilter(QDir::Files);
    if (!dir.exists())
        return false;

    const QFileInfoList list = dir.entryInfoList();
    if (list.isEmpty())
        return true;        // Nothing todo.

    QFileInfoList::const_iterator fi;

    for (fi = list.constBegin() ; !m_cancel && (fi != list.constEnd()) ; ++fi)
    {
        QString mime = mimeType(fi->suffix().toLower());

        if (!mime.isEmpty())
        {
            GPItemInfo info;
            QSize dims(-1, -1);

            if (getImageDimensions)
            {
                if (mime == QString("image/x-raw"))
                {
                    DMetadata metaData(fi->filePath());
                    dims = metaData.getImageDimensions();
                }
                else
                {
                    KFileMetaInfo meta(fi->filePath());

#warning "TODO: kde4 port it";
                    /* TODO: KDE4PORT: KFileMetaInfo API as Changed.
                             Check if new method to search "Dimensions" information is enough.

                    if (meta.isValid())
                    {
                        if (meta.containsGroup("Jpeg EXIF Data"))
                            dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
                        else if (meta.containsGroup("General"))
                            dims = meta.group("General").item("Dimensions").value().toSize();
                        else if (meta.containsGroup("Technical"))
                            dims = meta.group("Technical").item("Dimensions").value().toSize();
                    }*/

                    if (meta.isValid() && meta.item("Dimensions").isValid())
                    {
                        dims = meta.item("Dimensions").value().toSize();
                    }
                }
            }

            info.name             = fi->fileName();
            info.folder           = !folder.endsWith('/') ? folder + QString('/') : folder;
            info.mime             = mime;
            info.mtime            = fi->lastModified();
            info.size             = fi->size();
            info.width            = dims.width();
            info.height           = dims.height();
            info.downloaded       = GPItemInfo::DownloadUnknow;
            info.readPermissions  = fi->isReadable();
            info.writePermissions = fi->isWritable();

            infoList.append(info);
        }
    }

    return true;
}

bool UMSCamera::getThumbnail(const QString& folder, const QString& itemName, QImage& thumbnail)
{
    m_cancel = false;

    // JPEG files: try to get thumbnail from Exif data.

    DMetadata metadata(QFile::encodeName(folder + QString("/") + itemName));
    thumbnail = metadata.getExifThumbnail(true);
    if (!thumbnail.isNull())
        return true;

    // RAW files : try to extract embedded thumbnail using dcraw

    KDcrawIface::KDcraw::loadDcrawPreview(thumbnail, QString(folder + QString("/") + itemName));
    if (!thumbnail.isNull())
        return true;

    // THM files: try to get thumbnail from '.thm' files if we didn't manage to get
    // thumbnail from Exif. Any cameras provides *.thm files like JPEG files with RAW files.
    // Using this way is always speed up than ultimate loading using DImg.
    // Nota: the thumbnail extracted with this method can be in poor quality.
    // 2006/27/01 - Gilles - Tested with my Minolta Dynax 5D USM camera.

    QFileInfo fi(folder + QString("/") + itemName);

    if (thumbnail.load(folder + QString("/") + fi.baseName() + ".thm"))        // Lowercase
    {
        if (!thumbnail.isNull())
           return true;
    }
    else if (thumbnail.load(folder + QString("/") + fi.baseName() + ".THM"))   // Uppercase
    {
        if (!thumbnail.isNull())
           return true;
    }


    // Finaly, we trying to get thumbnail using DImg API (slow).

    DImg dimgThumb(QFile::encodeName(folder + QString("/") + itemName));

    if (!dimgThumb.isNull())
    {
        thumbnail = dimgThumb.copyQImage();
        return true;
    }

    return false;
}

bool UMSCamera::getExif(const QString&, const QString&, char **, int&)
{
    // not necessary to implement this. read it directly from the file
    // (done in camera controller)
    kWarning(50003) << "exif implemented yet in camera controller" << endl;
    return false;
}

bool UMSCamera::downloadItem(const QString& folder, const QString& itemName, const QString& saveFile)
{
    m_cancel = false;

    QString src  = folder + QString("/") + itemName;
    QString dest = saveFile;

    QFile sFile(src);
    QFile dFile(dest);

    if ( !sFile.open(QIODevice::ReadOnly) )
    {
        kWarning(50003) << "Failed to open source file for reading: "
                        << src << endl;
        return false;
    }

    if ( !dFile.open(QIODevice::WriteOnly) )
    {
        sFile.close();
        kWarning(50003) << "Failed to open dest file for writing: "
                        << dest << endl;
        return false;
    }

    const int MAX_IPC_SIZE = (1024*32);
    char buffer[MAX_IPC_SIZE];

    Q_LONG len;
    while ((len = sFile.read(buffer, MAX_IPC_SIZE)) != 0 && !m_cancel)
    {
        if (len == -1 || dFile.write(buffer, (Q_ULONG)len) != len)
        {
            sFile.close();
            dFile.close();
            return false;
        }
    }

    sFile.close();
    dFile.close();

    // set the file modification time of the downloaded file to that
    // of the original file
    struct stat st;
    ::stat(QFile::encodeName(src), &st);

    struct utimbuf ut;
    ut.modtime = st.st_mtime;
    ut.actime  = st.st_atime;

    ::utime(QFile::encodeName(dest), &ut);

    return true;
}

bool UMSCamera::setLockItem(const QString& folder, const QString& itemName, bool lock)
{
    QString src  = folder + QString("/") + itemName;

    if (lock)
    {
        // Lock the file to set read only flag
        if (::chmod(QFile::encodeName(src), S_IREAD) == -1)
            return false;
    }
    else
    {
        // Unlock the file to set read/write flag
        if (::chmod(QFile::encodeName(src), S_IREAD | S_IWRITE) == -1)
            return false;
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
        ::unlink(QFile::encodeName(thmLo.filePath()));

    QFileInfo thmUp(folder + QString("/") + fi.baseName() + ".THM");          // Uppercase

    if (thmUp.exists())
        ::unlink(QFile::encodeName(thmUp.filePath()));

    // Remove the real image.
    return (::unlink(QFile::encodeName(folder + QString("/") + itemName)) == 0);
}

bool UMSCamera::uploadItem(const QString& folder, const QString& itemName, const QString& localFile,
                           GPItemInfo& itemInfo, bool getImageDimensions)
{
    m_cancel = false;

    QString dest = folder + QString("/") + itemName;
    QString src  = localFile;

    QFile sFile(src);
    QFile dFile(dest);

    if ( !sFile.open(QIODevice::ReadOnly) )
    {
        kWarning(50003) << "Failed to open source file for reading: "
                        << src << endl;
        return false;
    }

    if ( !dFile.open(QIODevice::WriteOnly) )
    {
        sFile.close();
        kWarning(50003) << "Failed to open dest file for writing: "
                        << dest << endl;
        return false;
    }

    const int MAX_IPC_SIZE = (1024*32);
    char buffer[MAX_IPC_SIZE];

    Q_LONG len;
    while ((len = sFile.read(buffer, MAX_IPC_SIZE)) != 0 && !m_cancel)
    {
        if (len == -1 || dFile.write(buffer, (Q_ULONG)len) == -1)
        {
            sFile.close();
            dFile.close();
            return false;
        }
    }

    sFile.close();
    dFile.close();

    // set the file modification time of the uploaded file to that
    // of the original file
    struct stat st;
    ::stat(QFile::encodeName(src), &st);

    struct utimbuf ut;
    ut.modtime = st.st_mtime;
    ut.actime  = st.st_atime;

    ::utime(QFile::encodeName(dest), &ut);

    // Get new camera item information.

    QFileInfo fi(dest);
    QString mime = mimeType(fi.suffix().toLower());

    if (!mime.isEmpty())
    {
        QSize dims(-1, -1);

        if (getImageDimensions)
        {
            if (mime == QString("image/x-raw"))
            {
                DMetadata metaData(fi.filePath());
                dims = metaData.getImageDimensions();
            }
            else
            {
                KFileMetaInfo meta(fi.filePath());

#warning "TODO: kde4 port it";
                /* TODO: KDE4PORT: KFileMetaInfo API as Changed.
                         Check if new method to search "Dimensions" information is enough.

                if (meta.isValid())
                {
                    if (meta.containsGroup("Jpeg EXIF Data"))
                        dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
                    else if (meta.containsGroup("General"))
                        dims = meta.group("General").item("Dimensions").value().toSize();
                    else if (meta.containsGroup("Technical"))
                        dims = meta.group("Technical").item("Dimensions").value().toSize();
                }*/

                if (meta.isValid() && meta.item("Dimensions").isValid())
                {
                    dims = meta.item("Dimensions").value().toSize();
                }
            }
        }

        itemInfo.name             = fi.fileName();
        itemInfo.folder           = !folder.endsWith('/') ? folder + QString('/') : folder;
        itemInfo.mime             = mime;
        itemInfo.mtime            = fi.lastModified();
        itemInfo.size             = fi.size();
        itemInfo.width            = dims.width();
        itemInfo.height           = dims.height();
        itemInfo.downloaded       = GPItemInfo::DownloadUnknow;
        itemInfo.readPermissions  = fi.isReadable();
        itemInfo.writePermissions = fi.isWritable();
    }

    return true;
}

void UMSCamera::listFolders(const QString& folder, QStringList& subFolderList)
{
    if (m_cancel)
        return;

    QDir dir(folder);
    dir.setFilter(QDir::Dirs|QDir::Executable);

    const QFileInfoList list = dir.entryInfoList();
    if (list.isEmpty())
        return;

    QFileInfoList::const_iterator fi;

    for (fi = list.constBegin() ; !m_cancel && (fi != list.constEnd()) ; ++fi)
    {
        if (fi->fileName() == "." || fi->fileName() == "..")
            continue;

        QString subfolder = folder + QString(folder.endsWith('/') ? "" : "/") + fi->fileName();
        subFolderList.append(subfolder);
        listFolders(subfolder, subFolderList);
    }
}

bool UMSCamera::cameraSummary(QString& summary)
{
    summary =  QString(i18n("<b>Mounted Camera</b> driver for USB/IEEE1394 mass storage cameras and "
                            "Flash disk card readers.<br><br>"));

    summary += i18n("Title: <b>%1</b><br>"
                    "Model: <b>%2</b><br>"
                    "Port: <b>%3</b><br>"
                    "Path: <b>%4</b><br><br>",
                    title(),
                    model(),
                    port(),
                    path());

    summary += i18n("Thumbnails: <b>%1</b><br>"
                    "Capture image: <b>%2</b><br>"
                    "Delete items: <b>%3</b><br>"
                    "Upload items: <b>%4</b><br>"
                    "Create directories: <b>%5</b><br>"
                    "Delete directories: <b>%6</b><br><br>",
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
                         "mounted locally on your system.<br><br>"
                         "It does not use libgphoto2 drivers.<br><br>"
                         "To report any problems with this driver, please contact the digiKam team at:<br><br>"
                         "http://www.digikam.org/?q=contact"));
    return true;
}

}  // namespace Digikam
