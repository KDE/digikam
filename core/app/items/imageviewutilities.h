/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-04
 * Description : Various operations on images
 *
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_ITEMS_IMAGE_VIEW_UTILITIES_H
#define DIGIKAM_ITEMS_IMAGE_VIEW_UTILITIES_H

// Qt includes

#include <QList>
#include <QWidget>
#include <QUrl>

// Local includes

#include "iteminfo.h"
#include "digikam_export.h"

namespace Digikam
{
class Album;

class DIGIKAM_EXPORT ImageViewUtilities : public QObject
{
    Q_OBJECT

public:

    enum DeleteMode
    {
        DeletePermanently = 1,
        DeleteUseTrash    = 2
    };

public:

    explicit ImageViewUtilities(QWidget* const parentWidget);

public Q_SLOTS:

    void createNewAlbumForInfos(const QList<ItemInfo>& infos, Album* currentAlbum);
    bool deleteImages(const QList<ItemInfo>& infos, const DeleteMode deleteMode);
    void deleteImagesDirectly(const QList<ItemInfo>& infos, const DeleteMode deleteMode);

    void insertToLightTableAuto(const QList<ItemInfo>& all, const QList<ItemInfo>& selected, const ItemInfo& current);
    void insertToLightTable(const QList<ItemInfo>& list, const ItemInfo& current, bool addTo);

    void insertToQueueManager(const QList<ItemInfo>& list, const ItemInfo& currentInfo, bool newQueue);
    void insertSilentToQueueManager(const QList<ItemInfo>& list, const ItemInfo& currentInfo, int queueid);

    void notifyFileContentChanged(const QList<QUrl>& urls);

    void openInfos(const ItemInfo& info, const QList<ItemInfo>& allInfosToOpen, Album* currentAlbum);
    void openInfosWithDefaultApplication(const QList<ItemInfo>& allInfosToOpen);

    void rename(const QUrl& imageUrl, const QString& newName, bool overwrite = false);
    void setAsAlbumThumbnail(Album* album, const ItemInfo& imageInfo);

    void createGroupByTimeFromInfoList(const ItemInfoList& imageInfoList);
    void createGroupByFilenameFromInfoList(const ItemInfoList& imageInfoList);

Q_SIGNALS:

    void editorCurrentUrlChanged(const QUrl& url);
    void signalImagesDeleted(const QList<qlonglong>& imageIds);

protected:

    QWidget* m_widget;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImageViewUtilities::DeleteMode)

#endif // DIGIKAM_ITEMS_IMAGE_VIEW_UTILITIES_H
