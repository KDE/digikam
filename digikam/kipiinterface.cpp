/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : digiKam kipi library interface.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C Ansi includes

extern "C"
{
#include <sys/types.h>
#include <utime.h>
}

// Qt includes.

#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QRegExp>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kfilemetainfo.h>
#include <kio/netaccess.h>

// libKipi Includes.

#include <libkipi/version.h>

// Local includes.

#include "constants.h"
#include "ddebug.h"
#include "albummanager.h"
#include "albumitemhandler.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "dmetadata.h"
#include "imageattributeswatch.h"
#include "imagelister.h"
#include "namefilter.h"
#include "kipiinterface.h"
#include "kipiinterface.moc"

namespace Digikam
{

//-- Image Info ------------------------------------------------------------------

DigikamImageInfo::DigikamImageInfo( KIPI::Interface* interface, const KUrl& url )
                : KIPI::ImageInfoShared( interface, url ), palbum_(0)
{
}

DigikamImageInfo::~DigikamImageInfo()
{
}

PAlbum* DigikamImageInfo::parentAlbum()
{
    if (!palbum_)
    {
        KUrl u(_url.directory());
        palbum_ = AlbumManager::instance()->findPAlbum(u);
    }
    return palbum_;
}

QString DigikamImageInfo::title()
{
    return _url.fileName();
}

QString DigikamImageInfo::description()
{
    PAlbum* p = parentAlbum();

    if (p)
        DatabaseAccess().db()->getItemCaption(p->id(), _url.fileName());

    return QString();
}

void DigikamImageInfo::setTitle( const QString& newName )
{
    // Here we get informed that a plugin has renamed the file 

    PAlbum* p = parentAlbum();

    if ( p && !newName.isEmpty() )
    {
        DatabaseAccess().db()->moveItem(p->id(), _url.fileName(), p->id(), newName);
        _url = _url.upUrl();
        _url.addPath(newName);
    }
}

void DigikamImageInfo::setDescription( const QString& description )
{
    PAlbum* p = parentAlbum();

    if ( p  )
    {
        qlonglong imageId;
        {
            DatabaseAccess access;
            imageId = access.db()->getImageId(p->id(), _url.fileName());
            access.db()->setItemCaption(imageId, description);
        }

        // See B.K.O #140133. Do not set here metadata comments of picture.
        // Only the database comments must be set.
    }
}

QDateTime DigikamImageInfo::time( KIPI::TimeSpec /*spec*/ )
{
    PAlbum* p = parentAlbum();

    if (p)
        DatabaseAccess().db()->getItemDate(p->id(), _url.fileName());

    return QDateTime();
}

void DigikamImageInfo::setTime(const QDateTime& time, KIPI::TimeSpec)
{
    if ( !time.isValid() )
    {
        DWarning() << "Invalid datetime specified" << endl;
        return;
    }

    PAlbum* p = parentAlbum();

    if ( p )
    {
        qlonglong imageId;
        {
            DatabaseAccess access;
            imageId = access.db()->getImageId(p->id(), _url.fileName());
            access.db()->setItemDate(imageId, time);
        }
        AlbumManager::instance()->refreshItemHandler( _url );
    }
}

void DigikamImageInfo::cloneData( ImageInfoShared* other )
{
    // TODO: Added others picture attributes stored by digiKam 
    //       database to clone an item.

    setDescription( other->description() );
    setTime( other->time(KIPI::FromInfo), KIPI::FromInfo );
}

QMap<QString, QVariant> DigikamImageInfo::attributes()
{
    QMap<QString, QVariant> res;

    PAlbum* p = parentAlbum();
    if (p)
    {
        DatabaseAccess access;
        qlonglong imageId    = access.db()->getImageId(p->id(), _url.fileName());

        /* TODO:  Marcel, a new method from AlbumDb class need to created for that.

        // Get digiKam Tags Path list of picture from database. 
        // Ex.: "City/Paris/Monuments/Notre Dame"

        QStringList tagspath = access.db()->getItemTagPath(imageId);
        res["tagspath"]      = tagspath;*/

        // Get digiKam Tags name list of picture from database.
        // Ex.: "Notre Dame"
        QStringList tags     = access.db()->getItemTagNames(imageId);
        res["tags"]          = tags;

        // Get digiKam Rating of picture from database.
        int rating           = access.db()->getItemRating(imageId);
        res["rating"]        = rating;

        // TODO: add here a kipi-plugins access to future picture attributes stored by digiKam database
    }
    return res;
}

void DigikamImageInfo::addAttributes(const QMap<QString, QVariant>& res)
{
    PAlbum* p = parentAlbum();
    if (p)
    {
        DatabaseAccess access;
        qlonglong imageId = access.db()->getImageId(p->id(), _url.fileName());
        QMap<QString, QVariant> attributes = res;

        // Set digiKam Tags Path list of picture into database. 
        // Ex.: "City/Paris/Monuments/Notre Dame"
        if (attributes.find("tagspath") != attributes.end())
        {
            /* TODO:  Marcel, a new method from AlbumDb class need to created for that.

            QStringList tagspath = attributes["tagspath"].toStringList();*/
            access.db()->setItemTagPath(imageId, tagspath);
        }

        // Set digiKam Rating of picture into database.
        if (attributes.find("rating") != attributes.end())
        {
            int rating = attributes["rating"].toInt();
            if (rating >= RatingMin && rating <= RatingMax)
                access.db()->setItemRating(imageId, rating);
        }

        // TODO: add here a kipi-plugins access to future picture attributes stored by digiKam database
    }

    // To update sidebar content. Some kipi-plugins use this way to refresh sidebar 
    // using an empty QMap(). 
    ImageAttributesWatch::instance()->fileMetadataChanged(_url);
}

void DigikamImageInfo::clearAttributes()
{
    // TODO: implemente me.
}

int DigikamImageInfo::angle()
{
    AlbumSettings *settings = AlbumSettings::instance();
    if (settings->getExifRotate())
    {
        DMetadata metadata(_url.path());
        DMetadata::ImageOrientation orientation = metadata.getImageOrientation();
        
        switch (orientation) 
        {
            case DMetadata::ORIENTATION_ROT_180:
                return 180;
            case DMetadata::ORIENTATION_ROT_90:
            case DMetadata::ORIENTATION_ROT_90_HFLIP:
            case DMetadata::ORIENTATION_ROT_90_VFLIP:
                return 90;
            case DMetadata::ORIENTATION_ROT_270:
                return 270;
            default:
                return 0;
        }
    }
    
    return 0;
}

void DigikamImageInfo::setAngle(int /*angle*/)
{
    // TODO: set digiKam database with this information.
}

//-- Image Collection ------------------------------------------------------------

DigikamImageCollection::DigikamImageCollection( Type tp, Album* album, const QString& filter )
                      : tp_( tp ), album_(album), imgFilter_(filter)
{
    if (!album)
    {
        DWarning() << "This should not happen. No album specified" << endl;
    }
}

DigikamImageCollection::~DigikamImageCollection()
{
}

QString DigikamImageCollection::name()
{
    if ( album_->type() == Album::TAG )
    {
        return i18n("Tag: %1",album_->title());
    }
    else
        return album_->title();
}

QString DigikamImageCollection::category()
{
    if ( album_->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        return p->collection();
    }
    else if ( album_->type() == Album::TAG )
    {
        TAlbum *p = dynamic_cast<TAlbum*>(album_);
        return i18n("Tag: %1",p->tagPath());
    }
    else
        return QString();
}

QDate DigikamImageCollection::date()
{
    if ( album_->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        return p->date();
    }
    else
        return QDate();
}

QString DigikamImageCollection::comment()
{
    if ( album_->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        return p->caption();
    }
    else
        return QString();
}

KUrl::List DigikamImageCollection::images()
{
    switch ( tp_ )
    {
        case AllItems:
        {
            if (album_->type() == Album::PHYSICAL)
            {
                return imagesFromPAlbum(dynamic_cast<PAlbum*>(album_));
            }
            else if (album_->type() == Album::TAG)
            {
                return imagesFromTAlbum(dynamic_cast<TAlbum*>(album_));
            }
            else if (album_->type() == Album::DATE || 
                    album_->type() == Album::SEARCH)
            {
                AlbumItemHandler* handler = AlbumManager::instance()->getItemHandler();
    
                if (handler)
                {
                    return handler->allItems();
                }
    
                return KUrl::List();
        }
            else
            {
                DWarning() << "Unknown album type" << endl;
                return KUrl::List();
            }
    
            break;
        }
        case SelectedItems:
        {
            AlbumItemHandler* handler = AlbumManager::instance()->getItemHandler();
    
            if (handler)
            {
                return handler->selectedItems();
            }
    
            return KUrl::List();
    
            break;
        }
        default:
            break;
    }

    // We should never reach here
    return KUrl::List();
}

/** get the images from the Physical album in database and return the items found */

KUrl::List DigikamImageCollection::imagesFromPAlbum(PAlbum* album) const
{
    // get the images from the database and return the items found

    AlbumDB::ItemSortOrder sortOrder = AlbumDB::NoItemSorting;
    switch (AlbumSettings::instance()->getImageSortOrder())
    {
        default:
        case AlbumSettings::ByIName:
            sortOrder = AlbumDB::ByItemName;
            break;
        case AlbumSettings::ByIPath:
            sortOrder = AlbumDB::ByItemPath;
            break;
        case AlbumSettings::ByIDate:
            sortOrder = AlbumDB::ByItemDate;
            break;
        case AlbumSettings::ByIRating:
            sortOrder = AlbumDB::ByItemRating;
            break;
        // ByISize not supported
    }

    QStringList urls = DatabaseAccess().db()->getItemURLsInAlbum(album->id(), sortOrder);

    KUrl::List urlList;

    NameFilter nameFilter(imgFilter_);

    for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
    {
        if (nameFilter.matches(*it))
            urlList.append(*it);
    }

    return urlList;
}

/** get the images from the Tags album in database and return the items found */

KUrl::List DigikamImageCollection::imagesFromTAlbum(TAlbum* album) const
{
    QStringList urls;
    urls = DatabaseAccess().db()->getItemURLsInTag(album->id());

    KUrl::List urlList;

    NameFilter nameFilter(imgFilter_);

    for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
    {
        if (nameFilter.matches(*it))
            urlList.append(*it);
    }

    return urlList;
}

KUrl DigikamImageCollection::path()
{
    if (album_->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        KUrl url;
        url.setPath(p->folderPath());
        return url;
    }
    else
    {
        DWarning() << "Requesting kurl from a virtual album" << endl;
        return QString();
    }
}

KUrl DigikamImageCollection::uploadPath()
{
    if (album_->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        KUrl url;
        url.setPath(p->folderPath());
        return url;
    }
    else
    {
        DWarning() << "Requesting kurl from a virtual album" << endl;
        return KUrl();
    }
}

KUrl DigikamImageCollection::uploadRoot()
{
    return KUrl(CollectionManager::instance()->oneAlbumRootPath() + '/');
}

QString DigikamImageCollection::uploadRootName()
{
    return i18n("My Albums");
}

bool DigikamImageCollection::isDirectory()
{
    if (album_->type() == Album::PHYSICAL)
        return true;
    else
        return false;
}

bool DigikamImageCollection::operator==(ImageCollectionShared& imgCollection)
{
    DigikamImageCollection* thatCollection = static_cast<DigikamImageCollection*>(&imgCollection);
    return (album_ == thatCollection->album_);
}

//-- LibKipi interface -----------------------------------------------------------

DigikamKipiInterface::DigikamKipiInterface( QObject *parent, const char *name)
                    : KIPI::Interface( parent, name )
{
    albumManager_ = AlbumManager::instance();

    connect( albumManager_, SIGNAL( signalAlbumItemsSelected( bool ) ),
             this, SLOT( slotSelectionChanged( bool ) ) );

    connect( albumManager_, SIGNAL( signalAlbumCurrentChanged(Album*) ),
             this, SLOT( slotCurrentAlbumChanged(Album*) ) );
}

DigikamKipiInterface::~DigikamKipiInterface()
{
}

KIPI::ImageCollection DigikamKipiInterface::currentAlbum()
{
    Album* currAlbum = albumManager_->currentAlbum();
    if ( currAlbum )
    {
        return KIPI::ImageCollection(
            new DigikamImageCollection( DigikamImageCollection::AllItems,
                                        currAlbum, fileExtensions() ) );
    }
    else
    {
        return KIPI::ImageCollection(0);
    }
}

KIPI::ImageCollection DigikamKipiInterface::currentSelection()
{
    Album* currAlbum = albumManager_->currentAlbum();
    if ( currAlbum )
    {
        return KIPI::ImageCollection(
            new DigikamImageCollection( DigikamImageCollection::SelectedItems,
                                        currAlbum, fileExtensions() ) );
    }
    else
    {
        return KIPI::ImageCollection(0);
    }
}

QList<KIPI::ImageCollection> DigikamKipiInterface::allAlbums()
{
    QList<KIPI::ImageCollection> result;

    QString fileFilter(fileExtensions());

    AlbumList palbumList = albumManager_->allPAlbums();
    for ( AlbumList::Iterator it = palbumList.begin();
          it != palbumList.end(); ++it )
    {
        // don't add the root album
        if ((*it)->isRoot())
            continue;

        DigikamImageCollection* col = new DigikamImageCollection( DigikamImageCollection::AllItems,
                                                                  *it, fileFilter );
        result.append( KIPI::ImageCollection( col ) );
    }

    AlbumList talbumList = albumManager_->allTAlbums();
    for ( AlbumList::Iterator it = talbumList.begin();
          it != talbumList.end(); ++it )
    {
        // don't add the root album
        if ((*it)->isRoot())
            continue;

        DigikamImageCollection* col = new DigikamImageCollection( DigikamImageCollection::AllItems,
                                                                  *it, fileFilter );   
        result.append( KIPI::ImageCollection( col ) );
    }

    return result;
}

KIPI::ImageInfo DigikamKipiInterface::info( const KUrl& url )
{
    return KIPI::ImageInfo( new DigikamImageInfo( this, url ) );
}

void DigikamKipiInterface::refreshImages( const KUrl::List& urls )
{
    KUrl::List ulist = urls;

    // Re-scan metadata from pictures. This way will update Metadata sidebar and database.
    for ( KUrl::List::Iterator it = ulist.begin() ; it != ulist.end() ; ++it )
        ImageAttributesWatch::instance()->fileMetadataChanged(*it);
    
    // Refresh preview.
    albumManager_->refreshItemHandler(urls);
}

int DigikamKipiInterface::features() const
{
    return (

           KIPI::HostSupportsTags |
           KIPI::ImagesHasComments          | KIPI::AcceptNewImages            |
           KIPI::AlbumsHaveComments         | KIPI::ImageTitlesWritable        |
           KIPI::ImagesHasTime              | KIPI::AlbumsHaveCategory         |
           KIPI::AlbumsHaveCreationDate     | KIPI::AlbumsUseFirstImagePreview 
           );
}

bool DigikamKipiInterface::addImage( const KUrl& url, QString& errmsg )
{
    // Nota : All copy/move operations are processed by the plugins.

    if ( url.isValid() == false )
    {
        errmsg = i18n("Target URL %1 is not valid.",url.path());
        return false;
    }

    PAlbum *targetAlbum = albumManager_->findPAlbum(url.directory());

    if ( !targetAlbum )
    {
        errmsg = i18n("Target album is not in the album library.");
        return false;
    }

    albumManager_->refreshItemHandler( url );

    return true;
}

void DigikamKipiInterface::delImage( const KUrl& url )
{
    KUrl rootURL(CollectionManager::instance()->albumRoot(url));
    if ( !rootURL.isParentOf(url) )
    {
        DWarning() << "URL not in the album library" << endl;
    }

    // Is there a PAlbum for this url

    PAlbum *palbum = albumManager_->findPAlbum( KUrl(url.directory()) );

    if ( palbum )
    {
        // delete the item from the database
        DatabaseAccess().db()->deleteItem( palbum->id(), url.fileName() );
    }
    else
    {
        DWarning() << "Cannot find Parent album in album library" << endl;
    }
}

void DigikamKipiInterface::slotSelectionChanged( bool b )
{
    emit selectionChanged( b );
}

void DigikamKipiInterface::slotCurrentAlbumChanged( Album *album )
{
    emit currentAlbumChanged( album != 0 );
}

QString DigikamKipiInterface::fileExtensions()
{
    // do not save this into a local variable, as this
    // might change in the main app

    AlbumSettings* s = AlbumSettings::instance();
    return (s->getImageFileFilter() + ' ' +
            s->getMovieFileFilter() + ' ' +
            s->getAudioFileFilter() + ' ' +
            s->getRawFileFilter());
}

}  // namespace Digikam
