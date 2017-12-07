/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-10-04
 * Description : synchronize Input/Output jobs.
 *
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SYNC_JOB_H
#define SYNC_JOB_H

// Qt includes

#include <QObject>
#include <QPixmap>

class QString;

namespace Digikam
{

class Album;
class TAlbum;

// -------------------------------------------------------------------------------

class SyncJob : public QObject
{
    Q_OBJECT

public:

    /** Load the image or icon for the tag thumbnail.
     */
    static QPixmap getTagThumbnail(TAlbum* const album);
    static QPixmap getTagThumbnail(const QString& name, int size);

private:

    SyncJob();
    ~SyncJob();

    void enterWaitingLoop() const;
    void quitWaitingLoop()  const;

    QPixmap getTagThumbnailPriv(TAlbum* const album) const;

private Q_SLOTS:

    void slotGotThumbnailFromIcon(Album* album, const QPixmap& pix);
    void slotLoadThumbnailFailed(Album* album);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SYNC_JOB_H
