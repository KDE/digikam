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

#ifndef IMAGEVIEWUTILITIES_H
#define IMAGEVIEWUTILITIES_H

// Qt includes

#include <QList>
#include <QWidget>
#include <QUrl>

// Local includes

#include "imageinfo.h"
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

    void createNewAlbumForInfos(const QList<ImageInfo>& infos, Album* currentAlbum);
    bool deleteImages(const QList<ImageInfo>& infos, const DeleteMode deleteMode);
    void deleteImagesDirectly(const QList<ImageInfo>& infos, const DeleteMode deleteMode);

    void insertToLightTableAuto(const QList<ImageInfo>& all, const QList<ImageInfo>& selected, const ImageInfo& current);
    void insertToLightTable(const QList<ImageInfo>& list, const ImageInfo& current, bool addTo);

    void insertToQueueManager(const QList<ImageInfo>& list, const ImageInfo& currentInfo, bool newQueue);
    void insertSilentToQueueManager(const QList<ImageInfo>& list, const ImageInfo& currentInfo, int queueid);

    void notifyFileContentChanged(const QList<QUrl>& urls);

    void openInfos(const ImageInfo& info, const QList<ImageInfo>& allInfosToOpen, Album* currentAlbum);
    void openInfosWithDefaultApplication(const QList<ImageInfo>& allInfosToOpen);

    void rename(const QUrl& imageUrl, const QString& newName);
    void setAsAlbumThumbnail(Album* album, const ImageInfo& imageInfo);

    void createGroupByTimeFromInfoList(const ImageInfoList& imageInfoList);
    void createGroupByFilenameFromInfoList(const ImageInfoList& imageInfoList);

Q_SIGNALS:

    void editorCurrentUrlChanged(const QUrl& url);
    void signalImagesDeleted(const QList<qlonglong>& imageIds);

protected:

    QWidget* m_widget;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ImageViewUtilities::DeleteMode)

#endif /* IMAGEVIEWUTILITIES_H */
