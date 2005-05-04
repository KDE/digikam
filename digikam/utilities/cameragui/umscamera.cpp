/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-12-21
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <kdebug.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qdeepcopy.h>

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
}

#include <libkexif/kexifdata.h>

#include "albumsettings.h"

#include "umscamera.h"

UMSCamera::UMSCamera(const QString& model,
                     const QString& port,
                     const QString& path)
    : DKCamera(model, port, path)
{
    m_cancel = false;    

    AlbumSettings* settings = AlbumSettings::instance();
    m_imageFilter = QDeepCopy<QString>(settings->getImageFileFilter());
    m_movieFilter = QDeepCopy<QString>(settings->getMovieFileFilter());
    m_audioFilter = QDeepCopy<QString>(settings->getAudioFileFilter());
    m_rawFilter   = QDeepCopy<QString>(settings->getRawFileFilter());

    m_imageFilter = m_imageFilter.lower(); 
    m_movieFilter = m_movieFilter.lower();     
    m_audioFilter = m_audioFilter.lower();     
    m_rawFilter   = m_rawFilter.lower();     
}

UMSCamera::~UMSCamera()
{
}

bool UMSCamera::connect()
{
    return true;    
}

void UMSCamera::cancel()
{
    // set the cancel flag
    m_cancel = true;
}

void UMSCamera::getAllFolders(const QString& folder,
                              QStringList& subFolderList)
{
    m_cancel = false;
    subFolderList.clear();
    listFolders(folder, subFolderList);
}

bool UMSCamera::getItemsInfoList(const QString& folder,
                                 GPItemInfoList& infoList)
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
            info.name             = fi->fileName();
            info.folder           = folder;
            info.mime             = mime;
            info.mtime            = fi->lastModified().toTime_t();
            info.size             = fi->size();
            info.width            = -1; // todo
            info.height           = -1; // todo
            info.downloaded       = -1; 
            info.readPermissions  = fi->isReadable();
            info.writePermissions = fi->isWritable();
        
            infoList.append(info);
        }
    }
    
    return true;
}

bool UMSCamera::getThumbnail(const QString& folder,
                             const QString& itemName,
                             QImage& thumbnail)
{
    m_cancel = false;
    KExifData exifData;
    if (!exifData.readFromFile(folder + "/" + itemName))
    {
        return false;
    }

    thumbnail = exifData.getThumbnail();

    // TODO: check for thm files if we didn't manage to get thumbnail from exif
    
    return true;
}

bool UMSCamera::getExif(const QString& ,
                        const QString& ,
                        char **, int& )
{
    // not necessary to implement this. read it directly from the file
    // (done in camera controller)
    kdWarning() << "exif implemented yet" << endl;
    return false;
}

bool UMSCamera::downloadItem(const QString& folder,
                             const QString& itemName,
                             const QString& saveFile)
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
                    << src << endl;
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

bool UMSCamera::deleteItem(const QString& folder,
                           const QString& itemName)
{
    m_cancel = false;

    return (::unlink(QFile::encodeName(folder + "/" + itemName)) == 0);
}

bool UMSCamera::uploadItem(const QString& ,
                           const QString& ,
                           const QString& )
{
    kdWarning() << "Upload not implemented yet" << endl;
    return false;
}

void UMSCamera::listFolders(const QString& folder,
                            QStringList& subFolderList)
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

        QString subfolder = folder + QString(folder.endsWith("/") ? "" : "/")
                            + fi->fileName();
        subFolderList.append(subfolder);
        listFolders(subfolder, subFolderList);        
    }
}

QString UMSCamera::mimeType(const QString& fileext) const
{
    QString ext = fileext;
    
    // massage known variations of known mimetypes into kde specific ones
    if (ext == "jpg")
        ext = "jpeg";
    else if (ext == "tif")
        ext = "tiff";
    
    if (m_imageFilter.contains(ext))
    {
        return "image/" + ext;
    }
    else if (m_movieFilter.contains(ext))
    {
        return "video/" + ext;
    }
    else if (m_audioFilter.contains(ext))
    {
        return "audio/" + ext;
    }
    else if (m_rawFilter.contains(ext))
    {
        return "image/" + ext;
    }
    else
    {
        return QString();
    }
}
