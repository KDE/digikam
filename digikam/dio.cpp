/* ============================================================
 * File  : dio.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-17
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju
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
#include <stdio.h>
}

// QT includes.

#include <qfile.h>
#include <qcstring.h>
#include <qdatastream.h>

// KDE includes.

#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <kio/renamedlg.h>
#include <klocale.h>
#include <kmessagebox.h>

// Local includes.

#include "ddebug.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "albumlister.h"
#include "albumdb.h"
#include "album.h"
#include "dio.h"
#include "dio_p.h"

namespace DIO
{

KIO::Job* copy(const KURL& src, const KURL& dest)
{
    KIO::Job* job = KIO::copy(src, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* copy(const KURL::List& srcList, const KURL& dest)
{
    KIO::Job* job = KIO::copy(srcList, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* move(const KURL& src, const KURL& dest)
{
    KIO::Job* job = KIO::move(src, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* move(const KURL::List& srcList, const KURL& dest)
{
    KIO::Job* job = KIO::move(srcList, dest, true);
    new Watch(job);

    return job;
}

KIO::Job* del(const KURL& src, bool useTrash)
{
    KIO::Job* job = 0;
    
    if (useTrash)
    {
        KURL dest("trash:/");

        if (!KProtocolInfo::isKnownProtocol(dest))
        {
            dest = KGlobalSettings::trashPath();
        }
        
        job = KIO::move( src, dest );
    }   
    else
    {
        job = KIO::del(src);
    }
    
    new Watch(job);
    return job;
}

KIO::Job* del(const KURL::List& srcList, bool useTrash)
{
    KIO::Job* job = 0;

    if (useTrash)
    {
        KURL dest("trash:/");

        if (!KProtocolInfo::isKnownProtocol(dest))
        {
            dest = KGlobalSettings::trashPath();
        }
        
        job = KIO::move( srcList, dest );
    }   
    else
    {
        job = KIO::del(srcList);
    }
    
    new Watch(job);
    return job;
}

bool renameFile(const KURL& src, const KURL& dest)
{
    KURL srcdir;
    srcdir.setDirectory(src.directory());
    KURL dstdir;
    dstdir.setDirectory(src.directory());
    Digikam::PAlbum* srcAlbum = Digikam::AlbumManager::instance()->findPAlbum(srcdir);
    Digikam::PAlbum* dstAlbum = Digikam::AlbumManager::instance()->findPAlbum(dstdir);
    if (!srcAlbum || !dstAlbum)
    {
        DWarning() << "Source Album " << src.directory() << " not found" << endl;
        return false;
    }

    QString srcPath = Digikam::AlbumManager::instance()->getLibraryPath() + src.path();
    QString dstPath = Digikam::AlbumManager::instance()->getLibraryPath() + dest.path();
    QString newDstPath;

    bool overwrite = false;

    struct stat stbuf; 
    while (::stat(QFile::encodeName(dstPath), &stbuf) == 0)
    {
        KIO::RenameDlg_Result result =
            KIO::open_RenameDlg(i18n("Rename File"), srcPath, KURL(dstPath).fileName(),
                                KIO::RenameDlg_Mode(KIO::M_SINGLE |
                                                    KIO::M_OVERWRITE),
                                newDstPath);

        dstPath = newDstPath;

        switch (result)
        {
        case KIO::R_CANCEL:
        {
            return false;
        }
        case KIO::R_OVERWRITE:
        {
            overwrite = true;
            break;
        }
        default:
            break;
        }

        if (overwrite)
            break;
    }

    Digikam::AlbumDB* db = Digikam::AlbumManager::instance()->albumDB();
    if (::rename(QFile::encodeName(srcPath), QFile::encodeName(dstPath)) == 0)
    {
        db->moveItem(srcAlbum->id(), src.fileName(),
                     dstAlbum->id(), KURL(dstPath).fileName());
        return true;
    }

    KMessageBox::error(0, i18n("Failed to rename file\n%1")
                       .arg(src.fileName()), i18n("Rename Failed"));
    return false;    
}

KIO::Job* scan(const KURL& albumURL)
{
    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << Digikam::AlbumManager::instance()->getLibraryPath();
    ds << albumURL;
    ds << QString();
    ds << 0;
    ds << 1;

    KIO::Job* job = new KIO::TransferJob(albumURL,
                                         KIO::CMD_SPECIAL,
                                         ba, QByteArray(),
                                         false);
    new Watch(job);

    return job;
}

bool running()
{
    return (Watch::m_runCount != 0);
}

Watch::Watch(KIO::Job* job)
{
    m_runCount++;
    connect(job, SIGNAL(result(KIO::Job*)),
            SLOT(slotDone(KIO::Job*)));
}

void Watch::slotDone(KIO::Job*)
{
    Digikam::AlbumManager::instance()->refresh();
    Digikam::AlbumLister::instance()->refresh();
    m_runCount--;

    delete this;
}

uint Watch::m_runCount = 0;

}  // namespace DIO

#include "dio_p.moc"
