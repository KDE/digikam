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

QMap<QString,QVariant> DigikamImageInfo::attributes()
{
    QMap<QString,QVariant> res;
    
    // FIXME !
    
    /*if ( _info ) {
        for( QMapIterator<QString,QStringList> it = _info->_options.begin(); it != _info->_options.end(); ++it ) {
            res.insert( it.key(), QVariant( it.data() ) );
        }
    }*/
    
    return res;
}

void DigikamImageInfo::clearAttributes()
{
    // FIXME !
}

void DigikamImageInfo::addAttributes( const QMap<QString,QVariant>& map )
{
    // FIXME !
    
    /*if ( _info ) {
        for( QMapConstIterator<QString,QVariant> it = map.begin(); it != map.end(); ++it ) {
            QStringList list = it.data().toStringList();
            _info->addOption( it.key(), list );
        }
    }*/
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

DigikamImageCollection::DigikamImageCollection( Type tp, Digikam::AlbumInfo *album )
                      : _tp( tp )
{
    if (album)
       album_ = album;
    else
       album_ = Digikam::AlbumManager::instance()->currentAlbum();
}

DigikamImageCollection::~DigikamImageCollection()
{
}

QString DigikamImageCollection::name()
{
    if (album_) 
        {
        album_->openDB();
        QString title = album_->getTitle();
        album_->closeDB();    
        return (title);    
        }
    else
        return QString::null;
}

QString DigikamImageCollection::comment()
{
    if (album_) 
        {
        album_->openDB();
        QString comments = album_->getComments();
        album_->closeDB();        
        return (comments);
        }
    else
        return QString::null;
}

KURL::List DigikamImageCollection::images()
{
    QStringList items;

    if (album_) 
        {
        album_->openDB();
        
        switch ( _tp ) 
           {
           case AllAlbumItems:
              items = album_->getAllItemsPath();
              break;
              
           case AlbumItemsSelection:
              items = album_->getSelectedItemsPath();
              break;
              
           default:
              break;
           }
        
        album_->closeDB();        
        }
        
    if ( items.isEmpty() == true )
       return KURL::List();
    else
       return KURL::List(items);
}


KURL DigikamImageCollection::path()
{
    return commonRoot();
}

KURL DigikamImageCollection::commonRoot()
{
    KURL url;
    album_->openDB();
    url.setPath( album_->getPath() );
    album_->closeDB();        
    return url;
}

KURL DigikamImageCollection::uploadPath()
{
    return commonRoot();
}

KURL DigikamImageCollection::uploadRoot()
{
    return commonRoot();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////

DigikamKipiInterface::DigikamKipiInterface( QObject *parent, const char *name)
                     :KIPI::Interface( parent, name )
{
    albumManager_ = Digikam::AlbumManager::instance();
}

DigikamKipiInterface::~DigikamKipiInterface()
{
}

KIPI::ImageCollection DigikamKipiInterface::currentAlbum()
{
    return KIPI::ImageCollection( new DigikamImageCollection( DigikamImageCollection::AllAlbumItems ) );
}

KIPI::ImageCollection DigikamKipiInterface::currentSelection()
{
    return KIPI::ImageCollection( new DigikamImageCollection( DigikamImageCollection::AlbumItemsSelection ) );
}

QValueList<KIPI::ImageCollection> DigikamKipiInterface::allAlbums()
{
    QValueList<KIPI::ImageCollection> result;
    
    for (Digikam::AlbumInfo *album = albumManager_->firstAlbum() ;
         album ; album = album->nextAlbum())
        {
        album->openDB();
        DigikamImageCollection* col = new DigikamImageCollection( DigikamImageCollection::AllAlbumItems, album );
        result.append( KIPI::ImageCollection( col ) );
        album->closeDB();
        }

    return result;
}

KIPI::ImageInfo DigikamKipiInterface::info( const KURL& url )
{
    return KIPI::ImageInfo( new DigikamImageInfo( this, url ) );
}

void DigikamKipiInterface::refreshImages( const KURL::List& urls )
{
    albumManager_->refreshItemHandler(); 
}

int DigikamKipiInterface::features() const
{
    return KIPI::ImagesHasComments    | KIPI::AcceptNewImages |
           KIPI::AlbumEQDir           | KIPI::AlbumsHaveComments |
           KIPI::ImageTitlesWritable  | KIPI::ImagesHasTime;
}

bool DigikamKipiInterface::addImage( const KURL& url, QString& errmsg )
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

void DigikamKipiInterface::delImage( const KURL& url )
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
