/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-04
 * Description : synchronize Input/Output jobs.
 *
 * Copyright (C)      2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Concept copied from kdelibs/kio/kio/netaccess.h/cpp
 *   This file is part of the KDE libraries
 *   Copyright (C) 1997 Torben Weis (weis@kde.org)
 *   Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
 *   Copyright (C) 1999 David Faure (faure@kde.org)
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

#ifndef SYNCJOB_H
#define SYNCJOB_H

// Qt includes

#include <QObject>
#include <QPixmap>

// KDE includes

#include <kurl.h>

class QString;

class KJob;

namespace KIO
{
    class Job;
}

namespace Digikam
{

class Album;
class TAlbum;

class SyncJobResult
{
public:

    bool    success;
    QString errorString;

    operator bool() const
    {
        return success;
    }
};

// -------------------------------------------------------------------------------

class SyncJob : public QObject
{
    Q_OBJECT

public:

    /* this will delete the urls. */
    static SyncJobResult del(const KUrl::List& urls, bool useTrash);

    /* Load the image or icon for the tag thumbnail */
    static QPixmap getTagThumbnail(TAlbum* const album);
    static QPixmap getTagThumbnail(const QString& name, int size);

private:

    SyncJob();
    ~SyncJob();

    void enterWaitingLoop();
    void quitWaitingLoop();

    bool delPriv(const KUrl::List& urls);
    bool trashPriv(const KUrl::List& urls);

    QPixmap getTagThumbnailPriv(TAlbum* const album);

private Q_SLOTS:

    void slotResult(KJob* job);
    void slotGotThumbnailFromIcon(Album* album, const QPixmap& pix);
    void slotLoadThumbnailFailed(Album* album);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* SYNCJOB_H */
