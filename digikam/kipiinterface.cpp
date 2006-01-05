/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at free.fr>
 *          Ralf Holzer <ralf at well.com>
 *          Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date   : 2004-08-02
 * Description :
 *
 * Copyright 2004-2006 by Gilles Caulier
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// C Ansi includes

extern "C"
{
#include <sys/types.h>
#include <utime.h>
}

// Qt includes.

#include <qdir.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qregexp.h>

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kfilemetainfo.h>
#include <kio/netaccess.h>
#include <kdebug.h>

// LibKEXIF includes.

#include <libkexif/kexifutils.h>
#include <libkexif/kexifdata.h>

// Local includes.

#include "albummanager.h"
#include "albumitemhandler.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "kipiinterface.h"

namespace Digikam
{

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// IMAGE INFO IMPLEMENTATION CLASS ////////////////////////////////////////

DigikamImageInfo::DigikamImageInfo( KIPI::Interface* interface, const KURL& url )
    : KIPI::ImageInfoShared( interface, url ),
      palbum_(0)
{
}

DigikamImageInfo::~DigikamImageInfo()
{
}

PAlbum* DigikamImageInfo::parentAlbum()
{
    if (!palbum_)
    {
        KURL u(_url.directory());
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
    {
        AlbumDB* db = AlbumManager::instance()->albumDB();
        return db->getItemCaption(p->id(), _url.fileName());
    }

    return QString::null;
}

void DigikamImageInfo::setTitle( const QString& newName )
{
    //  here we get informed that a plugin has renamed the file 
    
    PAlbum* p = parentAlbum();

    if ( p && !newName.isEmpty() )
    {
        AlbumDB* db = AlbumManager::instance()->albumDB();
        db->moveItem(p->id(), _url.fileName(), p->id(), newName);
        _url = _url.upURL();
        _url.addPath(newName);
    }
}

void DigikamImageInfo::setDescription( const QString& description )
{
    PAlbum* p = parentAlbum();

    if ( p  )
    {
        AlbumDB* db = AlbumManager::instance()->albumDB();
        db->setItemCaption(p->id(), _url.fileName(), description);

        AlbumSettings *settings = AlbumSettings::instance();
        if (settings->getSaveExifComments())
        {
            KFileMetaInfo metaInfo(_url.path(), "image/jpeg", KFileMetaInfo::Fastest);
            if (metaInfo.isValid () && metaInfo.mimeType() == "image/jpeg")
            {
               // store as JPEG JFIF comment
                if (metaInfo.containsGroup("Jpeg EXIF Data"))
                {
                    metaInfo["Jpeg EXIF Data"].item("Comment").setValue(description);
                    metaInfo.applyChanges();
                }
            }
        }
    }
}

QDateTime DigikamImageInfo::time( KIPI::TimeSpec /*spec*/ )
{
    PAlbum* p = parentAlbum();

    if (p)
    {
        AlbumDB* db = AlbumManager::instance()->albumDB();
        return db->getItemDate(p->id(), _url.fileName());
    }

    return QDateTime();
}

void DigikamImageInfo::setTime(const QDateTime& time, KIPI::TimeSpec)
{
    if ( !time.isValid() )
    {
        kdWarning() << k_funcinfo
                    << "Invalid datetime specified"
                    << endl;
        return;
    }

    PAlbum* p = parentAlbum();

    if ( p )
    {
        AlbumDB* db = AlbumManager::instance()->albumDB();
        db->setItemDate(p->id(), _url.fileName(), time);
        AlbumManager::instance()->refreshItemHandler( _url );
    }
}

void DigikamImageInfo::cloneData( ImageInfoShared* other )
{
    // PENDING (Gilles) : Added new Image data to clone who are provide by the
    // new Renchi implementation for Digikam 0.7.0...

    setDescription( other->description() );
    setTime( other->time(KIPI::FromInfo), KIPI::FromInfo );
}

QMap<QString,QVariant> DigikamImageInfo::attributes()
{
    QMap<QString,QVariant> res;

    // TODO ! This will used for the futures tags Digikam features.

    return res;
}

void DigikamImageInfo::clearAttributes()
{
    // TODO ! This will used for the futures tags Digikam features.
}

void DigikamImageInfo::addAttributes( const QMap<QString,QVariant>& )
{
    // TODO ! This will used for the futures tags Digikam features.
}

int DigikamImageInfo::angle()
{
    AlbumSettings *settings = AlbumSettings::instance();
    if (settings->getExifRotate())
    {
        KExifData exifData;
        
        if (exifData.readFromFile(_url.path()))
        {
            KExifData::ImageOrientation orientation = exifData.getImageOrientation();

            switch (orientation) {
            case KExifData::ROT_180:
                return 180;
            case KExifData::ROT_90:
            case KExifData::ROT_90_HFLIP:
            case KExifData::ROT_90_VFLIP:
                return 90;
            case KExifData::ROT_270:
                return 270;
            default:
                return 0;
            }
        }
    }
    
    return 0;
}

void DigikamImageInfo::setAngle( int )
{
    // TODO ! This will a libKExif implementation call ?
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// IMAGE COLLECTION IMPLEMENTATION CLASS ////////////////////////////////////

DigikamImageCollection::DigikamImageCollection( Type tp, Album* album,
                                                const QString& filter )
    : tp_( tp ), album_(album), imgFilter_(filter)
{
    if (!album)
    {
        kdWarning() << k_funcinfo
                    << "kipiinterface::DigikamImageCollection::DigikamImageCollection:"
                       "This should not happen. No album specified"
                    << endl;
    }
}

DigikamImageCollection::~DigikamImageCollection()
{
}

QString DigikamImageCollection::name()
{
    if ( album_->type() == Album::TAG )
    {
        return i18n("Tag: %1").arg(album_->title());
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
        return i18n("Tag: %1").arg(p->url());
    }
    else
        return QString::null;
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
        return QString::null;
}

static QValueList<QRegExp> makeFilterList( const QString &filter )
{
    QValueList<QRegExp> regExps;
    if ( filter.isEmpty() )
        return regExps;

    QChar sep( ';' );
    int i = filter.find( sep, 0 );
    if ( i == -1 && filter.find( ' ', 0 ) != -1 )
        sep = QChar( ' ' );

    QStringList list = QStringList::split( sep, filter );
    QStringList::Iterator it = list.begin();
    while ( it != list.end() ) {
        regExps << QRegExp( (*it).stripWhiteSpace(), false, true );
        ++it;
    }
    return regExps;
}

static bool matchFilterList( const QValueList<QRegExp>& filters,
                             const QString &fileName )
{
    QValueList<QRegExp>::ConstIterator rit = filters.begin();
    while ( rit != filters.end() ) {
        if ( (*rit).exactMatch(fileName) )
            return true;
        ++rit;
    }
    return false;
}

KURL::List DigikamImageCollection::images()
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
            AlbumItemHandler* handler =
                    AlbumManager::instance()->getItemHandler();

            if (handler)
            {
                return handler->allItems();
            }

            return KURL::List();
       }
        else
        {
            kdWarning() << k_funcinfo
                        << "kipiinterface::DigikamImageCollection::images:Unknown album type"
                        << endl;
            return KURL::List();
        }

        break;
    }
    case SelectedItems:
    {
        AlbumItemHandler* handler =
            AlbumManager::instance()->getItemHandler();

        if (handler)
        {
            return handler->selectedItems();
        }

        return KURL::List();

        break;
    }
    default:
        break;
    }

    // we should never reach here
    return KURL::List();
}

KURL::List DigikamImageCollection::imagesFromPAlbum(PAlbum* album) const
{
    // get the images from the database and return the items found

    AlbumDB* db = AlbumManager::instance()->albumDB();

    QStringList     urls;

    db->beginTransaction();

    urls = db->getItemURLsInAlbum(album->id());

    db->commitTransaction();

    KURL::List urlList;

    QValueList<QRegExp> regex = makeFilterList(imgFilter_);
    
    for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
    {
        if (matchFilterList(regex, *it))
            urlList.append(*it);
    }

    return urlList;
}

KURL::List DigikamImageCollection::imagesFromTAlbum(TAlbum* album) const
{
    // get the images from the database and return the items found

    AlbumDB* db = AlbumManager::instance()->albumDB();

    QStringList     urls;

    db->beginTransaction();

    urls = db->getItemURLsInTag(album->id());

    db->commitTransaction();

    KURL::List urlList;

    QValueList<QRegExp> regex = makeFilterList(imgFilter_);
    
    for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
    {
        if (matchFilterList(regex, *it))
            urlList.append(*it);
    }

    return urlList;
}


KURL DigikamImageCollection::path()
{
    if (album_->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        KURL url;
        url.setPath(p->folderPath());
        return url;
    }
    else
    {
        kdWarning() << k_funcinfo
                    << "kipiinterface::DigikamImageCollection::path:Requesting kurl "
                       "from a virtual album"
                    << endl;
        return QString();
    }
}

KURL DigikamImageCollection::uploadPath()
{
    if (album_->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        KURL url;
        url.setPath(p->folderPath());
        return url;
    }
    else
    {
        kdWarning() << k_funcinfo
                    << "kipiinterface::DigikamImageCollection::uploadPath:Requesting kurl "
                       "from a virtual album"
                    << endl;
        return KURL();
    }
}


KURL DigikamImageCollection::uploadRoot()
{
    return KURL(AlbumManager::instance()->getLibraryPath() + "/");
}

QString DigikamImageCollection::uploadRootName()
{
    return i18n("My Albums");
}

bool DigikamImageCollection::isDirectory()
{
    if (album_->type() == Album::PHYSICAL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DigikamImageCollection::operator==(ImageCollectionShared& imgCollection)
{
    DigikamImageCollection* thatCollection =
        static_cast<DigikamImageCollection*>(&imgCollection);

    return (album_ == thatCollection->album_);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// KIPI INTERFACE IMPLEMENTATION CLASS //////////////////////////////////////////

DigikamKipiInterface::DigikamKipiInterface( QObject *parent, const char *name)
                     : KIPI::Interface( parent, name )
{
    albumManager_ = AlbumManager::instance();
    albumDB_      = albumManager_->albumDB();

    connect( albumManager_, SIGNAL( signalAlbumItemsSelected( bool ) ),
             SLOT( slotSelectionChanged( bool ) ) );

    connect( albumManager_, SIGNAL( signalAlbumCurrentChanged(Album*) ),
             SLOT( slotCurrentAlbumChanged(Album*) ) );
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

QValueList<KIPI::ImageCollection> DigikamKipiInterface::allAlbums()
{
    QValueList<KIPI::ImageCollection> result;

    QString fileFilter(fileExtensions());

    AlbumList palbumList = albumManager_->allPAlbums();
    for ( AlbumList::Iterator it = palbumList.begin();
          it != palbumList.end(); ++it )
    {
        // don't add the root album
        if ((*it)->isRoot())
            continue;

        DigikamImageCollection* col =
            new DigikamImageCollection( DigikamImageCollection::AllItems,
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

        DigikamImageCollection* col =
            new DigikamImageCollection( DigikamImageCollection::AllItems,
                                        *it, fileFilter );
        result.append( KIPI::ImageCollection( col ) );
    }

    return result;
}

KIPI::ImageInfo DigikamKipiInterface::info( const KURL& url )
{
    return KIPI::ImageInfo( new DigikamImageInfo( this, url ) );
}

void DigikamKipiInterface::refreshImages( const KURL::List& urls )
{

    albumManager_->refreshItemHandler(urls);
}

int DigikamKipiInterface::features() const
{
    return KIPI::ImagesHasComments          | KIPI::AcceptNewImages        |
           KIPI::AlbumsHaveComments         | KIPI::ImageTitlesWritable    |
	   KIPI::ImagesHasTime              | KIPI::AlbumsHaveCategory     |
	   KIPI::AlbumsHaveCreationDate     | KIPI::AlbumsUseFirstImagePreview;
}

bool DigikamKipiInterface::addImage( const KURL& url, QString& errmsg )
{
    // Nota : All copy/move operations are processed by the plugins.

    if ( url.isValid() == false )
    {
        errmsg = i18n("Target URL %1 is not valid.").arg(url.path());
        return false;
    }

    PAlbum *targetAlbum = albumManager_->findPAlbum(url.directory());

    if ( !targetAlbum )
    {
        errmsg = i18n("Target album is not in the albums library.");
        return false;
    }

    albumManager_->refreshItemHandler( url );

    return true;
}

void DigikamKipiInterface::delImage( const KURL& url )
{
    KURL rootURL(albumManager_->getLibraryPath());
    if ( !rootURL.isParentOf(url) )
    {
        kdWarning() << k_funcinfo
                    << "kipiinterface::DigikamKipiInterface::delImage:URL not in the Digikam Album library"
                    << endl;
    }

    // Is there a PAlbum for this url

    PAlbum *palbum = albumManager_->findPAlbum( KURL(url.directory()) );

    if ( palbum )
    {
        // delete the item from the database
        albumDB_->deleteItem( palbum->id(), url.fileName() );
    }
    else
    {
        kdWarning() << k_funcinfo
                    << "kipiinterface::DigikamKipiInterface::delImage:Cannot find Parent album "
                       "in Digikam Album library"
                    << endl;
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
    return (s->getImageFileFilter() + " " +
            s->getMovieFileFilter() + " " +
            s->getAudioFileFilter() + " " +
            s->getRawFileFilter());
}

}  // namespace Digikam

#include "kipiinterface.moc"

