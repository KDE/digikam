/* ============================================================
 * File  : digikamio.h
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

#ifndef DIGIKAMIO_H
#define DIGIKAMIO_H

#include <qobject.h>

class QStringList;
class KURL;

namespace Digikam
{
class AlbumInfo;
}

namespace KIO
{
class Job;
}

namespace DigikamIO
{

class AlbumFileOpJobPriv;

class AlbumFileOpJob : public QObject
{
    Q_OBJECT

public:

    AlbumFileOpJob(Digikam::AlbumInfo *srcAlbum,
                   Digikam::AlbumInfo *destAlbum,
                   const QStringList& fileList,
                   bool move = false);
    AlbumFileOpJob(Digikam::AlbumInfo *album,
                   const QString& srcFile,
                   const QString& destFile,
                   bool move = false);
    ~AlbumFileOpJob();
    
private slots:

    void slotCopyingDone(KIO::Job*, const KURL& from, const KURL& to,
                         bool directory, bool renamed);
    void slotResult(KIO::Job* job);

private:

    AlbumFileOpJobPriv *d;
};

/*! 
 * Copy a list of files from one album to another
 */
void copy(Digikam::AlbumInfo* srcAlbum, Digikam::AlbumInfo* destAlbum,
          const QStringList& fileList);

/*! 
 * Move a list of files from one album to another
 */
void move(Digikam::AlbumInfo* srcAlbum, Digikam::AlbumInfo* destAlbum,
          const QStringList& fileList);

/*! 
 * Copy a single file from one album to another
 */
void copy(Digikam::AlbumInfo* srcAlbum, Digikam::AlbumInfo* destAlbum,
           const QString& file);

/*! 
 * Move a single file from one album to another
 */
void move(Digikam::AlbumInfo* srcAlbum, Digikam::AlbumInfo* destAlbum,
          const QString& file);

/*! 
 * Copy a single file to another within an album
 */
void copy(Digikam::AlbumInfo* album, const QString& srcFile,
          const QString& destFile);

/*! 
 * Rename a single file within an album.
 * Synchronous operation. returns true if renamed, else false.
 */
bool rename(Digikam::AlbumInfo* album, const QString& srcFile,
            const QString& destFile);

}

#endif /* DIGIKAMIO_H */
