/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-04
 * Description : Various operations on images
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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
#include <kservicetypetrader.h>
#include <kurl.h>
#include <kwindowsystem.h>
#include <kdebug.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "albumselectdialog.h"
#include "albumsettings.h"
#include "deletedialog.h"
#include "dio.h"
#include "imageinfo.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "loadingcacheinterface.h"
#include "queuemgrwindow.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

const QString renameFileProperty("AdvancedRename source file");

ImageViewUtilities::ImageViewUtilities(QWidget* parentWidget)
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
        PAlbum* palbum = static_cast<PAlbum*>(album);

        QString err;
        AlbumManager::instance()->updatePAlbumIcon(palbum, imageInfo.id(), err);
    }
    else if (album->type() == Album::TAG)
    {
        TAlbum* talbum = static_cast<TAlbum*>(album);

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

    ImageInfo info(imageUrl.toLocalFile());

    KIO::CopyJob* job = DIO::rename(info, newName);
    job->setProperty(renameFileProperty.toAscii(), imageUrl.toLocalFile());

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));

    connect(job, SIGNAL(copyingDone(KIO::Job*,KUrl,KUrl,time_t,bool,bool)),
            this, SLOT(slotRenamed(KIO::Job*,KUrl,KUrl)));
}

void ImageViewUtilities::slotRenamed(KIO::Job* job, const KUrl&, const KUrl& newURL)
{
    // reconstruct file path from digikamalbums:// URL
    KUrl fileURL;
    fileURL.setPath(newURL.user());
    fileURL.addPath(newURL.path());

    // refresh thumbnail
    ThumbnailLoadThread::deleteThumbnail(fileURL.toLocalFile());
    // clean LoadingCache as well - be pragmatic, do it here.
    LoadingCacheInterface::fileChanged(fileURL.toLocalFile());

    KUrl url(job->property(renameFileProperty.toAscii()).toString());
    emit imageRenameSucceeded(url);
}

bool ImageViewUtilities::deleteImages(const QList<ImageInfo>& infos, bool deletePermanently)
{
    KUrl::List urlList;
    KUrl::List kioUrlList;

    foreach (const ImageInfo& info, infos)
    {
        urlList << info.fileUrl();
        kioUrlList << info.databaseUrl();
    }

    if (urlList.count() <= 0)
    {
        return false;
    }

    DeleteDialog dialog(m_widget);

    if (!dialog.confirmDeleteList(urlList,
                                  DeleteDialogMode::Files,
                                  deletePermanently ?
                                  DeleteDialogMode::NoChoiceDeletePermanently :
                                  DeleteDialogMode::NoChoiceTrash))
    {
        return false;
    }

    bool useTrash = !dialog.shouldDelete();

    // trash does not like non-local URLs, put is not implemented

#ifdef WIN32
    KIO::Job* job = DIO::del(kioUrlList, useTrash);
#else
    KIO::Job* job = DIO::del(useTrash ? urlList : kioUrlList, useTrash);
#endif

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));

    return true;
}

void ImageViewUtilities::deleteImagesDirectly(const QList<ImageInfo>& infos, bool useTrash)
{
    // This method deletes the selected items directly, without confirmation.
    // It is not used in the default setup.

    KUrl::List kioUrlList;
    KUrl::List urlList;

    foreach (const ImageInfo& info, infos)
    {
        urlList << info.fileUrl();
        kioUrlList << info.databaseUrl();
    }

    if (kioUrlList.count() <= 0)
    {
        return;
    }

    // trash does not like non-local URLs, put is not implemented

#ifdef WIN32
    KIO::Job* job = DIO::del(kioUrlList , useTrash);
#else
    KIO::Job* job = DIO::del(useTrash ? urlList : kioUrlList , useTrash);
#endif

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));
}

void ImageViewUtilities::slotDIOResult(KJob* kjob)
{
    KIO::Job* job = static_cast<KIO::Job*>(kjob);

    if (job->error())
    {
        // this slot can be used by others, too.
        // check if image renaming property is set.
        QVariant v = job->property(renameFileProperty.toAscii());

        if (!v.isNull())
        {
            if (job->error() == KIO::Job::KilledJobError)
            {
                emit renamingAborted();
            }
            else
            {
                KUrl url(v.toString());
                emit imageRenameFailed(url);
            }
        }

        job->ui()->setWindow(m_widget);
        job->ui()->showErrorMessage();
    }
}

void ImageViewUtilities::notifyFileContentChanged(const KUrl::List& urls)
{
    foreach (const KUrl& url, urls)
    {
        QString path = url.toLocalFile();
        ThumbnailLoadThread::deleteThumbnail(path);
        // clean LoadingCache as well - be pragmatic, do it here.
        LoadingCacheInterface::fileChanged(path);
    }
}

void ImageViewUtilities::createNewAlbumForInfos(const QList<ImageInfo>& infos, Album* currentAlbum)
{
    KUrl::List       kioURLs;
    QList<qlonglong> imageIDs;

    foreach (const ImageInfo& info, infos)
    {
        imageIDs << info.id();
        kioURLs << info.databaseUrl();
    }

    if (kioURLs.isEmpty() || imageIDs.isEmpty())
    {
        return;
    }

    if (currentAlbum && currentAlbum->type() != Album::PHYSICAL)
    {
        currentAlbum = 0;
    }

    QString header(i18n("<p>Please select the destination album from the digiKam library to "
                        "move the selected images into.</p>"));

    Album* album = AlbumSelectDialog::selectAlbum(m_widget, static_cast<PAlbum*>(currentAlbum), header);

    if (!album)
    {
        return;
    }

    KIO::Job* job = DIO::move(kioURLs, imageIDs, (PAlbum*)album);
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));
}

void ImageViewUtilities::insertToLightTableAuto(const QList<ImageInfo>& all, const QList<ImageInfo>& selected, const ImageInfo& current)
{
    ImageInfoList list          = selected;
    ImageInfo     singleInfo    = current;
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
    LightTableWindow* ltview = LightTableWindow::lightTableWindow();

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

void ImageViewUtilities::insertToQueueManagerAuto(const QList<ImageInfo>& list, const ImageInfo& current)
{
    insertToQueueManager(list, current, QueueMgrWindow::queueManagerWindow()->isBusy());
}

void ImageViewUtilities::insertToQueueManager(const QList<ImageInfo>& list, const ImageInfo& current, bool newQueue)
{
    Q_UNUSED(current);

    QueueMgrWindow* bqmview = QueueMgrWindow::queueManagerWindow();

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
        bqmview->addNewQueue();
    }

    bqmview->loadImageInfos(list, bqmview->currentQueueId());
}

void ImageViewUtilities::insertSilentToQueueManager(const QList<ImageInfo>& list, const ImageInfo& /*current*/, int queueid)
{
    QueueMgrWindow* bqmview = QueueMgrWindow::queueManagerWindow();
    bqmview->loadImageInfos(list, queueid);
}

void ImageViewUtilities::openInEditor(const ImageInfo& info, const QList<ImageInfo>& allInfosToOpen, Album* currentAlbum)
{
    if (info.isNull())
    {
        return;
    }

    QFileInfo fi(info.filePath());
    QString imagefilter = AlbumSettings::instance()->getImageFileFilter();
    imagefilter         += AlbumSettings::instance()->getRawFileFilter();

    // If the current item is not an image file.
    if ( !imagefilter.contains(fi.suffix().toLower()) )
    {
        KMimeType::Ptr mimePtr      = KMimeType::findByUrl(info.fileUrl(), 0, true, true);
        const KService::List offers = KServiceTypeTrader::self()->query(mimePtr->name(), "Type == 'Application'");

        if (offers.isEmpty())
        {
            return;
        }

        KService::Ptr ptr = offers.first();
        // Run the dedicated app to show the item.
        KRun::run(*ptr, info.fileUrl(), m_widget);
        return;
    }

    // Run digiKam ImageEditor with all image from current Album.

    ImageWindow* imview = ImageWindow::imageWindow();

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

} // namespace Digikam
