/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to interface digiKam with kipi library.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well dot com>
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kipiinterface.moc"

// Qt includes

#include <QAbstractItemModel>

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>

// Libkipi includes

#include <libkipi/imagecollection.h>
#include <libkipi/imageinfoshared.h>
#include <libkipi/imagecollectionshared.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "applicationsettings.h"
#include "imageinfo.h"
#include "metadatasettings.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "digikamview.h"
#include "loadingcacheinterface.h"
#include "filereadwritelock.h"
#include "scancontroller.h"
#include "imageattributeswatch.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "kipiimageinfo.h"
#include "kipiimagecollection.h"
#include "progressmanager.h"

namespace Digikam
{

class KipiInterface::Private
{
public:

    Private()
    {
        tagModel        = 0;
        thumbLoadThread = 0;
        albumManager    = 0;
    }

    AlbumManager*        albumManager;
    ThumbnailLoadThread* thumbLoadThread;
    QAbstractItemModel*  tagModel;
};

KipiInterface::KipiInterface(QObject* const parent, const char* name)
    : KIPI::Interface(parent, name), d(new Private())
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();
    d->albumManager    = AlbumManager::instance();

    connect(DigikamApp::instance()->view(), SIGNAL(signalSelectionChanged(int)),
            this, SLOT(slotSelectionChanged(int)));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
}

KipiInterface::~KipiInterface()
{
    delete d;
}

KIPI::ImageCollection KipiInterface::currentAlbum()
{
    if(d->albumManager->currentAlbums().isEmpty())
    {
        return KIPI::ImageCollection(0);
    }

    Album* currAlbum = d->albumManager->currentAlbums().first();

    if (currAlbum)
    {
        return KIPI::ImageCollection(new KipiImageCollection(KipiImageCollection::AllItems,
                                                             currAlbum,
                                                             hostSetting("FileExtensions").toString()));
    }
    else
    {
        return KIPI::ImageCollection(0);
    }
}

KIPI::ImageCollection KipiInterface::currentSelection()
{
    if (d->albumManager->currentAlbums().isEmpty())
    {
            return KIPI::ImageCollection(0);
    }

    Album* currAlbum = d->albumManager->currentAlbums().first();

    if (currAlbum)
    {
        return KIPI::ImageCollection(new KipiImageCollection(KipiImageCollection::SelectedItems,
                                                             currAlbum,
                                                             hostSetting("FileExtensions").toString()));
    }
    else
    {
        return KIPI::ImageCollection(0);
    }
}

QList<KIPI::ImageCollection> KipiInterface::allAlbums()
{
    QList<KIPI::ImageCollection> result;
    QString fileFilter(hostSetting("FileExtensions").toString());

    const AlbumList palbumList = d->albumManager->allPAlbums();

    for (AlbumList::ConstIterator it = palbumList.constBegin();
         it != palbumList.constEnd(); ++it)
    {
        // don't add the root album
        if ((*it)->isRoot())
        {
            continue;
        }

        KipiImageCollection* col = new KipiImageCollection(KipiImageCollection::AllItems, *it, fileFilter);
        result.append(KIPI::ImageCollection(col));
    }

    const AlbumList talbumList = d->albumManager->allTAlbums();

    for (AlbumList::ConstIterator it = talbumList.constBegin();
         it != talbumList.constEnd(); ++it)
    {
        // don't add the root album
        if ((*it)->isRoot())
        {
            continue;
        }

        KipiImageCollection* col = new KipiImageCollection(KipiImageCollection::AllItems,
                                                           *it, fileFilter);
        result.append(KIPI::ImageCollection(col));
    }

    return result;
}

KIPI::ImageInfo KipiInterface::info(const KUrl& url)
{
    return KIPI::ImageInfo(new KipiImageInfo(this, url));
}

void KipiInterface::refreshImages(const KUrl::List& urls)
{
    KUrl::List ulist = urls;

    // Hard Refresh
    QSet<QString>    dirs;
    QList<qlonglong> ids;

    foreach(const KUrl& url, urls)
    {
        ImageInfo info = ImageInfo::fromUrl(url);

        if (!info.isNull())
        {
            ids << info.id();
        }

        QString path = url.toLocalFile();
        ThumbnailLoadThread::deleteThumbnail(path);
        LoadingCacheInterface::fileChanged(path);
        ImageAttributesWatch::instance()->fileMetadataChanged(url);
        dirs << url.directory();
    }

    ScanController::instance()->hintAtModificationOfItems(ids);

    foreach(const QString& dir, dirs)
    {
        ScanController::instance()->scheduleCollectionScan(dir);
    }
}

int KipiInterface::features() const
{
    return(KIPI::CollectionsHaveComments
           | KIPI::CollectionsHaveCategory
           | KIPI::CollectionsHaveCreationDate
           | KIPI::ImagesHasComments
           | KIPI::ImagesHasTitlesWritable
           | KIPI::ImagesHasTime
           | KIPI::HostSupportsTags
           | KIPI::HostSupportsRating
           | KIPI::HostAcceptNewImages
           | KIPI::HostSupportsThumbnails
           | KIPI::HostSupportsProgressBar
           | KIPI::HostSupportsReadWriteLock
           | KIPI::HostSupportsPickLabel
           | KIPI::HostSupportsColorLabel
          );
}

bool KipiInterface::addImage(const KUrl& url, QString& errmsg)
{
    // Note : All copy/move operations are processed by the plugins.

    if (url.isValid() == false)
    {
        errmsg = i18n("Target URL %1 is not valid.", url.toLocalFile());
        return false;
    }

    PAlbum* targetAlbum = d->albumManager->findPAlbum(url.directory());

    if (!targetAlbum)
    {
        errmsg = i18n("Target album is not in the album library.");
        return false;
    }

    //d->albumManager->refreshItemHandler( url );

    return true;
}

void KipiInterface::delImage(const KUrl& url)
{
    KUrl rootURL(CollectionManager::instance()->albumRoot(url));

    if (!rootURL.isParentOf(url))
    {
        kWarning() << "URL not in the album library";
    }

    // Is there a PAlbum for this URL

    PAlbum* palbum = d->albumManager->findPAlbum(KUrl(url.directory()));

    if (palbum)
    {
        // delete the item from the database
        DatabaseAccess().db()->deleteItem(palbum->id(), url.fileName());
    }
    else
    {
        kWarning() << "Cannot find Parent album in the album library";
    }
}

void KipiInterface::slotSelectionChanged(int count)
{
    emit selectionChanged(count);
}

void KipiInterface::slotCurrentAlbumChanged(QList<Album*> albums)
{
    emit currentAlbumChanged(!(albums.isEmpty()));
}

void KipiInterface::thumbnail(const KUrl& url, int /*size*/)
{
    // NOTE: size is not used here. Cache use the max pixmap size to store thumbs (256).
    d->thumbLoadThread->find(ImageInfo::fromUrl(url).thumbnailIdentifier());
}

void KipiInterface::thumbnails(const KUrl::List& list, int size)
{
    for (KUrl::List::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        thumbnail(*it, size);
    }
}

void KipiInterface::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    emit gotThumbnail(KUrl(desc.filePath), pix);
}

KIPI::ImageCollectionSelector* KipiInterface::imageCollectionSelector(QWidget* parent)
{
    return (new KipiImageCollectionSelector(this, parent));
}

KIPI::UploadWidget* KipiInterface::uploadWidget(QWidget* parent)
{
    return (new KipiUploadWidget(this, parent));
}

QAbstractItemModel* KipiInterface::getTagTree() const
{

    if (!d->tagModel)
    {
        QAbstractItemModel* newTagModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, NULL);
        d->tagModel                     = newTagModel;
    }

    return d->tagModel;
}

QVariant KipiInterface::hostSetting(const QString& settingName)
{
    MetadataSettings* mSettings = MetadataSettings::instance();
    ApplicationSettings* aSettings    = ApplicationSettings::instance();

    if (!mSettings || !aSettings)
    {
        return QVariant();
    }

    MetadataSettingsContainer set = mSettings->settings();

    if (settingName == QString("WriteMetadataUpdateFiletimeStamp"))
    {
        return (set.updateFileTimeStamp);
    }
    else if (settingName == QString("WriteMetadataToRAW"))
    {
        return (set.writeRawFiles);
    }
    else if (settingName == QString("UseXMPSidecar4Reading"))
    {
        return (set.useXMPSidecar4Reading);
    }
    else if (settingName == QString("MetadataWritingMode"))
    {
        return (set.metadataWritingMode);
    }
    else if (settingName == QString("FileExtensions"))
    {
        // NOTE : do not save type mime settings into a local variable, as this
        // might change in the main app

        return QString(aSettings->getImageFileFilter() + ' ' +
                       aSettings->getMovieFileFilter() + ' ' +
                       aSettings->getAudioFileFilter() + ' ' +
                       aSettings->getRawFileFilter());
    }
    else if (settingName == QString("ImagesExtensions"))
    {
        return QString(aSettings->getImageFileFilter());
    }
    else if (settingName == QString("RawExtensions"))
    {
        return QString(aSettings->getRawFileFilter());
    }
    else if (settingName == QString("VideoExtensions"))
    {
        return QString(aSettings->getMovieFileFilter());
    }
    else if (settingName == QString("AudioExtensions"))
    {
        return QString(aSettings->getAudioFileFilter());
    }

    return QVariant();
}

QString KipiInterface::progressScheduled(const QString& title, bool canBeCanceled, bool hasThumb) const
{
    ProgressItem* item = ProgressManager::createProgressItem(title, QString(), canBeCanceled, hasThumb);

    if (canBeCanceled)
    {
        connect(item, SIGNAL(progressItemCanceled(QString)),
                this, SIGNAL(progressCanceled(QString)));
    }

    return item->id();
}

void KipiInterface::progressValueChanged(const QString& id, float percent)
{
    ProgressItem* item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setProgress(percent);
    }
}

void KipiInterface::progressStatusChanged(const QString& id, const QString& status)
{
    ProgressItem* item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setStatus(status);
    }
}

void KipiInterface::progressThumbnailChanged(const QString& id, const QPixmap& thumb)
{
    ProgressItem* item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setThumbnail(thumb);
    }
}

void KipiInterface::progressCompleted(const QString& id)
{
    ProgressItem* item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setComplete();
    }
}

#if KIPI_VERSION >= 0x020100
void KipiInterface::aboutToEdit(const KUrl& url, KIPI::EditHints hints)
{
    if (hints == KIPI::HintMetadataOnlyChange)
    {
        ImageInfo info = ImageInfo::fromUrl(url);
        ScanController::instance()->beginFileMetadataWrite(info);
    }
}

void KipiInterface::editingFinished(const KUrl& url, KIPI::EditHints hints)
{
    if ((hints & ~KIPI::HintEditAborted) == KIPI::HintMetadataOnlyChange)
    {
        ImageInfo info = ImageInfo::fromUrl(url);
        ScanController::instance()->finishFileMetadataWrite(info, !(hints & KIPI::HintEditAborted));
    }
}
#endif


// ---------------------------------------------------------------------------------------

class KipiInterfaceFileReadWriteLock : public KIPI::FileReadWriteLock
{
public:

    explicit KipiInterfaceFileReadWriteLock(const QString& filePath)
        : key(filePath)
    {
    }

    ~KipiInterfaceFileReadWriteLock()
    {
    }

public:

    void lockForRead()                { key.lockForRead();                   }
    void lockForWrite()               { key.lockForWrite();                  }
    bool tryLockForRead()             { return key.tryLockForRead();         }
    bool tryLockForRead(int timeout)  { return key.tryLockForRead(timeout);  }
    bool tryLockForWrite()            { return key.tryLockForWrite();        }
    bool tryLockForWrite(int timeout) { return key.tryLockForWrite(timeout); }
    void unlock()                     { key.unlock();                        }

public:

    FileReadWriteLockKey key;
};

KIPI::FileReadWriteLock* KipiInterface::createReadWriteLock(const KUrl& url) const
{
    return new KipiInterfaceFileReadWriteLock(url.toLocalFile());
}

}  // namespace Digikam
