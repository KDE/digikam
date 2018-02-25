/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-17
 * Description : low level files management interface.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef _DIGIKAM_IO_H_
#define _DIGIKAM_IO_H_

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"

class QUrl;

namespace Digikam
{

class PAlbum;
class ImageInfo;
class ProgressItem;

class DIGIKAM_EXPORT DIO : public QObject
{
    Q_OBJECT

public:

    static void cleanUp();

    /**
     * All DIO methods will take care for sidecar files, if they exist
     */

    /// Copy an album to another album
    static void copy(const PAlbum* const src, const PAlbum* const dest);

    /// Copy items to another album
    static void copy(const QList<ImageInfo>& infos, const PAlbum* const dest);

    /// Copy an external file to another album
    static void copy(const QUrl& src, const PAlbum* const dest);

    /// Copy external files to another album
    static void copy(const QList<QUrl>& srcList, const PAlbum* const dest);

    /// Move an album into another album
    static void move(const PAlbum* const src, const PAlbum* const dest);

    /// Move items to another album
    static void move(const QList<ImageInfo>& infos, const PAlbum* const dest);

    /// Move external files another album
    static void move(const QUrl& src, const PAlbum* const dest);

    /// Move external files into another album
    static void move(const QList<QUrl>& srcList, const PAlbum* const dest);

    static void del(const QList<ImageInfo>& infos, bool useTrash);
    static void del(const ImageInfo& info, bool useTrash);
    static void del(const PAlbum* const album, bool useTrash);

    /// Rename item to new name
    static void rename(const ImageInfo& info, const QString& newName);

    static DIO* instance();

Q_SIGNALS:

    void signalRenameSucceeded(const QUrl&);
    void signalRenameFailed(const QUrl&);

private:

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

    DIO();
    ~DIO();

    void imagesToAlbum(int operation, const QList<ImageInfo>& ids, const PAlbum* const dest);
    void albumToAlbum(int operation, const PAlbum* const src, const PAlbum* const dest);
    void filesToAlbum(int operation, const QList<QUrl>& src, const PAlbum* const dest);

    void renameFile(const ImageInfo& info, const QString& newName);
    void deleteFiles(const QList<ImageInfo>& infos, bool useTrash);

    void processJob(int operation, const QList<QUrl>& src, const QUrl& dest);
    void processRename(const QUrl& src, const QUrl& dest);
    void createJob(int operation, const QList<QUrl>& src, const QUrl& dest);

    ProgressItem* getProgressItem(int operation);

private Q_SLOTS:

    void slotResult();
    void slotCancel(ProgressItem* item);
    void slotOneProccessed(int operation);
    void slotRenamed(const QUrl& oldUrl, const QUrl& newUrl);

private:

    friend class DIOCreator;
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

} // namespace Digikam

#endif // _DIGIKAM_IO_H_
