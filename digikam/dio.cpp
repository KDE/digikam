/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-17
 * Description : low level files management interface.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#include "dio.h"

// KDE includes


#include <kio/deletejob.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "imageinfo.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "imagelister.h"
#include "scancontroller.h"

namespace Digikam
{

namespace DIO
{

KIO::Job* copy(const PAlbum* src, const PAlbum* dest)
{
    KUrl srcUrl(src->databaseUrl());
    KUrl destUrl(dest->databaseUrl());
    ScanController::instance()->hintAtMoveOrCopyOfAlbum(src, dest);

    return KIO::copy(srcUrl, destUrl);
}

KIO::Job* move(const PAlbum* src, const PAlbum* dest)
{
    KUrl srcUrl(src->databaseUrl());
    KUrl destUrl(dest->databaseUrl());
    ScanController::instance()->hintAtMoveOrCopyOfAlbum(src, dest);

    return KIO::move(srcUrl, destUrl);
}

KIO::Job* copy(const KUrl::List& srcList, const QList<qlonglong> ids, const PAlbum* dest)
{
    KUrl destUrl(dest->databaseUrl());

    QStringList filenames;
    foreach(const KUrl& url, srcList)
    {
        filenames << url.fileName();
    }
    ScanController::instance()->hintAtMoveOrCopyOfItems(ids, dest, filenames);

    return KIO::copy(srcList, destUrl);
}

KIO::Job* move(const KUrl::List& srcList, const QList<qlonglong> ids, const PAlbum* dest)
{
    KUrl destUrl(dest->databaseUrl());

    QStringList filenames;
    foreach(const KUrl& url, srcList)
    {
        filenames << url.fileName();
    }
    ScanController::instance()->hintAtMoveOrCopyOfItems(ids, dest, filenames);

    return KIO::move(srcList, destUrl);
}

KIO::Job* copy(const KUrl& src, const PAlbum* dest)
{
    KUrl destUrl(dest->databaseUrl());

    return KIO::copy(src, destUrl);
}

KIO::Job* copy(const KUrl::List& srcList, const PAlbum* dest)
{
    KUrl destUrl(dest->databaseUrl());

    return KIO::copy(srcList, destUrl);
}

KIO::Job* move(const KUrl& src, const PAlbum* dest)
{
    KUrl destUrl(dest->databaseUrl());

    kDebug() << "File to move : " << src;

    return KIO::move(src, destUrl);
}

KIO::Job* move(const KUrl::List& srcList, const PAlbum* dest)
{
    KUrl destUrl(dest->databaseUrl());

    kDebug() << "Files to move : " << srcList;

    return KIO::move(srcList, destUrl);
}

KIO::CopyJob* rename(const ImageInfo& info, const QString& newName)
{
    KUrl oldUrl = info.databaseUrl();
    KUrl newUrl = oldUrl;
    newUrl.setFileName(newName);

    PAlbum* album = AlbumManager::instance()->findPAlbum(info.albumId());

    if (album)
    {
        ScanController::instance()->hintAtMoveOrCopyOfItem(info.id(), album, newName);
    }

    return KIO::move(oldUrl, newUrl, KIO::HideProgressInfo);
}

KIO::Job* del(const KUrl& src, bool useTrash)
{
    KIO::Job* job = 0;

    kDebug() << "File to delete : " << src;

    if (useTrash)
    {
        job = KIO::trash( src );
    }
    else
    {
        job = KIO::del(src);
    }

    return job;
}

KIO::Job* del(const KUrl::List& srcList, bool useTrash)
{
    KIO::Job* job = 0;

    kDebug() << "Files to delete : " << srcList;

    if (useTrash)
    {
        job = KIO::trash( srcList);
    }
    else
    {
        job = KIO::del(srcList);
    }

    return job;
}

} // namespace DIO

} // namespace Digikam
