/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2004-12-21
 * Description : USB Mass Storage camera interface
 *
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2005-2006 by Gilles Caulier
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

#include <kdebug.h>
#include <kfilemetainfo.h>

// Local includes.

#include "dimg.h"
#include "dcrawpreview.h"
#include "dmetadata.h"
#include "umscamera.h"

namespace Digikam
{

UMSCamera::UMSCamera(const QString& model, const QString& port, const QString& path)
         : DKCamera(model, port, path)
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
            QSize   dims(-1, -1);

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
            info.folder           = folder;
            info.mime             = mime;
            info.mtime            = fi->lastModified().toTime_t();
            info.size             = fi->size();
            info.width            = dims.width();
            info.height           = dims.height();
            info.downloaded       = -1; // TODO
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

    // In 1st, we trying to get thumbnail from Exif data if we are JPEG file.

    DMetadata metadata(QFile::encodeName(folder + "/" + itemName));
    thumbnail = metadata.getExifThumbnail(true);
    if (!thumbnail.isNull())
        return true;

    // In 2th, we trying to get thumbnail from '.thm' files if we didn't manage to get 
    // thumbnail from Exif. Any cameras provides *.thm files like JPEG files with RAW files. 
    // Using this way is always more speed than using dcraw parse utility.
    // 2006/27/01 - Gilles - Tested with my Minolta Dynax 5D USM camera.

    QFileInfo fi(folder + "/" + itemName);

    if (thumbnail.load(folder + "/" + fi.baseName() + ".thm"))        // Lowercase
    {
        if (!thumbnail.isNull())
           return true;
    }
    else if (thumbnail.load(folder + "/" + fi.baseName() + ".THM"))   // Uppercase
    {
        if (!thumbnail.isNull())
           return true;
    }

    // In 3rd, if file image type is TIFF, load thumb using DImg befire to use dcraw::parse method 
    // to prevent broken 16 bits TIFF thumb.

    if (fi.extension().upper() == QString("TIFF") ||
        fi.extension().upper() == QString("TIF"))
    {
        DImg dimgThumb(QFile::encodeName(folder + "/" + itemName));
        if (!dimgThumb.isNull())
        {
            thumbnail = dimgThumb.copyQImage();
            return true;
        }
    }

    // In 4th, try to extract embedded thumbnail using dcraw

    DcrawPreview::loadDcrawPreview(thumbnail, QString(folder + "/" + itemName));
    if (!thumbnail.isNull())
        return true;

    // Finaly, we trying to get thumbnail using DImg API.

    DImg dimgThumb(QFile::encodeName(folder + "/" + itemName));

    if (!dimgThumb.isNull())
    {
        thumbnail = dimgThumb.copyQImage();
        return true;
    }

    return false;
}

bool UMSCamera::getExif(const QString&, const QString&, char **,int&)
{
    // not necessary to implement this. read it directly from the file
    // (done in camera controller)
    kdWarning() << "exif implemented yet" << endl;
    return false;
}

bool UMSCamera::downloadItem(const QString& folder, const QString& itemName, const QString& saveFile)
{
    m_cancel = false;

    QString src  = folder + "/" + itemName;
    QString dest = saveFile;

    QFile sFile(src);
    QFile dFile(dest);

    if ( !sFile.open(IO_ReadOnly) )
    {
        kdWarning() << "Failed to open source file for reading: "
                    << src << endl;
        return false;
    }

    if ( !dFile.open(IO_WriteOnly) )
    {
        sFile.close();
        kdWarning() << "Failed to open dest file for writing: "
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

bool UMSCamera::deleteItem(const QString& folder, const QString& itemName)
{
    m_cancel = false;

    // Any camera provide THM (thumbnail) file with real image. We need to remove it also.

    QFileInfo fi(folder + "/" + itemName);

    QFileInfo thmLo(folder + "/" + fi.baseName() + ".thm");          // Lowercase

    if (thmLo.exists())
        ::unlink(QFile::encodeName(thmLo.filePath()));

    QFileInfo thmUp(folder + "/" + fi.baseName() + ".THM");          // Uppercase

    if (thmUp.exists())
        ::unlink(QFile::encodeName(thmUp.filePath()));

    // Remove the real image.
    return (::unlink(QFile::encodeName(folder + "/" + itemName)) == 0);
}

bool UMSCamera::uploadItem(const QString&, const QString&, const QString&)
{
    kdWarning() << "Upload not implemented yet" << endl;
    return false;
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

}  // namespace Digikam
