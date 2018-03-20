/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-04
 * Description : Various operations on images
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "imageviewutilities.h"

// Qt includes

#include <QFileInfo>
#include <QUrl>

// KDE includes

#include <kwindowsystem.h>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "albumselectdialog.h"
#include "applicationsettings.h"
#include "deletedialog.h"
#include "dio.h"
#include "imageinfo.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "loadingcacheinterface.h"
#include "queuemgrwindow.h"
#include "thumbnailloadthread.h"
#include "fileactionmngr.h"
#include "dfileoperations.h"
#include "coredb.h"
#include "coredbaccess.h"

namespace Digikam
{

ImageViewUtilities::ImageViewUtilities(QWidget* const parentWidget)
    : QObject(parentWidget)
{
    m_widget = parentWidget;

    connect(this, SIGNAL(signalImagesDeleted(const QList<qlonglong>&)),
            AlbumManager::instance(), SLOT(slotImagesDeleted(const QList<qlonglong>&)));
}

void ImageViewUtilities::setAsAlbumThumbnail(Album* album, const ImageInfo& imageInfo)
{
    if (!album)
    {
        return;
    }

    if (album->type() == Album::PHYSICAL)
    {
        PAlbum* const palbum = static_cast<PAlbum*>(album);

        QString err;
        AlbumManager::instance()->updatePAlbumIcon(palbum, imageInfo.id(), err);
    }
    else if (album->type() == Album::TAG)
    {
        TAlbum* const talbum = static_cast<TAlbum*>(album);

        QString err;
        AlbumManager::instance()->updateTAlbumIcon(talbum, QString(), imageInfo.id(), err);
    }
}

void ImageViewUtilities::rename(const QUrl& imageUrl, const QString& newName)
{
    if (imageUrl.isEmpty() || !imageUrl.isLocalFile() || newName.isEmpty())
    {
        return;
    }

    ImageInfo info = ImageInfo::fromUrl(imageUrl);
    DIO::rename(info, newName);
}

bool ImageViewUtilities::deleteImages(const QList<ImageInfo>& infos, const DeleteMode deleteMode)
{
    if (infos.isEmpty())
    {
        return false;
    }

    QList<ImageInfo> deleteInfos = infos;

    QList<QUrl> urlList;
    QList<qlonglong> imageIds;

    // Buffer the urls for deletion and imageids for notification of the AlbumManager
    foreach(const ImageInfo& info, deleteInfos)
    {
        urlList << info.fileUrl();
        imageIds << info.id();
    }

    DeleteDialog dialog(m_widget);

    DeleteDialogMode::DeleteMode deleteDialogMode = DeleteDialogMode::NoChoiceTrash;

    if (deleteMode == ImageViewUtilities::DeletePermanently)
    {
        deleteDialogMode = DeleteDialogMode::NoChoiceDeletePermanently;
    }

    if (!dialog.confirmDeleteList(urlList, DeleteDialogMode::Files, deleteDialogMode))
    {
        return false;
    }

    const bool useTrash = !dialog.shouldDelete();
    if (!useTrash)
    {
        // If the images should be deleted permanently, mark the images as obsolete and remove them
        // from their album
        CoreDbAccess access;
        foreach(const ImageInfo& info, deleteInfos)
        {
            access.db()->removeItemsPermanently(QList<qlonglong>() << info.id(), QList<int>() << info.albumId());
        }
    }

    DIO::del(deleteInfos, useTrash);

    // Signal the Albummanager about the ids of the deleted images.
    emit signalImagesDeleted(imageIds);

    return true;
}

void ImageViewUtilities::deleteImagesDirectly(const QList<ImageInfo>& infos, const DeleteMode deleteMode)
{
    // This method deletes the selected items directly, without confirmation.
    // It is not used in the default setup.

    if (infos.isEmpty())
    {
        return;
    }

    QList<qlonglong> imageIds;

    foreach(const ImageInfo& info, infos)
    {
        imageIds << info.id();
    }

    const bool useTrash = (deleteMode == ImageViewUtilities::DeleteUseTrash);
    if (!useTrash)
    {
        // If the images should be deleted permanently, mark the images as obsolete and remove them
        // from their album
        CoreDbAccess access;
        foreach(const ImageInfo& info, infos)
        {
            access.db()->removeItemsPermanently(QList<qlonglong>() << info.id(), QList<int>() << info.albumId());
        }
    }
    DIO::del(infos, useTrash);

    // Signal the Albummanager about the ids of the deleted images.
    emit signalImagesDeleted(imageIds);
}

void ImageViewUtilities::notifyFileContentChanged(const QList<QUrl>& urls)
{
    foreach(const QUrl& url, urls)
    {
        QString path = url.toLocalFile();
        ThumbnailLoadThread::deleteThumbnail(path);
        // clean LoadingCache as well - be pragmatic, do it here.
        LoadingCacheInterface::fileChanged(path);
    }
}

void ImageViewUtilities::createNewAlbumForInfos(const QList<ImageInfo>& infos, Album* currentAlbum)
{
    if (infos.isEmpty())
    {
        return;
    }

    if (currentAlbum && currentAlbum->type() != Album::PHYSICAL)
    {
        currentAlbum = 0;
    }

    QString header(i18n("<p>Please select the destination album from the digiKam library to "
                        "move the selected images into.</p>"));

    Album* const album = AlbumSelectDialog::selectAlbum(m_widget, static_cast<PAlbum*>(currentAlbum), header);

    if (!album)
    {
        return;
    }

    DIO::move(infos, (PAlbum*)album);
}

void ImageViewUtilities::insertToLightTableAuto(const QList<ImageInfo>& all, const QList<ImageInfo>& selected, const ImageInfo& current)
{
    ImageInfoList list   = selected;
    ImageInfo singleInfo = current;

    if (list.isEmpty() || (list.size() == 1 && LightTableWindow::lightTableWindow()->isEmpty()))
    {
        list = all;
    }

    if (singleInfo.isNull() && !list.isEmpty())
    {
        singleInfo = list.first();
    }

    insertToLightTable(list, current, list.size() <= 1);
}

void ImageViewUtilities::insertToLightTable(const QList<ImageInfo>& list, const ImageInfo& current, bool addTo)
{
    LightTableWindow* const ltview = LightTableWindow::lightTableWindow();

    // If addTo is false, the light table will be emptied before adding
    // the images.
    ltview->loadImageInfos(list, current, addTo);
    ltview->setLeftRightItems(list, addTo);

    if (ltview->isHidden())
    {
        ltview->show();
    }

    if (ltview->isMinimized())
    {
        KWindowSystem::unminimizeWindow(ltview->winId());
    }

    KWindowSystem::activateWindow(ltview->winId());
}

void ImageViewUtilities::insertToQueueManager(const QList<ImageInfo>& list, const ImageInfo& current, bool newQueue)
{
    Q_UNUSED(current);



    QueueMgrWindow* const bqmview = QueueMgrWindow::queueManagerWindow();

    if (bqmview->isHidden())
    {
        bqmview->show();
    }

    if (bqmview->isMinimized())
    {
        KWindowSystem::unminimizeWindow(bqmview->winId());
    }

    KWindowSystem::activateWindow(bqmview->winId());

    if (newQueue)
    {
        bqmview->loadImageInfosToNewQueue(list);
    }
    else
    {
        bqmview->loadImageInfosToCurrentQueue(list);
    }
}

void ImageViewUtilities::insertSilentToQueueManager(const QList<ImageInfo>& list, const ImageInfo& /*current*/, int queueid)
{
    QueueMgrWindow* const bqmview = QueueMgrWindow::queueManagerWindow();
    bqmview->loadImageInfos(list, queueid);
}

void ImageViewUtilities::openInfos(const ImageInfo& info, const QList<ImageInfo>& allInfosToOpen, Album* currentAlbum)
{
    if (info.isNull())
    {
        return;
    }

    QFileInfo fi(info.filePath());
    QString imagefilter = ApplicationSettings::instance()->getImageFileFilter();
    imagefilter        += ApplicationSettings::instance()->getRawFileFilter();

    // If the current item is not an image file.
    if ( !imagefilter.contains(fi.suffix().toLower()) )
    {
        // Openonly the first one from the list.
        openInfosWithDefaultApplication(QList<ImageInfo>() << info);
        return;
    }

    // Run digiKam ImageEditor with all image from current Album.

    ImageWindow* const imview = ImageWindow::imageWindow();

    imview->disconnect(this);

    connect(imview, SIGNAL(signalURLChanged(QUrl)),
            this, SIGNAL(editorCurrentUrlChanged(QUrl)));

    imview->loadImageInfos(allInfosToOpen, info,
                           currentAlbum ? i18n("Album \"%1\"", currentAlbum->title()) : QString());

    if (imview->isHidden())
    {
        imview->show();
    }

    if (imview->isMinimized())
    {
        KWindowSystem::unminimizeWindow(imview->winId());
    }

    KWindowSystem::activateWindow(imview->winId());
}

void ImageViewUtilities::openInfosWithDefaultApplication(const QList<ImageInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    QList<QUrl> urls;

    foreach (const ImageInfo& inf, infos)
    {
        urls << inf.fileUrl();
    }

    DFileOperations::openFilesWithDefaultApplication(urls);
}

namespace
{

bool lessThanByTimeForImageInfo(const ImageInfo& a, const ImageInfo& b)
{
    return a.dateTime() < b.dateTime();
}

bool lowerThanByNameForImageInfo(const ImageInfo& a, const ImageInfo& b)
{
    return a.name() < b.name();
}

bool lowerThanBySizeForImageInfo(const ImageInfo& a, const ImageInfo& b)
{
    return a.fileSize() < b.fileSize();
}

} // namespace

void ImageViewUtilities::createGroupByTimeFromInfoList(const ImageInfoList& imageInfoList)
{
    QList<ImageInfo> groupingList = imageInfoList;
    // sort by time
    std::stable_sort(groupingList.begin(), groupingList.end(), lessThanByTimeForImageInfo);

    QList<ImageInfo>::iterator it, it2;

    for (it = groupingList.begin(); it != groupingList.end(); )
    {
        const ImageInfo& leader = *it;
        QList<ImageInfo> group;
        QDateTime time          = it->dateTime();

        for (it2 = it + 1; it2 != groupingList.end(); ++it2)
        {
            if (qAbs(time.secsTo(it2->dateTime())) < 2)
            {
                group << *it2;
            }
            else
            {
                break;
            }
        }

        // increment to next item not put in the group
        it = it2;

        if (!group.isEmpty())
        {
            FileActionMngr::instance()->addToGroup(leader, group);
        }
    }
}

void ImageViewUtilities::createGroupByFilenameFromInfoList(const ImageInfoList& imageInfoList)
{
    QList<ImageInfo> groupingList = imageInfoList;
    // sort by Name
    std::stable_sort(groupingList.begin(), groupingList.end(), lowerThanByNameForImageInfo);

    QList<ImageInfo>::iterator it, it2;

    for (it = groupingList.begin(); it != groupingList.end(); )
    {
        QList<ImageInfo> group;
        QString fname = it->name().left(it->name().lastIndexOf(QLatin1Char('.')));
        // don't know the leader yet so put first element also in group
        group << *it;

        for (it2 = it + 1; it2 != groupingList.end(); ++it2)
        {
            QString fname2 = it2->name().left(it2->name().lastIndexOf(QLatin1Char('.')));

            if (fname == fname2)
            {
                group << *it2;
            }
            else
            {
                break;
            }
        }

        // increment to next item not put in the group
        it = it2;

        if (group.count() > 1)
        {
            // sort by filesize and take smallest as leader
            std::stable_sort(group.begin(), group.end(), lowerThanBySizeForImageInfo);
            const ImageInfo& leader = group.takeFirst();
            FileActionMngr::instance()->addToGroup(leader, group);
        }
    }
}

} // namespace Digikam
