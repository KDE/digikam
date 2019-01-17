/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : page visualizing photos user choosing to upload and
 *               user albums list to upload photos to. Creating new album 
 *               is also available on this page.
 *
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_WS_IMAGES_PAGE_H
#define DIGIKAM_WS_IMAGES_PAGE_H

// Qt includes

#include <QObject>
#include <QList>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Local includes

#include "dwizardpage.h"
#include "wsitem.h"

namespace Digikam
{

class WSImagesPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit WSImagesPage(QWizard* const dialog, const QString& title);
    ~WSImagesPage();

    void    initializePage();
    bool    validatePage();
    bool    isComplete() const;

    void    setItemsList(const QList<QUrl>& urls);

private:

    /*
     * Get a structure from albums list and add it recursively to albums view
     */
    void addChildToTreeView(QTreeWidgetItem* const parent,
                            const QMap<QString, AlbumSimplified>& albumTree,
                            const QStringList& childrenAlbums);

    /*
     * Set id for album chosen to upload photos.
     *
     * This method should be called in validatePage(), so that talker can get it
     * from d->wizard later.
     */
    void setCurrentAlbumId(const QString& currentAlbumId);

Q_SIGNALS:

    /*
     * Signal to inform talker to list albums.
     */
    void signalListAlbumsRequest();

private Q_SLOTS:

    /*
     * Connected to signal signalListAlbumsDone of WSAuthentication to visualize albums list
     */
    void slotListAlbumsDone(const QMap<QString, AlbumSimplified>& albumTree,
                            const QStringList& rootAlbums,
                            const QString& currentAlbumId);

    /*
     * Connected to signalCreatAlbumDone of WSAuthentication to refresh album list and point
     * pre-selected album to new album
     */
    void slotCreateAlbumDone(int errCode, const QString& errMsg, const QString& newAlbumId);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_WS_IMAGES_PAGE_H
