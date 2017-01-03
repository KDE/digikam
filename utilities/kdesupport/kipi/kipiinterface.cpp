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
 * Copyright (C) 2004-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kipiinterface.h"

// Qt includes

#include <QAbstractItemModel>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Libkipi includes

#include <KIPI/ImageCollection>
#include <KIPI/ImageInfoShared>
#include <KIPI/ImageCollectionShared>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "coredb.h"
#include "applicationsettings.h"
#include "drawdecoderwidget.h"
#include "imageinfo.h"
#include "metadatasettings.h"
#include "collectionmanager.h"
#include "coredbaccess.h"
#include "digikamapp.h"
#include "digikamview.h"
#include "drawdecoder.h"
#include "loadingcacheinterface.h"
#include "previewloadthread.h"
#include "filereadwritelock.h"
#include "scancontroller.h"
#include "imageattributeswatch.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "kipiimageinfo.h"
#include "kipiimagecollection.h"
#include "progressmanager.h"
#include "dimgloaderobserver.h"
#include "dimg.h"
#include "dmetadata.h"

namespace Digikam
{

class KipiDImgObserver : public DImgLoaderObserver
{

public:

    explicit KipiDImgObserver(bool* cancel)
        : DImgLoaderObserver(),
          m_cancel(cancel)
    {
    }

    ~KipiDImgObserver()
    {
    }

    bool continueQuery(const DImg* const)
    {
        if (m_cancel)
            return (!*m_cancel);

        return true;
    }

private:

    bool* m_cancel;
};

// ------------------------------------------------------------------------------

class KipiInterface::Private
{
public:

    Private()
    {
        tagModel        = 0;
        thumbLoadThread = 0;
        previewThread   = 0;
        albumManager    = 0;
    }

    AlbumManager*        albumManager;
    ThumbnailLoadThread* thumbLoadThread;
    PreviewLoadThread*   previewThread;
    QAbstractItemModel*  tagModel;
};

KipiInterface::KipiInterface(QObject* const parent, const QString& name)
    : KIPI::Interface(parent, name),
      d(new Private())
{
    d->previewThread   = new PreviewLoadThread(this);
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();
    d->albumManager    = AlbumManager::instance();

    connect(DigikamApp::instance()->view(), SIGNAL(signalSelectionChanged(int)),
            this, SLOT(slotSelectionChanged(int)));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(d->previewThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription,DImg)));
}

KipiInterface::~KipiInterface()
{
    delete d;
}

KIPI::ImageCollection KipiInterface::currentAlbum()
{
    if (d->albumManager->currentAlbums().isEmpty())
    {
        return KIPI::ImageCollection(0);
    }

    Album* const currAlbum = d->albumManager->currentAlbums().first();

    if (currAlbum)
    {
        return KIPI::ImageCollection(new KipiImageCollection(KipiImageCollection::AllItems,
                                                             currAlbum,
                                                             ApplicationSettings::instance()->getAllFileFilter()));
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

    Album* const currAlbum = d->albumManager->currentAlbums().first();

    if (currAlbum)
    {
        return KIPI::ImageCollection(new KipiImageCollection(KipiImageCollection::SelectedItems,
                                                             currAlbum,
                                                             ApplicationSettings::instance()->getAllFileFilter()));
    }
    else
    {
        return KIPI::ImageCollection(0);
    }
}

QList<KIPI::ImageCollection> KipiInterface::allAlbums()
{
    QList<KIPI::ImageCollection> result;
    QString fileFilter(ApplicationSettings::instance()->getAllFileFilter());

    const AlbumList palbumList = d->albumManager->allPAlbums();

    for (AlbumList::ConstIterator it = palbumList.constBegin();
         it != palbumList.constEnd(); ++it)
    {
        // don't add the root album
        if ((*it)->isRoot())
        {
            continue;
        }

        KipiImageCollection* const col = new KipiImageCollection(KipiImageCollection::AllItems, *it, fileFilter);
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

        KipiImageCollection* const col = new KipiImageCollection(KipiImageCollection::AllItems, *it, fileFilter);
        result.append(KIPI::ImageCollection(col));
    }

    return result;
}

KIPI::ImageInfo KipiInterface::info(const QUrl& url)
{
    return KIPI::ImageInfo(new KipiImageInfo(this, url));
}

void KipiInterface::refreshImages(const QList<QUrl>& urls)
{
    QList<QUrl> ulist = urls;

    // Hard Refresh
    QSet<QString>    dirs;
    QList<qlonglong> ids;

    foreach(const QUrl& url, urls)
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
        dirs << url.adjusted(QUrl::RemoveFilename).toLocalFile();
    }

    ScanController::instance()->hintAtModificationOfItems(ids);

    foreach(const QString& dir, dirs)
    {
        ScanController::instance()->scheduleCollectionScan(dir);
    }
}

int KipiInterface::features() const
{
    return(  KIPI::CollectionsHaveComments
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
           | KIPI::HostSupportsPreviews
           | KIPI::HostSupportsRawProcessing
           | KIPI::HostSupportsMetadataProcessing
           | KIPI::HostSupportsSaveImages
          );
}

bool KipiInterface::addImage(const QUrl& url, QString& errmsg)
{
    // Note : All copy/move operations are processed by the plugins.

    if (url.isValid() == false)
    {
        errmsg = i18n("Target URL %1 is not valid.", url.toLocalFile());
        return false;
    }

    PAlbum* const targetAlbum = d->albumManager->findPAlbum(url.adjusted(QUrl::RemoveFilename));

    if (!targetAlbum)
    {
        errmsg = i18n("Target album is not in the album library.");
        return false;
    }

    //d->albumManager->refreshItemHandler( url );

    return true;
}

void KipiInterface::delImage(const QUrl& url)
{
    QUrl rootURL(CollectionManager::instance()->albumRoot(url));

    if (!rootURL.isParentOf(url))
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "URL not in the album library";
    }

    // Is there a PAlbum for this URL

    PAlbum* const palbum = d->albumManager->findPAlbum(url.adjusted(QUrl::RemoveFilename));

    if (palbum)
    {
        // delete the item from the database
        CoreDbAccess().db()->deleteItem(palbum->id(), url.fileName());
    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot find Parent album in the album library";
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

QImage KipiInterface::preview(const QUrl& url)
{
    return (PreviewLoadThread::loadHighQualitySynchronously(url.toLocalFile()).copyQImage());
}

void KipiInterface::preview(const QUrl& url, int /*TODO resizedTo*/)
{
    d->previewThread->loadHighQuality(url.toLocalFile());
}

bool KipiInterface::saveImage(const QUrl& url, const QString& format,
                              const QByteArray& data, uint width, uint height,
                              bool  sixteenBit, bool hasAlpha,
                              bool* cancel)
{
    KipiDImgObserver* const observer = new KipiDImgObserver(cancel);
    DImg img(width, height, sixteenBit, hasAlpha, (uchar*)data.constData(), true);
    bool ret = img.save(url.toLocalFile(), format, observer);
    delete observer;
    return ret;
}

void KipiInterface::thumbnail(const QUrl& url, int /*size*/)
{
    // NOTE: size is not used here. Cache use the max pixmap size to store thumbs.

    d->thumbLoadThread->find(ThumbnailIdentifier(url.toLocalFile()));
}

void KipiInterface::thumbnails(const QList<QUrl>& list, int size)
{
    for (QList<QUrl>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        thumbnail(*it, size);
    }
}

void KipiInterface::slotGotImagePreview(const LoadingDescription& desc, const DImg& image)
{
    QUrl url = QUrl::fromLocalFile(desc.filePath);

    if (desc.isThumbnail() || !url.isValid())
    {
        return;
    }

    emit gotPreview(url, image.copyQImage());
}

void KipiInterface::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    emit gotThumbnail(QUrl::fromLocalFile(desc.filePath), pix);
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
        QAbstractItemModel* const newTagModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, NULL);
        d->tagModel                           = newTagModel;
    }

    return d->tagModel;
}

QString KipiInterface::rawFiles()
{
    return ApplicationSettings::instance()->getRawFileFilter();
}

QString KipiInterface::progressScheduled(const QString& title, bool canBeCanceled, bool hasThumb) const
{
    ProgressItem* const item = ProgressManager::createProgressItem(title, QString(), canBeCanceled, hasThumb);

    if (canBeCanceled)
    {
        connect(item, SIGNAL(progressItemCanceled(QString)),
                this, SIGNAL(progressCanceled(QString)));
    }

    return item->id();
}

void KipiInterface::progressValueChanged(const QString& id, float percent)
{
    ProgressItem* const item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setProgress(percent);
    }
}

void KipiInterface::progressStatusChanged(const QString& id, const QString& status)
{
    ProgressItem* const item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setStatus(status);
    }
}

void KipiInterface::progressThumbnailChanged(const QString& id, const QPixmap& thumb)
{
    ProgressItem* const item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setThumbnail(thumb);
    }
}

void KipiInterface::progressCompleted(const QString& id)
{
    ProgressItem* const item = ProgressManager::instance()->findItembyId(id);

    if (item)
    {
        item->setComplete();
    }
}

void KipiInterface::aboutToEdit(const QUrl& url, KIPI::EditHints hints)
{
    if (hints == KIPI::HintMetadataOnlyChange)
    {
        ImageInfo info = ImageInfo::fromUrl(url);
        ScanController::instance()->beginFileMetadataWrite(info);
    }
}

void KipiInterface::editingFinished(const QUrl& url, KIPI::EditHints hints)
{
    if ((hints & ~KIPI::HintEditAborted) == KIPI::HintMetadataOnlyChange)
    {
        ImageInfo info = ImageInfo::fromUrl(url);
        ScanController::instance()->finishFileMetadataWrite(info, !(hints & KIPI::HintEditAborted));
    }
}

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

KIPI::FileReadWriteLock* KipiInterface::createReadWriteLock(const QUrl& url) const
{
    return new KipiInterfaceFileReadWriteLock(url.toLocalFile());
}

// ---------------------------------------------------------------------------------------

class KipiInterfaceMatadataProcessor : public KIPI::MetadataProcessor
{
public:

    KipiInterfaceMatadataProcessor()
    {
    }

    ~KipiInterfaceMatadataProcessor()
    {
    }

    bool load(const QUrl& url)
    {
        return meta.load(url.toLocalFile());
    }

    bool save(const QUrl& url, bool writeToFileOnly)
    {
        if (writeToFileOnly)
            meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);

        return meta.save(url.toLocalFile());
    }

    bool applyChanges()
    {
        return meta.applyChanges();
    }

    QSize getPixelSize()
    {
        return meta.getPixelSize();
    }

    bool setImageProgramId(const QString& program, const QString& version)
    {
        return meta.setImageProgramId(program, version);
    }

    QSize getImageDimensions()
    {
        return meta.getImageDimensions();
    }

    bool setImageDimensions(const QSize& size)
    {
        return meta.setImageDimensions(size);
    }

    int getImageOrientation()
    {
        return meta.getImageOrientation();
    }

    bool setImageOrientation(int orientation)
    {
        return meta.setImageOrientation((DMetadata::ImageOrientation)orientation);
    }

    bool rotateExifQImage(QImage& img, int orientation)
    {
        return meta.rotateExifQImage(img, (DMetadata::ImageOrientation)orientation);
    }

    QDateTime getImageDateTime()
    {
        return meta.getImageDateTime();
    }

    bool setImageDateTime(const QDateTime& dt)
    {
        return meta.setImageDateTime(dt);
    }

    bool getImagePreview(QImage& img)
    {
        return meta.getImagePreview(img);
    }

    bool setImagePreview(const QImage& img)
    {
        return meta.setImagePreview(img);
    }

    bool hasExif()
    {
        return meta.hasExif();
    }

    bool hasIptc()
    {
        return meta.hasIptc();
    }

    bool hasXmp()
    {
        return meta.hasXmp();
    }

    QByteArray getExif()
    {
        return meta.getExifEncoded();
    }

    QByteArray getIptc()
    {
        return meta.getIptc();
    }

    QByteArray getXmp()
    {
        return meta.getXmp();
    }

    bool setExif(const QByteArray& data)
    {
        return meta.setExif(data);
    }

    bool setIptc(const QByteArray& data)
    {
        return meta.setIptc(data);
    }

    bool setXmp(const QByteArray& data)
    {
        return meta.setXmp(data);
    }

    bool registerXmpNameSpace(const QString& uri, const QString& prefix)
    {
        return meta.registerXmpNameSpace(uri, prefix);
    }

    bool supportXmp()
    {
        return meta.supportXmp();
    }

    bool canWriteXmp(const QUrl& url)
    {
        return meta.canWriteXmp(url.toLocalFile());
    }

    bool removeExifTags(const QStringList& tagFilters)
    {
        DMetadata::MetaDataMap m = meta.getExifTagsDataList(tagFilters);

        if (m.isEmpty())
            return false;

        for (DMetadata::MetaDataMap::iterator it = m.begin(); it != m.end(); ++it)
        {
            meta.removeExifTag(it.key().toLatin1().constData());
        }

        return true;
    }

    bool removeIptcTags(const QStringList& tagFilters)
    {
        DMetadata::MetaDataMap m = meta.getIptcTagsDataList(tagFilters);

        if (m.isEmpty())
            return false;

        for (DMetadata::MetaDataMap::iterator it = m.begin(); it != m.end(); ++it)
        {
            meta.removeIptcTag(it.key().toLatin1().constData());
        }

        return true;
    }

    bool removeXmpTags(const QStringList& tagFilters)
    {
        DMetadata::MetaDataMap m = meta.getXmpTagsDataList(tagFilters);

        if (m.isEmpty())
            return false;

        for (DMetadata::MetaDataMap::iterator it = m.begin(); it != m.end(); ++it)
        {
            meta.removeXmpTag(it.key().toLatin1().constData());
        }

        return true;
    }

    bool getGPSInfo(double& alt, double& lat, double& lon)
    {
        return meta.getGPSInfo(alt, lat, lon);
    }

    bool setGPSInfo(const double alt, const double lat, const double lon)
    {
        return meta.setGPSInfo(alt, lat, lon);
    }

    bool removeGPSInfo()
    {
        return meta.removeGPSInfo();
    }

    QString getExifTagString(const QString& tag)
    {
        return meta.getExifTagString(tag.toLatin1().constData());
    }

    bool setExifTagString(const QString& tag, const QString& val)
    {
        return meta.setExifTagString(tag.toLatin1().constData(), val);
    }

    bool getExifTagRational(const QString& tag, long int& num, long int& den)
    {
        return meta.getExifTagRational(tag.toLatin1().constData(), num, den);
    }

    bool setExifTagRational(const QString& tag, long int num, long int den)
    {
        return meta.setExifTagRational(tag.toLatin1().constData(), num, den);
    }

    QString getXmpTagString(const QString& tag)
    {
        return meta.getXmpTagString(tag.toLatin1().constData());
    }

    bool setXmpTagString(const QString& tag, const QString& val)
    {
        return meta.setXmpTagString(tag.toLatin1().constData(), val);
    }

    QStringList getXmpKeywords()
    {
        return meta.getXmpKeywords();
    }

    bool  setXmpKeywords(const QStringList& keywords)
    {
        return meta.setXmpKeywords(keywords);
    }

    QVariant getXmpTagVariant(const QString& tag)
    {
        return meta.getXmpTagVariant(tag.toLatin1().constData());
    }

private:

    DMetadata meta;
};

KIPI::MetadataProcessor* KipiInterface::createMetadataProcessor() const
{
    return new KipiInterfaceMatadataProcessor;
}

}  // namespace Digikam
