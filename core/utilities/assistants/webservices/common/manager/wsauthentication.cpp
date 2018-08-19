/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-03
 * Description : Web Service authentication container.
 *
 * Copyright (C) 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#include "wsauthentication.h"

// Qt includes

#include <QApplication>
#include <QMap>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "dmetadata.h"
#include "previewloadthread.h"
#include "wstoolutils.h"
#include "wstalker.h"
#include "dbtalker.h"
#include "fbtalker.h"
#include "fbnewalbumdlg.h"
#include "flickrtalker.h"
#include "gptalker.h"
#include "gdtalker.h"
#include "imgurtalker.h"
#include "smugtalker.h"

namespace Digikam
{

class WSAuthentication::Private
{
public:

    explicit Private()
      : wizard(0),
        iface(0),
        talker(0),
        ws(WSSettings::WebService::FLICKR),
        albumDlg(0),
        imagesCount(0),
        imagesTotal(0)
    {
    }

    WSWizard*               wizard;
    DInfoInterface*         iface;

    WSTalker*               talker;

    WSSettings::WebService  ws;
    QString                 serviceName;

    WSNewAlbumDialog*       albumDlg;
    QString                 currentAlbumId;
    WSAlbum                 baseAlbum;

    QStringList             tmpPath;
    QString                 tmpDir;
    unsigned int            imagesCount;
    unsigned int            imagesTotal;
    QMap<QString, QString>  imagesCaption;

    QList<QUrl>             transferQueue;
};

WSAuthentication::WSAuthentication(QWidget* const parent, DInfoInterface* const iface)
    : d(new Private())
{
    d->wizard = dynamic_cast<WSWizard*>(parent);

    if(d->wizard)
    {
        d->iface = d->wizard->iface();
    }
    else
    {
        d->iface = iface;
    }

    /* --------------------
     * Temporary path to store images before uploading
     */

    d->tmpPath.clear();
    d->tmpDir = WSToolUtils::makeTemporaryDir(d->serviceName.toUtf8()).absolutePath() + QLatin1Char('/');
}

WSAuthentication::~WSAuthentication()
{
    slotCancel();
    delete d;
}

void WSAuthentication::createTalker(WSSettings::WebService ws, const QString& serviceName)
{
    d->ws          = ws;
    d->serviceName = serviceName;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "create " << serviceName << "talker";

    switch(ws)
    {
        case WSSettings::WebService::FLICKR:
            //d->talker = new FlickrTalker(d->wizard, serviceName, d->iface);
            break;
        case WSSettings::WebService::DROPBOX:
            //d->talker = new DBTalker(d->wizard);
            break;
        case WSSettings::WebService::IMGUR:
            //d->talker = new ImgurTalker(d->wizard);
            break;
        case WSSettings::WebService::FACEBOOK:
            d->albumDlg = new FbNewAlbumDlg(d->wizard, d->serviceName);
            d->talker   = new FbTalker(d->wizard, d->albumDlg);
            break;
        case WSSettings::WebService::SMUGMUG:
            //d->talker = new SmugTalker(d->iface, d->wizard);
            break;
        case WSSettings::WebService::GDRIVE:
            //d->talker = new GDTalker(d->wizard);
            break;
        case WSSettings::WebService::GPHOTO:
            //d->talker = new GPTalker(d->wizard);
            break;
    }

    connect(d->talker, SIGNAL(signalOpenBrowser(QUrl)),
            this, SIGNAL(signalOpenBrowser(QUrl)));

    connect(d->talker, SIGNAL(signalCloseBrowser()),
            this, SIGNAL(signalCloseBrowser()));

    connect(d->talker, SIGNAL(signalAuthenticationComplete(bool)),
            this, SIGNAL(signalAuthenticationComplete(bool)));

    connect(this, SIGNAL(signalResponseTokenReceived(QMap<QString,QString>)),
            d->talker, SLOT(slotResponseTokenReceived(QMap<QString,QString>)));

    connect(d->talker, SIGNAL(signalCreateAlbumDone(int,QString,QString)),
            this, SIGNAL(signalCreateAlbumDone(int,QString,QString)));

    connect(d->talker, SIGNAL(signalListAlbumsDone(int,QString,QList<WSAlbum>)),
            this, SLOT(slotListAlbumsDone(int,QString,QList<WSAlbum>)));

    connect(d->talker, SIGNAL(signalAddPhotoDone(int,QString)),
            this, SLOT(slotAddPhotoDone(int,QString)));
}

void WSAuthentication::cancelTalker()
{
    if (d->talker)
    {
        d->talker->cancel();
    }
}

QString WSAuthentication::webserviceName()
{
    return d->serviceName;
}

void WSAuthentication::authenticate()
{
    d->talker->authenticate();
}

void WSAuthentication::reauthenticate()
{
    d->talker->reauthenticate();
}

bool WSAuthentication::authenticated() const
{
    return d->talker->linked();
}

void WSAuthentication::parseTreeFromListAlbums(const QList <WSAlbum>& albumsList,
                                               QMap<QString, AlbumSimplified>& albumTree,
                                               QStringList& rootAlbums)
{
    foreach (const WSAlbum& album, albumsList)
    {
        if (albumTree.contains(album.id))
        {
            albumTree[album.id].title       = album.title;
            albumTree[album.id].uploadable  = album.uploadable;
        }
        else
        {
            AlbumSimplified item(album.title, album.uploadable);
            albumTree[album.id] = item;
        }

        if (album.isRoot)
        {
            rootAlbums << album.id;
        }
        else
        {
            if (albumTree.contains(album.parentID))
            {
                albumTree[album.parentID].childrenIDs << album.id;
            }
            else
            {
                AlbumSimplified parentAlbum;
                parentAlbum.childrenIDs << album.id;
                albumTree[album.parentID] = parentAlbum;
            }
        }
    }
}

QString WSAuthentication::getImageCaption(const QString& fileName)
{
    DItemInfo info(d->iface->itemInfo(QUrl::fromLocalFile(fileName)));

    // If webservice doesn't support image titles, include it in descriptions if needed.
    QStringList descriptions = QStringList() << info.title() << info.comment();
    descriptions.removeAll(QLatin1String(""));
    return descriptions.join(QLatin1String("\n\n"));
}

void WSAuthentication::prepareForUpload()
{
    d->transferQueue  = d->wizard->settings()->inputImages;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "prepareForUpload invoked";

    if (d->transferQueue.isEmpty())
    {
        emit(QLatin1String("transferQueue is empty"), true);
        return;
    }

    d->currentAlbumId = d->wizard->settings()->currentAlbumId;
    d->imagesTotal    = d->transferQueue.count();
    d->imagesCount    = 0;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "upload request got album id from widget: " << d->currentAlbumId;

    if (d->wizard->settings()->imagesChangeProp)
    {
        foreach(const QUrl& imgUrl, d->transferQueue)
        {
            QString imgPath = imgUrl.toLocalFile();
            QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();

            if (image.isNull())
            {
                image.load(imgPath);
            }

            if (image.isNull())
            {
                emit d->talker->signalAddPhotoDone(666, i18n("Cannot open image at %1\n", imgPath));
                return;
            }

            // get temporary file name
            d->tmpPath << d->tmpDir + QFileInfo(imgPath).baseName().trimmed() + d->wizard->settings()->format();

            // rescale image if requested
            int maxDim = d->wizard->settings()->imageSize;

            if (image.width() > maxDim || image.height() > maxDim)
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing to " << maxDim;
                image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
            }

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Saving to temp file: " << d->tmpPath.last();
            image.save(d->tmpPath.last(), "JPEG", d->wizard->settings()->imageCompression);

            // copy meta data to temporary image and get caption for image
            DMetadata meta;
            QString caption = QLatin1String("");

            if (meta.load(imgPath))
            {
                meta.setImageDimensions(image.size());
                meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
                meta.setImageProgramId(QLatin1String("digiKam"), digiKamVersion());
                meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
                meta.save(d->tmpPath.last());
                caption = getImageCaption(imgPath);
            }

            d->imagesCaption[imgPath] = caption;
        }
    }
}

unsigned int WSAuthentication::numberItemsUpload()
{
    return d->imagesTotal;
}

void WSAuthentication::uploadNextPhoto()
{
    if (d->transferQueue.isEmpty())
    {
        emit signalDone();
        return;
    }

    /*
     * This comparaison is a little bit complicated and may seem unnecessary, but it will be useful later
     * when we will be able to choose to change or not image properties for EACH image.
     */
    QString imgPath = d->transferQueue.first().toLocalFile();
    QString tmpPath = d->tmpDir + QFileInfo(imgPath).baseName().trimmed() + d->wizard->settings()->format();

    if (!d->tmpPath.isEmpty() && tmpPath == d->tmpPath.first())
    {
        d->talker->addPhoto(tmpPath, d->currentAlbumId, d->imagesCaption[imgPath]);
        d->tmpPath.removeFirst();
    }
    else
    {
        d->talker->addPhoto(imgPath, d->currentAlbumId, d->imagesCaption[imgPath]);
    }
}

void WSAuthentication::startTransfer()
{
    uploadNextPhoto();
}

void WSAuthentication::slotCancel()
{
    // First we cancel talker
    cancelTalker();

    // Then the folder containing all temporary photos to upload will be removed after all.
    QDir tmpDir(d->tmpDir);
    if(tmpDir.exists())
    {
        tmpDir.removeRecursively();
    }

    emit signalProgress(0);
}

void WSAuthentication::slotNewAlbumRequest()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Slot create New Album";

    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Calling New Album method";
        d->talker->createNewAlbum();
    }
}

void WSAuthentication::slotListAlbumsRequest()
{
    d->talker->listAlbums();
}

void WSAuthentication::slotListAlbumsDone(int errCode, const QString& errMsg, const QList<WSAlbum>& albumsList)
{
    QString albumDebug = QLatin1String("");

    foreach (const WSAlbum &album, albumsList)
    {
        albumDebug.append(QString::fromLatin1("%1: %2\n").arg(album.id).arg(album.title));
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received albums (errCode = " << errCode << ", errMsg = " 
                                     << errMsg << "): " << albumDebug;

    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(),
                              i18n("%1 Call Failed",  d->serviceName), 
                              i18n("Code: %1. %2", errCode, errMsg));
        return;
    }

    QMap<QString, AlbumSimplified> albumTree;
    QStringList rootAlbums;
    parseTreeFromListAlbums(albumsList, albumTree, rootAlbums);

    emit signalListAlbumsDone(albumTree, rootAlbums, QLatin1String(""));
}

void WSAuthentication::slotAddPhotoDone(int errCode, const QString& errMsg)
{
    if (errCode == 0)
    {
        emit signalMessage(QDir::toNativeSeparators(d->transferQueue.first().toLocalFile()), false);
        d->transferQueue.removeFirst();

        d->imagesCount++;
        emit signalProgress(d->imagesCount);
    }
    else
    {
        if (QMessageBox::question(d->wizard, i18n("Uploading Failed"),
            i18n("Failed to upload photo: %1\n"
            "Do you want to continue?", errMsg))
            != QMessageBox::Yes)
        {
            d->transferQueue.clear();
            return;
        }
    }

    uploadNextPhoto();
}

} // namespace Digikam
