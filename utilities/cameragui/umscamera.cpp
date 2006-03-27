/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2004-12-21
 * Description : USB Mass Storage Implementation of abstract
 * type DKCamera
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

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

// QT includes.

#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qdeepcopy.h>
#include <qwmatrix.h>

// KDE includes.

#include <ktempfile.h>
#include <kdebug.h>

// Exiv2 includes.

#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>
#include <exiv2/tags.hpp>

// Local includes.

#include "dimg.h"
#include "dcraw_parse.h"
#include "albumsettings.h"
#include "umscamera.h"

namespace Digikam
{

class UMSCameraPriv
{
public:

    enum ImageOrientation
    {
        ORIENTATION_NORMAL=1, 
        ORIENTATION_HFLIP=2, 
        ORIENTATION_ROT_180=3, 
        ORIENTATION_VFLIP=4, 
        ORIENTATION_ROT_90_HFLIP=5, 
        ORIENTATION_ROT_90=6, 
        ORIENTATION_ROT_90_VFLIP=7, 
        ORIENTATION_ROT_270=8
    };

    UMSCameraPriv()
    {
        cancel = false;
    }

    bool    cancel;
    
    QString imageFilter;
    QString movieFilter;
    QString audioFilter;
    QString rawFilter;
};

UMSCamera::UMSCamera(const QString& model,
                     const QString& port,
                     const QString& path)
    : DKCamera(model, port, path)
{
    d = new UMSCameraPriv;
    
    AlbumSettings* settings = AlbumSettings::instance();
    d->imageFilter = QDeepCopy<QString>(settings->getImageFileFilter());
    d->movieFilter = QDeepCopy<QString>(settings->getMovieFileFilter());
    d->audioFilter = QDeepCopy<QString>(settings->getAudioFileFilter());
    d->rawFilter   = QDeepCopy<QString>(settings->getRawFileFilter());

    d->imageFilter = d->imageFilter.lower();
    d->movieFilter = d->movieFilter.lower();
    d->audioFilter = d->audioFilter.lower();
    d->rawFilter   = d->rawFilter.lower();
}

UMSCamera::~UMSCamera()
{
    delete d;
}

bool UMSCamera::doConnect()
{
    return true;    
}

void UMSCamera::cancel()
{
    // set the cancel flag
    d->cancel = true;
}

void UMSCamera::getAllFolders(const QString& folder,
                              QStringList& subFolderList)
{
    d->cancel = false;
    subFolderList.clear();
    subFolderList.append(folder);
    listFolders(folder, subFolderList);
}

bool UMSCamera::getItemsInfoList(const QString& folder,
                                 GPItemInfoList& infoList)
{
    d->cancel = false;
    infoList.clear();

    QDir dir(folder);
    dir.setFilter(QDir::Files);

    const QFileInfoList *list = dir.entryInfoList();
    if (!list)
        return false;

    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    while ((fi = it.current()) != 0 && !d->cancel)
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
            info.width            = -1; // TODO
            info.height           = -1; // TODO
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
    d->cancel = false;

    // In 1st, we trying to get thumbnail from Exif data if we are JPEG file.

    try
    {    
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)
                                      (QFile::encodeName(folder + "/" + itemName)));
        image->readMetadata();
        Exiv2::ExifData &exifData = image->exifData();
        Exiv2::DataBuf const c1(exifData.copyThumbnail());
        thumbnail.loadFromData(c1.pData_, c1.size_);
        
        if (!thumbnail.isNull())
        {
            Exiv2::ExifKey key("Exif.Image.Orientation");
            Exiv2::ExifData::iterator it = exifData.findKey(key);
            if (it != exifData.end())
            {
                QWMatrix matrix;
                long orientation = it->toLong();
                kdDebug() << itemName << " ==> Orientation: " << orientation << endl;
                
                switch (orientation) 
                {
                    case UMSCameraPriv::ORIENTATION_HFLIP:
                        matrix.scale(-1, 1);
                        break;
                
                    case UMSCameraPriv::ORIENTATION_ROT_180:
                        matrix.rotate(180);
                        break;
                
                    case UMSCameraPriv::ORIENTATION_VFLIP:
                        matrix.scale(1, -1);
                        break;
                
                    case UMSCameraPriv::ORIENTATION_ROT_90_HFLIP:
                        matrix.scale(-1, 1);
                        matrix.rotate(90);
                        break;
                
                    case UMSCameraPriv::ORIENTATION_ROT_90:
                        matrix.rotate(90);
                        break;
                
                    case UMSCameraPriv::ORIENTATION_ROT_90_VFLIP:
                        matrix.scale(1, -1);
                        matrix.rotate(90);
                        break;
                
                    case UMSCameraPriv::ORIENTATION_ROT_270:
                        matrix.rotate(270);
                        break;
                        
                    default:
                        break;
                }
    
                if ( orientation != UMSCameraPriv::ORIENTATION_NORMAL )
                    thumbnail = thumbnail.xForm( matrix );
            }
                
            return true;
        }
    }
    catch( Exiv2::Error &e )
    {
        kdDebug() << "Cannot load thumbnail using Exiv2 (" 
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
    }        

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

    // In 3rd we trying to get thumbnail from RAW files using dcraw parse utility.

    KTempFile thumbFile(QString::null, "camerarawthumb");
    thumbFile.setAutoDelete(true);
    DcrawParse rawFileParser;
    int orientation = 0;
  
    if (thumbFile.status() == 0)
    {
        if (rawFileParser.getThumbnail(QFile::encodeName(folder + "/" + itemName),
                                       QFile::encodeName(thumbFile.name()),
                                       &orientation) == 0)
        {
            thumbnail.load(thumbFile.name());
            if (!thumbnail.isNull())
            {
                if(orientation)
                {
                    QWMatrix M;
                    QWMatrix flip = QWMatrix(-1, 0, 0, 1, 0, 0);

                    switch(orientation+1)
                    {  // notice intentional fallthroughs
                        case 2: M = flip; break;
                        case 4: M = flip;
                        case 3: M.rotate(180); break;
                        case 5: M = flip;
                        case 6: M.rotate(90); break;
                        case 7: M = flip;
                        case 8: M.rotate(270); break;
                        default: break; // should never happen
                    }

                    thumbnail = thumbnail.xForm(M);
                }
                
                return true;
            }
        }
    }

    // Finaly, we trying to get thumbnail using DImg API. This way can take a while.
    // TODO : in the future, we need to use a new DImg::getEmbeddedThumbnail() method instead !

    DImg dimgThumb(QFile::encodeName(folder + "/" + itemName));

    if (!dimgThumb.isNull())
    {
        thumbnail = dimgThumb.copyQImage();
        return true;
    }

    return false;
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
    d->cancel = false;

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
    while ((len = sFile.readBlock(buffer, MAX_IPC_SIZE)) != 0 && !d->cancel)
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
    d->cancel = false;

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
    if (d->cancel)
        return;

    QDir dir(folder);
    dir.setFilter(QDir::Dirs|QDir::Executable);

    const QFileInfoList *list = dir.entryInfoList();
    if (!list)
        return;

    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    while ((fi = it.current()) != 0 && !d->cancel)
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
    
    if (d->imageFilter.contains(ext))
    {
        return "image/" + ext;
    }
    else if (d->movieFilter.contains(ext))
    {
        return "video/" + ext;
    }
    else if (d->audioFilter.contains(ext))
    {
        return "audio/" + ext;
    }
    else if (d->rawFilter.contains(ext))
    {
        return "image/" + ext;
    }
    else
    {
        return QString();
    }
}

}  // namespace Digikam
