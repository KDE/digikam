////////////////////////////////////////////////////////////////////////////////
//
//    KIPIINTERFACE.CPP
//
//    Copyright (C) 2004 Gilles Caulier <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_KIPI

// KDE includes.

#include <klocale.h>

// Local includes.

#include "albummanager.h"
#include "albuminfo.h"
#include "digikamio.h"
#include "kipiinterface.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////

DigikamImageInfo::DigikamImageInfo( KIPI::Interface* interface, const KURL& url )
                : KIPI::ImageInfoShared( interface, url )
{
    imageName_ = url.fileName();
    albumName_ = url.path().section('/', -2, -2);
    
    if (albumName_.isEmpty() == false && imageName_.isEmpty() == false)
       {
       album_ = Digikam::AlbumManager::instance()->findAlbum(albumName_);
       
       if (album_)
          {
          album_->openDB();
          imageComments_ = album_->getItemComments(imageName_);
          album_->closeDB();    
          }
       } 
}

DigikamImageInfo::~DigikamImageInfo()
{
}

QString DigikamImageInfo::title()
{
    return imageName_;
}

QString DigikamImageInfo::description()
{
    return imageComments_;
}

QMap<QString,QVariant> DigikamImageInfo::attributes()
{
    QMap<QString,QVariant> res;   // FIXME !
    /*if ( _info ) {
        for( QMapIterator<QString,QStringList> it = _info->_options.begin(); it != _info->_options.end(); ++it ) {
            res.insert( it.key(), QVariant( it.data() ) );
        }
    }*/
    return res;
}

void DigikamImageInfo::setTitle( const QString& name )
{
    if (album_ && name.isEmpty() == false)
       {
       DigikamIO::rename(album_, imageName_, name);
       }
}

void DigikamImageInfo::setDescription( const QString& description )
{
    if (album_)
       {
       album_->openDB();
       album_->setItemComments(imageName_, description);
       album_->closeDB();    
       }
}

void DigikamImageInfo::clearAttributes()
{
    // FIXME !
}

void DigikamImageInfo::addAttributes( const QMap<QString,QVariant>& map )
{
    // FIXME !
}

int DigikamImageInfo::angle()
{
    return 0; // FIXME !
}

void DigikamImageInfo::setAngle( int angle )
{
    // FIXME !
}

QDateTime DigikamImageInfo::time( KIPI::TimeSpec what )
{
    return KIPI::ImageInfoShared::time( what );    // FIXME !
}

bool DigikamImageInfo::isTimeExact()
{
    return true;   // FIXME !
}

void DigikamImageInfo::setTime( const QDateTime& time, KIPI::TimeSpec spec )
{
    // FIXME !
}

void DigikamImageInfo::cloneData( ImageInfoShared* other )
{
    ImageInfoShared::cloneData( other );   // FIXME !
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

DigikamImageCollection::DigikamImageCollection( Type tp )
                      : _tp( tp )
{
}

DigikamImageCollection::~DigikamImageCollection()
{
}

QString DigikamImageCollection::name()
{
    Digikam::AlbumInfo *currentAlbum = Digikam::AlbumManager::instance()->currentAlbum();
    
    if (currentAlbum) 
        {
        currentAlbum->openDB();
        QString title = currentAlbum->getTitle();
        currentAlbum->closeDB();    
        return (title);    
        }
    else
        return QString::null;
}

QString DigikamImageCollection::comment()
{
    Digikam::AlbumInfo *currentAlbum = Digikam::AlbumManager::instance()->currentAlbum();
    
    if (currentAlbum) 
        {
        currentAlbum->openDB();
        QString comments = currentAlbum->getComments();
        currentAlbum->closeDB();        
        return (comments);
        }
    else
        return QString::null;
}

KURL::List DigikamImageCollection::images()
{
    Digikam::AlbumInfo *currentAlbum = Digikam::AlbumManager::instance()->currentAlbum();
    
    if (currentAlbum) 
        {
        switch ( _tp ) 
           {
           case CurrentAlbum:
              return KURL::List( currentAlbum->getAllItems() );

           case CurrentSelection:
              return KURL::List( currentAlbum->getSelectedItems() );

           case SubClass:
              qFatal( "The subclass should implement images()" );   // FIXME !!!
              return KURL::List();
           }
        }
        
    return KURL::List();
}


KURL DigikamImageCollection::path()
{
    return commonRoot();
}

KURL DigikamImageCollection::commonRoot()
{
    KURL url;
    url.setPath( Digikam::AlbumManager::instance()->getLibraryPath() );
    return url;
}

KURL DigikamImageCollection::uploadPath()
{
    return commonRoot();
}

KURL DigikamImageCollection::uploadRoot()
{
    KURL url;
    url.setPath( Digikam::AlbumManager::instance()->getLibraryPath() );
    return url;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

KipiInterface::KipiInterface( QObject *parent, const char *name)
              :KIPI::Interface( parent, name )
{
}

KipiInterface::~KipiInterface()
{
}

KIPI::ImageCollection KipiInterface::currentAlbum()
{
    return KIPI::ImageCollection( new DigikamImageCollection( DigikamImageCollection::CurrentAlbum ) );
}

KIPI::ImageCollection KipiInterface::currentSelection()
{
    return KIPI::ImageCollection( new DigikamImageCollection( DigikamImageCollection::CurrentSelection ) );
}

QValueList<KIPI::ImageCollection> KipiInterface::allAlbums()
{
    QValueList<KIPI::ImageCollection> result;
    /*ImageSearchInfo context = MainView::theMainView()->currentContext();
    QString optionGroup = MainView::theMainView()->currentBrowseCategory();
    if ( optionGroup.isNull() )
        optionGroup = Options::instance()->albumCategory();

    QMap<QString,int> categories = ImageDB::instance()->classify( context, optionGroup );

    for( QMapIterator<QString,int> it = categories.begin(); it != categories.end(); ++it ) {
        CategoryImageCollection* col = new CategoryImageCollection( context, optionGroup, it.key() );
        result.append( KIPI::ImageCollection( col ) );
    }
*/

    for (Digikam::AlbumInfo *album=Digikam::AlbumManager::instance()->firstAlbum() ;
         album ; album = album->nextAlbum())
        {
        album->openDB();
//        result.append( KIPI::ImageCollection( col ) );
        album->closeDB();
        }

    return result;
}

KIPI::ImageInfo KipiInterface::info( const KURL& url )
{
    return KIPI::ImageInfo( new DigikamImageInfo( this, url ) );
}

void KipiInterface::refreshImages( const KURL::List& urls )
{
    emit imagesChanged( urls );
}

int KipiInterface::features() const
{
    return KIPI::ImagesHasComments | KIPI::ImagesHasTime | KIPI::SupportsDateRanges |
           KIPI::AcceptNewImages   | KIPI::AlbumEQDir    | KIPI::AlbumsHaveComments;
}

bool KipiInterface::addImage( const KURL& url, QString& errmsg )
{
    /*QString dir = url.path();
    QString root = Options::instance()->imageDirectory();
    if ( !dir.startsWith( root ) ) {
        errmsg = i18n("<qt>Image needs to be placed in a sub directory of the KimDaBa image database, "
                      "which is rooted at %1. Image path was %2</qt>").arg( root ).arg( dir );
        return false;
    }

    dir = dir.mid( root.length() );
    ImageInfo* info = new ImageInfo( dir );
    ImageDB::instance()->addImage( info );*/
    return true;      // FIXME !
}

void KipiInterface::delImage( const KURL& url )
{
    /*ImageInfo* info = ImageDB::instance()->find( url.path() );
    if ( info ) {
        ImageInfoList list;
        list.append( info );
        ImageDB::instance()->deleteList( list );
    }*/
    
    // FIXME !
}

#include "kipiinterface.moc"

#endif  // HAVE_KIPI
