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
 * Copyright (C) 2018      by Maik Qualmann <metzpinguin at gmail dot com>
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
class IOJobData;
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
    static void copy(PAlbum* const src, PAlbum* const dest);

    /// Copy items to another album
    static void copy(const QList<ImageInfo>& infos, PAlbum* const dest);

    /// Copy an external file to another album
    static void copy(const QUrl& src, PAlbum* const dest);

    /// Copy external files to another album
    static void copy(const QList<QUrl>& srcList, PAlbum* const dest);

    /// Move an album into another album
    static void move(PAlbum* const src, PAlbum* const dest);

    /// Move items to another album
    static void move(const QList<ImageInfo>& infos, PAlbum* const dest);

    /// Move external files another album
    static void move(const QUrl& src, PAlbum* const dest);

    /// Move external files into another album
    static void move(const QList<QUrl>& srcList, PAlbum* const dest);

    static void del(const QList<ImageInfo>& infos, bool useTrash);
    static void del(const ImageInfo& info, bool useTrash);
    static void del(PAlbum* const album, bool useTrash);

    /// Rename item to new name
    static void rename(const ImageInfo& info, const QString& newName);

    static DIO* instance();

Q_SIGNALS:

    void signalRenameSucceeded(const QUrl&);
    void signalRenameFailed(const QUrl&);

private:

    DIO();
    ~DIO();

    void processJob(IOJobData* const data);
    void createJob(IOJobData* const data);

    ProgressItem* getProgressItem(int operation) const;
    QString       getItemString(int operation)   const;

private Q_SLOTS:

    void slotResult();
    void slotCancel(ProgressItem* item);
    void slotOneProccessed(int operation);

private:

    friend class DIOCreator;
};

// -----------------------------------------------------------------------------------------

class SidecarFinder
{

public:

    explicit SidecarFinder(const QList<QUrl>& files);

    QList<QUrl>    localFiles;
    QList<QString> localFileSuffixes;
};

// -----------------------------------------------------------------------------------------

class GroupedImagesFinder
{

public:

    explicit GroupedImagesFinder(const QList<ImageInfo>& source);

    QList<ImageInfo> infos;
};

} // namespace Digikam

#endif // _DIGIKAM_IO_H_
