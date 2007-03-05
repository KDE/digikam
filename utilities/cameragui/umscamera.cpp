/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com> 
 * Date   : 2004-12-21
 * Description : USB Mass Storage camera interface
 *
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2005-2007 by Gilles Caulier
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

// QT includes.

#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qdeepcopy.h>
#include <qwmatrix.h>

// KDE includes.

#include <klocale.h>
#include <kfilemetainfo.h>

// LibKDcraw includes.

#include <libkdcraw/kdcraw.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dmetadata.h"
#include "umscamera.h"

namespace Digikam
{

UMSCamera::UMSCamera(const QString& title, const QString& model, const QString& port, const QString& path)
         : DKCamera(title, model, port, path)
{
    m_cancel = false;
}

UMSCamera::~UMSCamera()
{
}

bool UMSCamera::doConnect()
{
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

    const QFileInfoList *list = dir.entryInfoList();
    if (!list)
        return false;

    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    while ((fi = it.current()) != 0 && !m_cancel)
    {
        ++it;

        QString mime = mimeType(fi->extension(false).lower());

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
                    if (meta.isValid())
                    {
                        if (meta.containsGroup("Jpeg EXIF Data"))
                            dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
                        else if (meta.containsGroup("General"))
                            dims = meta.group("General").item("Dimensions").value().toSize();
                        else if (meta.containsGroup("Technical"))
                            dims = meta.group("Technical").item("Dimensions").value().toSize();
                    }
                }
            }

            info.name             = fi->fileName();
            info.folder           = !folder.endsWith("/") ? folder + QString("/") : folder;
            info.mime             = mime;
            info.mtime            = fi->lastModified().toTime_t();
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
    DWarning() << "exif implemented yet in camera controller" << endl;
    return false;
}

bool UMSCamera::downloadItem(const QString& folder, const QString& itemName, const QString& saveFile)
{
    m_cancel = false;

    QString src  = folder + QString("/") + itemName;
    QString dest = saveFile;

    QFile sFile(src);
    QFile dFile(dest);

    if ( !sFile.open(IO_ReadOnly) )
    {
        DWarning() << "Failed to open source file for reading: "
                    << src << endl;
        return false;
    }

    if ( !dFile.open(IO_WriteOnly) )
    {
        sFile.close();
        DWarning() << "Failed to open dest file for writing: "
                    << dest << endl;
        return false;
    }

    const int MAX_IPC_SIZE = (1024*32);
    char buffer[MAX_IPC_SIZE];

    Q_LONG len;
    while ((len = sFile.readBlock(buffer, MAX_IPC_SIZE)) != 0 && !m_cancel)
    {
        if (len == -1 || dFile.writeBlock(buffer, (Q_ULONG)len) != len)
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

    if ( !sFile.open(IO_ReadOnly) )
    {
        DWarning() << "Failed to open source file for reading: "
                    << src << endl;
        return false;
    }

    if ( !dFile.open(IO_WriteOnly) )
    {
        sFile.close();
        DWarning() << "Failed to open dest file for writing: "
                    << dest << endl;
        return false;
    }

    const int MAX_IPC_SIZE = (1024*32);
    char buffer[MAX_IPC_SIZE];

    Q_LONG len;
    while ((len = sFile.readBlock(buffer, MAX_IPC_SIZE)) != 0 && !m_cancel)
    {
        if (len == -1 || dFile.writeBlock(buffer, (Q_ULONG)len) == -1)
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
    QString mime = mimeType(fi.extension(false).lower());

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
                if (meta.isValid())
                {
                    if (meta.containsGroup("Jpeg EXIF Data"))
                        dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
                    else if (meta.containsGroup("General"))
                        dims = meta.group("General").item("Dimensions").value().toSize();
                    else if (meta.containsGroup("Technical"))
                        dims = meta.group("Technical").item("Dimensions").value().toSize();
                }
            }
        }

        itemInfo.name             = fi.fileName();
        itemInfo.folder           = !folder.endsWith("/") ? folder + QString("/") : folder;
        itemInfo.mime             = mime;
        itemInfo.mtime            = fi.lastModified().toTime_t();
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

    const QFileInfoList *list = dir.entryInfoList();
    if (!list)
        return;

    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    while ((fi = it.current()) != 0 && !m_cancel)
    {
        ++it;

        if (fi->fileName() == "." || fi->fileName() == "..")
            continue;

        QString subfolder = folder + QString(folder.endsWith("/") ? "" : "/") + fi->fileName();
        subFolderList.append(subfolder);
        listFolders(subfolder, subFolderList);
    }
}

bool UMSCamera::cameraSummary(QString& summary)
{
    summary = QString(i18n("<b>Mounted Camera</b> driver for USB/IEEE1394 mass storage cameras and "
                           "Flash disk card readers.<br><br>"));

    summary.append(i18n("Title: %1<br>"
                        "Model: %2<br>"
                        "Port: %3<br>"
                        "Path: %4<br>")
                        .arg(title())
                        .arg(model())
                        .arg(port())
                        .arg(path()));
    return true;
}

bool UMSCamera::cameraManual(QString& manual)
{
    manual = QString(i18n("For more information about the <b>Mounted Camera</b> driver, "
                          "please read the digiKam manual on <b>Supported Digital Still "
                          "Cameras</b> section."));
    return true;
}

bool UMSCamera::cameraAbout(QString& about)
{
    about = QString(i18n("The <b>Mounted Camera</b> driver is a simple interface to a camera disk "
                         "mounted locally on your system.<br><br>"
                         "It doesn't use any libgphoto2 drivers.<br><br>"
                         "To report any problems with this driver, please contact the digiKam team at:<br><br>"
                         "http://www.digikam.org/?q=contact"));
    return true;
}

}  // namespace Digikam
