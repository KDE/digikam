/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-04
 * Description : Various operations on images
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageviewutilities.moc"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <kinputdialog.h>
#include <kio/jobuidelegate.h>
#include <kmimetype.h>
#include <krun.h>
#include <kservice.h>
#include <kmimetypetrader.h>
#include <kurl.h>
#include <kwindowsystem.h>
#include <kdebug.h>

// Local includes

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
#include "fileoperation.h"

namespace Digikam
{

ImageViewUtilities::ImageViewUtilities(QWidget* const parentWidget)
    : QObject(parentWidget)
{
    m_widget = parentWidget;
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

void ImageViewUtilities::rename(const KUrl& imageUrl, const QString& newName)
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

    KUrl::List urlList;

    foreach(const ImageInfo& info, infos)
    {
        urlList << info.fileUrl();
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
    DIO::del(infos, useTrash);

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

    const bool useTrash = (deleteMode == ImageViewUtilities::DeleteUseTrash);
    DIO::del(infos, useTrash);
}

void ImageViewUtilities::notifyFileContentChanged(const KUrl::List& urls)
{
    foreach(const KUrl& url, urls)
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

    connect(imview, SIGNAL(signalURLChanged(KUrl)),
            this, SIGNAL(editorCurrentUrlChanged(KUrl)));

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

    KUrl::List urls;

    foreach (const ImageInfo& inf, infos)
    {
        urls << inf.fileUrl();
    }

    FileOperation::openFilesWithDefaultApplication(urls, m_widget);
}

namespace
{

bool lessThanByTimeForImageInfo(const ImageInfo& a, const ImageInfo& b)
{
    return a.dateTime() < b.dateTime();
}

} // namespace

void ImageViewUtilities::createGroupByTimeFromInfoList(const ImageInfoList& imageInfoList)
{
    QList<ImageInfo> groupingList = imageInfoList;
    // sort by time
    qStableSort(groupingList.begin(), groupingList.end(), lessThanByTimeForImageInfo);

    QList<ImageInfo>::iterator it, it2;

    for (it = groupingList.begin(); it != groupingList.end(); )
    {
        const ImageInfo& leader = *it;
        QList<ImageInfo> group;
        QDateTime time          = it->dateTime();

        for (it2 = it + 1; it2 != groupingList.end(); ++it2)
        {
            if (abs(time.secsTo(it2->dateTime())) < 2)
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

} // namespace Digikam
