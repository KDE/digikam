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

// KIPI includes.

#include <libkipi/imagecollection.h>

// Local includes.

#include "kipiinterface.h"



KipiInterface::KipiInterface( QObject *parent, const char *name )
              :KIPI::Interface( parent, name )
{
}

KIPI::ImageCollection KipiInterface::currentAlbum()
{
return 0;
//    return KIPI::ImageCollection( new MyImageCollection( MyImageCollection::CurrentAlbum ) );
}

KIPI::ImageCollection KipiInterface::currentSelection()
{
return 0;
//    return KIPI::ImageCollection( new MyImageCollection( MyImageCollection::CurrentSelection ) );
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
    return result;
}

KIPI::ImageInfo KipiInterface::info( const KURL& url )
{
return 0;    
    //return KIPI::ImageInfo( new MyImageInfo( this, url ) );
}

void KipiInterface::refreshImages( const KURL::List& urls )
{
    emit imagesChanged( urls );
}

int KipiInterface::features() const
{
/*    return KIPI::ImagesHasComments | KIPI::ImagesHasTime | KIPI::SupportsDateRanges |
           KIPI::AcceptNewImages | KIPI::ImageTitlesWritable;*/
      return 0;
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
    return true;
}

void KipiInterface::delImage( const KURL& url )
{
    /*ImageInfo* info = ImageDB::instance()->find( url.path() );
    if ( info ) {
        ImageInfoList list;
        list.append( info );
        ImageDB::instance()->deleteList( list );
    }*/
}

#include "kipiinterface.moc"

#endif  // HAVE_KIPI
