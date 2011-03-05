/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to interface digiKam with kipi library.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>

// Qt includes

#include <QAbstractItemModel>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "metadatasettings.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "digikamview.h"
#include "loadingcacheinterface.h"
#include "scancontroller.h"
#include "imageattributeswatch.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "kipiimageinfo.h"
#include "kipiimagecollection.h"

namespace Digikam
{

class KipiInterface::KipiInterfacePrivate
{
public:

    KipiInterfacePrivate()
    {
        tagModel        = 0;
        thumbLoadThread = 0;
        albumManager    = 0;
    }

    AlbumManager*        albumManager;
    ThumbnailLoadThread* thumbLoadThread;
    QAbstractItemModel*  tagModel;
};

KipiInterface::KipiInterface(QObject* parent, const char* name)
    : KIPI::Interface(parent, name), d(new KipiInterfacePrivate())
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();
    d->albumManager    = AlbumManager::instance();

    connect(DigikamApp::instance()->view(), SIGNAL(signalSelectionChanged(int)),
            this, SLOT(slotSelectionChanged(int)));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));
}

KipiInterface::~KipiInterface()
{
}

KIPI::ImageCollection KipiInterface::currentAlbum()
{
    Album* currAlbum = d->albumManager->currentAlbum();

    if ( currAlbum )
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
    Album* currAlbum = d->albumManager->currentAlbum();

    if ( currAlbum )
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

    for ( AlbumList::ConstIterator it = palbumList.constBegin();
          it != palbumList.constEnd(); ++it )
    {
        // don't add the root album
        if ((*it)->isRoot())
        {
            continue;
        }

        KipiImageCollection* col = new KipiImageCollection(KipiImageCollection::AllItems, *it, fileFilter);
        result.append( KIPI::ImageCollection( col ) );
    }

    const AlbumList talbumList = d->albumManager->allTAlbums();

    for ( AlbumList::ConstIterator it = talbumList.constBegin();
          it != talbumList.constEnd(); ++it )
    {
        // don't add the root album
        if ((*it)->isRoot())
        {
            continue;
        }

        KipiImageCollection* col = new KipiImageCollection(KipiImageCollection::AllItems,
                *it, fileFilter);
        result.append( KIPI::ImageCollection( col ) );
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
    QSet<QString> dirs;
    QList<qlonglong> ids;
    foreach (const KUrl& url, urls)
    {
        ImageInfo info(url);

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
    foreach (const QString& dir, dirs)
    {
        ScanController::instance()->scheduleCollectionScan(dir);
    }

}

int KipiInterface::features() const
{
    return(
              KIPI::HostSupportsTags            | KIPI::HostSupportsRating      |
              KIPI::HostAcceptNewImages         | KIPI::HostSupportsThumbnails  |
              KIPI::HostSupportsProgressBar     |
              KIPI::ImagesHasComments           | KIPI::ImagesHasTitlesWritable |
              KIPI::ImagesHasTime               |
              KIPI::CollectionsHaveComments     | KIPI::CollectionsHaveCategory |
              KIPI::CollectionsHaveCreationDate
          );
}

bool KipiInterface::addImage( const KUrl& url, QString& errmsg )
{
    // Note : All copy/move operations are processed by the plugins.

    if ( url.isValid() == false )
    {
        errmsg = i18n("Target URL %1 is not valid.",url.toLocalFile());
        return false;
    }

    PAlbum* targetAlbum = d->albumManager->findPAlbum(url.directory());

    if ( !targetAlbum )
    {
        errmsg = i18n("Target album is not in the album library.");
        return false;
    }

    //d->albumManager->refreshItemHandler( url );

    return true;
}

void KipiInterface::delImage( const KUrl& url )
{
    KUrl rootURL(CollectionManager::instance()->albumRoot(url));

    if ( !rootURL.isParentOf(url) )
    {
        kWarning() << "URL not in the album library";
    }

    // Is there a PAlbum for this URL

    PAlbum* palbum = d->albumManager->findPAlbum( KUrl(url.directory()) );

    if ( palbum )
    {
        // delete the item from the database
        DatabaseAccess().db()->deleteItem( palbum->id(), url.fileName() );
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

void KipiInterface::slotCurrentAlbumChanged( Album* album )
{
    emit currentAlbumChanged( album != 0 );
}

void KipiInterface::thumbnail(const KUrl& url, int /*size*/)
{
    // NOTE: size is not used here. Cache use the max pixmap size to store thumbs (256).
    d->thumbLoadThread->find(url.toLocalFile());
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
    emit gotThumbnail( KUrl(desc.filePath), pix );
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
        d->tagModel = newTagModel;
    }

    return d->tagModel;
}

QVariant KipiInterface::hostSetting(const QString& settingName)
{
    MetadataSettings* mSettings = MetadataSettings::instance();

    if (!mSettings)
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
        // do not save this into a local variable, as this
        // might change in the main app

        AlbumSettings* s = AlbumSettings::instance();
        return (s->getImageFileFilter() + ' ' +
                s->getMovieFileFilter() + ' ' +
                s->getAudioFileFilter() + ' ' +
                s->getRawFileFilter());
    }

    return QVariant();
}

}  // namespace Digikam
