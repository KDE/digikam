/* ============================================================
 * File  : digikamio.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-10-13
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

#include <qstringlist.h>
#include <qmap.h>

#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kio/renamedlg.h>
#include <kurl.h>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
}

#include "albuminfo.h"
#include "albummanager.h"

#include "digikamio.h"

namespace DigikamIO
{

class AlbumFileOpJobPriv
{
public:
    
    KIO::CopyJob          *copyJob;
    Digikam::AlbumInfo    *srcAlbum;
    Digikam::AlbumInfo    *destAlbum;
    QStringList            fileList;
    bool                   move;
    QMap<QString,QString>  commentsMap;
    bool                   singleFile;
    QString                singleFileComment;
};
    
AlbumFileOpJob::AlbumFileOpJob(Digikam::AlbumInfo *srcAlbum,
                               Digikam::AlbumInfo *destAlbum,
                               const QStringList& fileList,
                               bool move )
{
    d = new AlbumFileOpJobPriv;
    
    d->srcAlbum   = srcAlbum;
    d->destAlbum  = destAlbum;
    d->fileList   = fileList;
    d->move       = move;
    d->singleFile = false;

    KURL destURL(destAlbum->getPath());
    KURL::List srcURLs;

    srcAlbum->openDB();
    for (QStringList::Iterator it = d->fileList.begin();
         it != d->fileList.end(); ++it) {
        QString file(*it);
        KURL url(srcAlbum->getPath() + QString("/") + file);
        srcURLs.append(url);
        QString comment(srcAlbum->getItemComments(*it));
        d->commentsMap[file] = comment;
    }
    srcAlbum->closeDB();

    if (move)
        d->copyJob = KIO::move(srcURLs, destURL, true);
    else
        d->copyJob = KIO::copy(srcURLs, destURL, true);
    
    connect(d->copyJob,
            SIGNAL(copyingDone(KIO::Job*, const KURL&,
                               const KURL&, bool, bool)),
            SLOT(slotCopyingDone(KIO::Job*, const KURL&,
                                 const KURL&, bool, bool)));
    connect(d->copyJob,
            SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
            
}

AlbumFileOpJob::AlbumFileOpJob(Digikam::AlbumInfo *album,
                               const QString& srcFile,
                               const QString& destFile,
                               bool move)
{
    d = new AlbumFileOpJobPriv;
    
    d->srcAlbum   = album;
    d->destAlbum  = 0;
    d->move       = move;
    d->singleFile = true;

    album->openDB();
    d->singleFileComment = album->getItemComments(srcFile);
    album->closeDB();

    KURL srcURL(album->getPath() + QString("/") + srcFile);
    KURL destURL(album->getPath() + QString("/") + destFile);
    
    if (move)
        d->copyJob = KIO::moveAs(srcURL, destURL, true);
    else
        d->copyJob = KIO::copy(srcURL, destURL, true);
    
    connect(d->copyJob,
            SIGNAL(copyingDone(KIO::Job*, const KURL&,
                               const KURL&, bool, bool)),
            SLOT(slotCopyingDone(KIO::Job*, const KURL&,
                                 const KURL&, bool, bool)));
    connect(d->copyJob,
            SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
}

AlbumFileOpJob::~AlbumFileOpJob()
{
    delete d;
}

void AlbumFileOpJob::slotCopyingDone(KIO::Job*, const KURL& from,
                                     const KURL& to, bool, bool)
{
    QString srcFile(from.filename());
    QString destFile(to.filename());

    if (d->singleFile) {
        d->srcAlbum->openDB();
        d->srcAlbum->setItemComments(destFile,
                                     d->singleFileComment);
    }
    else {
        QString comment(d->commentsMap[srcFile]);
        d->destAlbum->openDB();
        d->destAlbum->setItemComments(destFile, comment);
        d->destAlbum->closeDB();
    }
        
    if (d->move) {
        d->srcAlbum->openDB();
        d->srcAlbum->deleteItemComments(srcFile);
        d->srcAlbum->closeDB();
    }
}

void AlbumFileOpJob::slotResult(KIO::Job* job)
{
    if (job->error()) {
        job->showErrorDialog();
    }

    if (!d->singleFile) 
        Digikam::AlbumManager::instance()->refreshItemHandler();

    delete this;
}

// -------------------------------------------------------------------

void copy(Digikam::AlbumInfo* srcAlbum,
          Digikam::AlbumInfo* destAlbum,
          const QStringList& fileList)
{
    if (!srcAlbum || !destAlbum || fileList.isEmpty())
        return;

    if (srcAlbum == destAlbum) return;
    
    new AlbumFileOpJob(srcAlbum, destAlbum, fileList);
}

void move(Digikam::AlbumInfo* srcAlbum,
          Digikam::AlbumInfo* destAlbum,
          const QStringList& fileList)
{
    if (!srcAlbum || !destAlbum || fileList.isEmpty())
        return;

    if (srcAlbum == destAlbum) return;
    
    new AlbumFileOpJob(srcAlbum, destAlbum, fileList, true);
}


void copy(Digikam::AlbumInfo* srcAlbum,
          Digikam::AlbumInfo* destAlbum,
          const QString& file)
{
    if (!srcAlbum || !destAlbum)
        return;

    if (srcAlbum == destAlbum) return;

    QStringList fileList;
    fileList.append(file);
    new AlbumFileOpJob(srcAlbum, destAlbum, fileList);
}

void move(Digikam::AlbumInfo* srcAlbum,
          Digikam::AlbumInfo* destAlbum,
          const QString& file)
{
    if (!srcAlbum || !destAlbum)
        return;

    if (srcAlbum == destAlbum) return;
    
    QStringList fileList;
    fileList.append(file);
    new AlbumFileOpJob(srcAlbum, destAlbum, fileList, true);
}


void copy(Digikam::AlbumInfo* album,
          const QString& srcFile,
          const QString& destFile)
{
    if (!album) return;

    new AlbumFileOpJob(album, srcFile, destFile);
}

bool rename(Digikam::AlbumInfo* album,
            const QString& srcFile,
            const QString& destFile)
{
    if (!album) return false;

    QString srcPath = album->getPath() + QString("/")
                      + srcFile;
    QString destPath = album->getPath() + QString("/")
                      + destFile;

    bool overwrite = false;
    
    struct stat info;
    while (stat(destPath.latin1(), &info) == 0 ) {

        QString newDestPath;
        
        KIO::RenameDlg_Result result =
            open_RenameDlg("Rename File", srcPath, destPath,
                           KIO::M_OVERWRITE, newDestPath);

        if (result == KIO::R_CANCEL)
            return false;

        destPath = newDestPath; 
        
        if (result == KIO::R_OVERWRITE) {
            overwrite = true;
            break;
        }
    }

    album->openDB();
    QString comment = album->getItemComments(srcFile);
    album->closeDB();

    if (::rename(srcPath.latin1(), destPath.latin1()) == 0) {
        album->openDB();
        album->setItemComments(destFile, comment);
        album->closeDB();
        return true;
    }

    return false;
}

}
