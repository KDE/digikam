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
        albumDlg(0)
    {
    }
    
    WSWizard*               wizard;
    DInfoInterface*         iface;
    
    WSTalker*               talker;
    
    WSSettings::WebService  ws;
    QString                 serviceName;
    
    WSNewAlbumDialog*       albumDlg;
    QString                 currentAlbumId;
    
    QString                 tmpPath;
    QString                 tmpDir;
    unsigned int            imagesCount;
    unsigned int            imagesTotal;
    
    QList<QUrl>             transferQueue;
};
    
WSAuthentication::WSAuthentication(QWidget* const parent, DInfoInterface* const iface)
  : d(new Private())
{
    d->wizard   = dynamic_cast<WSWizard*>(parent);
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
    // Just to be sure that all temporary files will be removed after all 
    QFile::remove(d->tmpPath);
    
    delete d;
}

void WSAuthentication::createTalker(WSSettings::WebService ws, const QString& serviceName)
{
    d->ws = ws;
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
            d->talker   = new FbTalker(d->wizard);
            d->albumDlg = new FbNewAlbumDlg(d->wizard, d->serviceName);
            connect(d->albumDlg, SIGNAL(signalCreateAlbum(const FbAlbum&)),
                    d->talker, SLOT(slotCreateAlbum(const FbAlbum&)));
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
    
    connect(this, SIGNAL(signalResponseTokenReceived(const QMap<QString, QString>&)),
            d->talker, SLOT(slotResponseTokenReceived(const QMap<QString, QString>&)));
    connect(d->talker, SIGNAL(signalOpenBrowser(const QUrl&)),
            this, SIGNAL(signalOpenBrowser(const QUrl&)));
    connect(d->talker, SIGNAL(signalCloseBrowser()),
            this, SIGNAL(signalCloseBrowser()));
    connect(d->talker, SIGNAL(signalAuthenticationComplete(bool)),
            this, SIGNAL(signalAuthenticationComplete(bool)));
    
    connect(d->talker, SIGNAL(signalListAlbumsDone(int, const QString&, const QList <WSAlbum>&)),
            this, SLOT(slotListAlbumsDone(int, const QString&, const QList <WSAlbum>&)));
    connect(d->talker, SIGNAL(signalAddPhotoDone(int,QString)),
            this, SLOT(slotAddPhotoDone(int,QString)));
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
    foreach(const WSAlbum& album, albumsList)
    {
        if(albumTree.contains(album.id))
        {
            albumTree[album.id].title       = album.title;
            albumTree[album.id].uploadable  = album.uploadable;
        }
        else
        {
            AlbumSimplified item(album.title, album.uploadable);
            albumTree[album.id] = item;
        }
        
        if(album.isRoot)
        {
            rootAlbums << album.id;
        }
        else
        {
            if(albumTree.contains(album.parentID))
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
    descriptions.removeAll(QString::fromLatin1(""));
    return descriptions.join(QString::fromLatin1("\n\n"));
}

void WSAuthentication::prepareImageForUpload(const QString& imgPath, QString& caption)
{
    QImage image = PreviewLoadThread::loadHighQualitySynchronously(imgPath).copyQImage();
    
    if (image.isNull())
    {
        image.load(imgPath);
    }
    
    if (image.isNull())
    {
        emit d->talker->signalAddPhotoDone(666, i18n(QString::fromLatin1("Cannot open image at %1").arg(imgPath).toLatin1()));
        return;
    }

    // get temporary file name
    d->tmpPath = d->tmpDir + QFileInfo(imgPath).baseName().trimmed() + QString::fromLatin1(".jpg");
    
    // rescale image if requested
    int maxDim = d->wizard->settings()->imageSize;
    
    if (d->wizard->settings()->imagesChangeProp
        && (image.width() > maxDim || image.height() > maxDim))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing to " << maxDim;
        image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    }
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Saving to temp file: " << d->tmpPath;
    image.save(d->tmpPath, "JPEG", d->wizard->settings()->imageCompression);
    
    // copy meta data to temporary image
    
    DMetadata meta;
    
    if (meta.load(imgPath))
    {
        caption = getImageCaption(imgPath);
        meta.setImageDimensions(image.size());
        meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);
        meta.setImageProgramId(QString::fromLatin1("digiKam"), digiKamVersion());
        meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
        meta.save(d->tmpPath);
    }
    else
    {
        caption.clear();
    }
}

void WSAuthentication::listAlbums()
{
    d->talker->listAlbums();
}

void WSAuthentication::uploadNextPhoto()
{    
    if (d->transferQueue.isEmpty())
    {
        emit signalDone();
        return;
    }
    
    QString imgPath = d->transferQueue.first().toLocalFile();
    
    QString caption;
    
    if (d->wizard->settings()->imagesChangeProp)
    {
        prepareImageForUpload(imgPath, caption);
        d->talker->addPhoto(d->tmpPath, d->currentAlbumId, caption);
    }
    else
    {
        caption = getImageCaption(imgPath);
        d->tmpPath.clear();
        d->talker->addPhoto(imgPath, d->currentAlbumId, caption);
    }
}

void WSAuthentication::slotStartTransfer()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotStartTransfer invoked";

    d->transferQueue  = d->wizard->settings()->inputImages;
    
    if (d->transferQueue.isEmpty())
    {
        return;
    }
    
    d->currentAlbumId = d->wizard->settings()->currentAlbumId;
    d->imagesTotal    = d->transferQueue.count();
    d->imagesCount    = 0;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "upload request got album id from widget: " << d->currentAlbumId;
        
    uploadNextPhoto();
}

void WSAuthentication::slotAddPhotoDone(int errCode, const QString& errMsg)
{
    // Remove temporary file if it was used
    if (!d->tmpPath.isEmpty())
    {
        QFile::remove(d->tmpPath);
        d->tmpPath.clear();
    }
    
    if (errCode == 0)
    {
        d->transferQueue.removeFirst();
        d->imagesCount++;
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

void WSAuthentication::slotListAlbumsDone(int errCode, const QString& errMsg, const QList<WSAlbum>& albumsList)
{
    QString albumDebug = QString::fromLatin1("");
    foreach(const WSAlbum &album, albumsList)
    {
        albumDebug.append(QString::fromLatin1("%1: %2\n").arg(album.id).arg(album.title));
    }
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received albums (errCode = " << errCode << ", errMsg = " 
                                     << errMsg << "): " << albumDebug;
    
    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(), 
                              i18n(QString::fromLatin1("%1 Call Failed: %2\n").arg(d->serviceName).arg(errCode).toLatin1()),
                              errMsg);
        return;
    }
    
    QMap<QString, AlbumSimplified> albumTree;
    QStringList rootAlbums;
    parseTreeFromListAlbums(albumsList, albumTree, rootAlbums);
    
    emit signalListAlbumsDone(albumTree, rootAlbums, QLatin1String(""));
}

void WSAuthentication::slotNewAlbumRequest()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Slot create New Album";
    
    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Calling New Album method";
        d->albumDlg->getAlbumProperties();
    }
}

} // namespace Digikam




