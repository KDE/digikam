/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-03-13
 * Description : low level files management interface.
 *
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "workerobject.h"
#include "dio.h"

namespace Digikam
{

class Album;

class DIO::Private : public WorkerObject
{
    Q_OBJECT

public:

    explicit Private(DIO* const q);

    void albumToAlbum(int operation, const PAlbum* const src, const PAlbum* const dest);
    void imagesToAlbum(int operation, const QList<ImageInfo> ids, const PAlbum* const dest);
    void filesToAlbum(int operation, const KUrl::List& src, const PAlbum* const dest);

    void renameFile(const ImageInfo& info, const QString& newName);

    void deleteFiles(const QList<ImageInfo>& infos, bool useTrash);

    bool directLocalFileMove(const QString& src, const QString& destPath);

public Q_SLOTS:

    void processJob(int operation, const KUrl::List& src, const KUrl& dest);
    void processRename(const KUrl& src, const KUrl& dest);

Q_SIGNALS:

    void jobToProcess(int operation, const KUrl::List& src, const KUrl& dest);
    void renameToProcess(const KUrl& src, const KUrl& dest);
    void jobToCreate(int operation, const KUrl::List& src, const KUrl& dest);
    void remoteFilesToStat(int operation, const KUrl::List& srcToStat, const KUrl& dest);

public:

    DIO* const q;
};

// -----------------------------------------------------------------------------------------

namespace // anonymous namespace
{

class SidecarFinder
{
public:

    explicit SidecarFinder(const KUrl::List& files);
    explicit SidecarFinder(const KUrl& file);

    KUrl::List localFiles;
    KUrl::List remoteFiles;
    KUrl::List possibleRemoteSidecars;

private:

    void process(const KUrl::List&);
};

// ------------------------

class GroupedImagesFinder
{
public:

    explicit GroupedImagesFinder(const QList<ImageInfo> source);

    QList<ImageInfo> infos;

private:

    void process(const QList<ImageInfo> source);
};

enum Operation
{
    Copy                = 1 << 0,
    Move                = 1 << 1,
    Rename              = 1 << 2,
    Trash               = 1 << 3,
    Delete              = 1 << 4,
    SourceStatusUnknown = 1 << 20,

    OperationMask       = 0xffff,
    FlagMask            = 0xffff0000
};

} // anonymous namespace

} // namespace Digikam

#endif /* DIO_H */
