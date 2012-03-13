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

class DIO : public QObject
{
    Q_OBJECT

public:

    static void cleanUp();

    /// Copy an album to another album
    static void copy(const PAlbum* src, const PAlbum* dest);

    /// Copy items to another album
    static void copy(const QList<ImageInfo> infos, const PAlbum* dest);

    /// Copy an external file to another album
    static void copy(const KUrl& src, const PAlbum* dest);

    /// Copy external files to another album
    static void copy(const KUrl::List& srcList, const PAlbum* dest);

    /// Move an album into another album
    static void move(const PAlbum* src, const PAlbum* dest);

    /// Move items to another album
    static void move(const QList<ImageInfo> infos, const PAlbum* dest);

    /// Move external files another album
    static void move(const KUrl& src, const PAlbum* dest);

    /// Move external files into another album
    static void move(const KUrl::List& srcList, const PAlbum* dest);

    static void del(const QList<ImageInfo>& infos, bool useTrash);
    static void del(const ImageInfo& info, bool useTrash);
    static void del(PAlbum* album, bool useTrash);

    /// Rename item to new name
    static void rename(const ImageInfo& info, const QString& newName);

    static DIO* instance();

Q_SIGNALS:

    void imageRenameSucceeded(const KUrl&);
    void imageRenameFailed(const KUrl&);
    void renamingAborted(const KUrl&);

protected Q_SLOTS:

    void slotResult(KJob* kjob);
    void slotRenamed(KIO::Job*, const KUrl&, const KUrl& newURL);
    KIO::Job* createJob(int operation, const KUrl::List& src, const KUrl& dest);

private:

    DIO();
    ~DIO();

    friend class DIOCreator;
    class DIOPriv;
    DIOPriv* const d;
};

} // namespace Digikam

#endif /* DIO_H */
