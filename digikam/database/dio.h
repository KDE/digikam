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

#ifndef DIO_H
#define DIO_H

// KDE includes

#include <kio/job.h>
#include <kio/copyjob.h>

namespace Digikam
{

class PAlbum;
class ImageInfo;

namespace DIO
{
/// Copy an album to another album
KIO::Job* copy(const PAlbum* src, const PAlbum* dest);

/// Copy items to another album
KIO::Job* copy(const KUrl::List& srcList, const QList<qlonglong> ids, const PAlbum* dest);

/// Copy an external file to another album
KIO::Job* copy(const KUrl& src, const PAlbum* dest);

/// Copy external files to another album
KIO::Job* copy(const KUrl::List& srcList, const PAlbum* dest);

/// Move an album into another album
KIO::Job* move(const PAlbum* src, const PAlbum* dest);

/// Move items to another album
KIO::Job* move(const KUrl::List& srcList, const QList<qlonglong> ids, const PAlbum* dest);

/// Move external files another album
KIO::Job* move(const KUrl& src, const PAlbum* dest);

/// Move external files into another album
KIO::Job* move(const KUrl::List& srcList, const PAlbum* dest);

KIO::Job* del(const KUrl& src, bool useTrash = true);

KIO::Job* del(const KUrl::List& srcList, bool useTrash = true);

/// Rename item to new name
KIO::CopyJob* rename(const ImageInfo& info, const QString& newName);
}

} // namespace Digikam

#endif /* DIO_H */
