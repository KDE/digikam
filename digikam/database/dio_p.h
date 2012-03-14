/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-03-13
 * Description : low level files management interface.
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIO_P_H
#define DIO_P_H

// KDE includes

// Local includes

#include "workerobject.h"
#include "dio.h"

namespace Digikam
{

class Album;

class DIO::DIOPriv : public WorkerObject
{
    Q_OBJECT

public:

    DIOPriv(DIO* q);

    void albumToAlbum(int operation, const PAlbum* src, const PAlbum* dest);
    void imagesToAlbum(int operation, const QList<ImageInfo> ids, const PAlbum* dest);
    void filesToAlbum(int operation, const KUrl::List& src, const PAlbum* dest);

    void renameFile(const ImageInfo& info, const QString& newName);

    void deleteFiles(const QList<ImageInfo>& infos, bool useTrash);

public Q_SLOTS:

    void processJob(int operation, const KUrl::List& src, const KUrl& dest);
    void processRename(const KUrl& src, const KUrl& dest);

Q_SIGNALS:

    void jobToProcess(int operation, const KUrl::List& src, const KUrl& dest);
    void renameToProcess(const KUrl& src, const KUrl& dest);
    void jobToCreate(int operation, const KUrl::List& src, const KUrl& dest);

public:

    DIO* q;
};

} // namespace Digikam

#endif /* DIO_H */
