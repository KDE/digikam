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

#ifndef _DIGIKAM_IO_P_H_
#define _DIGIKAM_IO_P_H_

// Qt includes

#include <QObject>

namespace Digikam
{

class Album;
class DIO;

class DIO::Private : public QObject
{
    Q_OBJECT

public:

    explicit Private(DIO* const qq);

    void imagesToAlbum(int operation, const QList<ImageInfo>& ids, const PAlbum* const dest);
    void albumToAlbum(int operation, const PAlbum* const src, const PAlbum* const dest);
    void filesToAlbum(int operation, const QList<QUrl>& src, const PAlbum* const dest);

    void renameFile(const ImageInfo& info, const QString& newName);
    void deleteFiles(const QList<ImageInfo>& infos, bool useTrash);

    void processJob(int operation, const QList<QUrl>& src, const QUrl& dest);
    void processRename(const QUrl& src, const QUrl& dest);

Q_SIGNALS:

    void jobToCreate(int operation, const QList<QUrl>& src, const QUrl& dest);

public:

    DIO* const q;
};

// -----------------------------------------------------------------------------------------

class SidecarFinder
{
public:

    explicit SidecarFinder(const QList<QUrl>& files);
    explicit SidecarFinder(const QUrl& file);

    QList<QUrl> localFiles;
    QList<QUrl> remoteFiles;
    QList<QUrl> possibleRemoteSidecars;

    QList<QString> localFileSuffixes;
    QList<QString> remoteFileSuffixes;
    QList<QString> possibleRemoteSidecarSuffixes;

private:

    void process(const QList<QUrl>&);
};

// -----------------------------------------------------------------------------------------

class GroupedImagesFinder
{
public:

    explicit GroupedImagesFinder(const QList<ImageInfo>& source);

    QList<ImageInfo> infos;

private:

    void process(const QList<ImageInfo>& source);
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

} // namespace Digikam

#endif // _DIGIKAM_IO_H_
