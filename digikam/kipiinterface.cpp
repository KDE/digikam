////////////////////////////////////////////////////////////////////////////////
//
//    KIPIINTERFACE.CPP
//
//    Copyright (C) 2004 Gilles Caulier <caulier dot gilles at free.fr>
//                       Ralf Holzer <ralf at well.com>
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

// KDE includes.

#include <klocale.h>
#include <kconfig.h>
#include <kfilemetainfo.h>
#include <kio/netaccess.h>
#include <kdebug.h>

// LibKEXIF includes.

#include <libkexif/kexifutils.h>

// Local includes.

#include "albummanager.h"
#include "albumitemhandler.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "kipiinterface.h"


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
        return db->getItemCaption(p, _url.fileName());
    }

    return QString::null;
}

void DigikamImageInfo::setTitle( const QString& name )
{
    // TODO: what is this supposed to do exactly?
    
    PAlbum* p = parentAlbum(); 

    if ( p && !name.isEmpty() )
    {
        AlbumDB* db = AlbumManager::instance()->albumDB();
        db->moveItem(p, _url.fileName(), p, name);
    }
}

void DigikamImageInfo::setDescription( const QString& description )
{
    PAlbum* p = parentAlbum(); 

    if ( p  )
    {
        AlbumDB* db = AlbumManager::instance()->albumDB();
        db->setItemCaption(p, _url.fileName(), description);
    
        AlbumSettings *settings = AlbumSettings::instance();
        if (settings->getSaveExifComments())
        {
            KFileMetaInfo metaInfo(_url, "image/jpeg", KFileMetaInfo::Fastest);
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

void DigikamImageInfo::setTime(const QDateTime& time, KIPI::TimeSpec)
{
    // PENDING (Gilles) : This fonction must support KURL in the future !!!...
    //                    Or the best way is a new AlbumDB method for to set the time of items.

    if ( !time.isValid() )
    {
        kdWarning() << k_funcinfo << "Invalid datetime specified"
                    << endl;
    }

    QFileInfo fi(_url.path());
    if ( !fi.exists() || !fi.isReadable() )
    {
        kdWarning() << k_funcinfo << "Target does not exist or is unreadable"
                    << endl;
    }
    
    struct utimbuf t;
    t.actime  = time.toTime_t();
    t.modtime = time.toTime_t();

    if ( ::utime( QFile::encodeName(_url.path()), &t ) != 0 )
    {
        kdWarning() << k_funcinfo << "Failed to change image file date and time"
                    << endl;
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
    // TODO ! This will a libKExif implementation call ?
    
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
                    << "This should not happen. No album specified"
                    << endl;
    }
}

DigikamImageCollection::~DigikamImageCollection()
{
}

QString DigikamImageCollection::name()
{
     return album_->getTitle();    
}

QString DigikamImageCollection::category()
{
    if ( album_->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        return p->getCollection();
    }
    else
        return QString::null;
}

QDate DigikamImageCollection::date()
{
    if ( album_->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        return p->getDate();
    }
    else
        return QDate();
}

QString DigikamImageCollection::comment()
{
    if ( album_->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        return p->getCaption();
    }
    else
        return QString::null;
}

KURL::List DigikamImageCollection::images()
{
    switch ( tp_ ) 
    {
    case AllItems:
    {
        // PENDING (Gilles) : Support KURL !...
        // Or a new method on PAlbum for to get all KURL items from an Album.

        if (album_->type() == Album::PHYSICAL)
        {
            return imagesFromPAlbum(dynamic_cast<PAlbum*>(album_));
        }
        else if (album_->type() == Album::TAG)
        {
            return imagesFromTAlbum(dynamic_cast<TAlbum*>(album_));
        }
        else
        {
            kdWarning() << k_funcinfo << "Unknown album type" << endl;
            return KURL::List();
        }

        break;
    }
    case SelectedItems:
    {
        QStringList items =
            AlbumManager::instance()->getItemHandler()->selectedItemsPath();

        return KURL::List(items);
              
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
    // if this album is the current album, then its already open
    // just return whatevers visible in the albumitemhandler
    if ( album == AlbumManager::instance()->currentAlbum() )
    {
        QStringList items =
            AlbumManager::instance()->getItemHandler()->allItemsPath();
        return KURL::List(items);
    }

    // else load the directory and return the items found
    
    QStringList items;
    QDir dir(album->getKURL().path(), imgFilter_,
             QDir::Name|QDir::IgnoreCase, QDir::Files|QDir::Readable);
    
    QStringList Files = dir.entryList();
    
    for ( QStringList::Iterator it = Files.begin() ; it != Files.end() ; ++it )
        items.append(album->getKURL().path(1) + *it);

    return KURL::List(items);
}

KURL::List DigikamImageCollection::imagesFromTAlbum(TAlbum* album) const
{
    AlbumDB* db = AlbumManager::instance()->albumDB();

    db->beginTransaction();

    QStringList     urls;
    QValueList<int> dirIDs;
    
    db->getItemsInTAlbum(album, urls, dirIDs);

    db->commitTransaction();
    
    QString basePath(AlbumManager::instance()->getLibraryPath());
    if (!basePath.endsWith("/"))
        basePath += "/";

    KURL::List urlList;
    
    QStringList::iterator     itU = urls.begin();
    while (itU != urls.end())
    {
        urlList.append(KURL(basePath + *itU));
        itU++;
    }

    return urlList;    
}


KURL DigikamImageCollection::path()
{
    if (album_->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        return p->getKURL();
    }
    else
    {
        kdWarning() << k_funcinfo << "Requesting kurl from a virtual album"
                    << endl;
        return KURL(album_->getURL());
    }
}

KURL DigikamImageCollection::uploadPath()
{
    if (album_->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(album_);
        return p->getKURL();
    }
    else
    {
        kdWarning() << k_funcinfo << "Requesting kurl from a virtual album"
                    << endl;
        return KURL(album_->getURL());
    }
}


KURL DigikamImageCollection::uploadRoot()
{
    return KURL(AlbumManager::instance()->getLibraryPath());
}

QString DigikamImageCollection::uploadRootName()
{
    return (i18n("My Albums"));
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

KIPI::ImageCollection DigikamKipiInterface::currentScope()
{
    // in digikam currentScope is always currentSelection

    return currentSelection();
}

QValueList<KIPI::ImageCollection> DigikamKipiInterface::allAlbums()
{
    QValueList<KIPI::ImageCollection> result;

    QString fileFilter(fileExtensions());

    // TODO: for now just add PAlbums. at some point we have
    // to start thinking off adding TAlbums too.
    PAlbumList palbumList = albumManager_->pAlbums();
    
    for ( QValueList<PAlbum*>::Iterator it = palbumList.begin();
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

    return result;
}

KIPI::ImageInfo DigikamKipiInterface::info( const KURL& url )
{
    return KIPI::ImageInfo( new DigikamImageInfo( this, url ) );
}

void DigikamKipiInterface::refreshImages( const KURL::List& urls )
{

    // PENDING (gilles) : Renchi, there is a way for to use KURL::List instead QStringList
    // with refreshItemHandler() method ?
    
    albumManager_->refreshItemHandler(urls.toStringList()); 
}

int DigikamKipiInterface::features() const
{
    return KIPI::ImagesHasComments          | KIPI::AcceptNewImages        |
           KIPI::AlbumEQDir                 | KIPI::AlbumsHaveComments     |
           KIPI::ImageTitlesWritable        | KIPI::ImagesHasTime          |
           KIPI::AlbumsHaveCategory         | KIPI::AlbumsHaveCreationDate |
           KIPI::AlbumsUseFirstImagePreview;
}

bool DigikamKipiInterface::addImage( const KURL& , QString&  )
{
    /* TODO:
       
    m_sourceAlbum = 0;
    m_targetAlbum = 0;
    
    // The root path is the Digikam Album library path ?
    // If it's true, do nothing because the image is already in the Albums library.
    
    if ( url.path().section('/', 0, -3) == albumManager_->getLibraryPath() )
       return true;    
    
    m_sourceAlbum = albumManager_->findPAlbum(KURL::KURL(url.directory()));
    
    // PENDING (Gilles) : Renchi, there is no way for to get a PAlbum instead Album with currentAlbum() method ?
    Album *current = albumManager_->currentAlbum();
    m_targetAlbum = albumManager_->findPAlbum(KURL::KURL(current->getURL()));
    
    if ( !m_targetAlbum ) 
       {
       errmsg = i18n("No current Album selected.");
       return false;
       }
    
    m_imageFileName = url.path().section('/', -1);                            
        
    if ( url.isValid() == false )
       {
       errmsg = i18n("'%1' is not a valid URL.").arg(url.path());
       return false;
       }
    else 
       {
       KIO::CopyJob* job = KIO::copy(url, KURL(m_targetAlbum->getURL() + "/" + m_imageFileName), true);
       
       connect(job, SIGNAL(result(KIO::Job*)),
               this, SLOT(slot_onAddImageFinished(KIO::Job*)));
       
       return true;
       }
    */
    return false;
}

void DigikamKipiInterface::slot_onAddImageFinished(KIO::Job* )
{
    /* TODO
    if (job->error())
        job->showErrorDialog(0);
    else
       {  
       // Copy the image comments if the source image is in the Albums library.
       
       if (m_sourceAlbum)
          {
          QString comments = albumDB_->getItemCaption( m_sourceAlbum, m_imageFileName );

          albumDB_->setItemCaption( m_targetAlbum, m_imageFileName, comments );
          }
       }
    
    albumManager_->refreshItemHandler( m_targetAlbum->getURL() + "/" + m_imageFileName ); 
    */
}


void DigikamKipiInterface::delImage( const KURL& url )
{
    KURL rootURL(albumManager_->getLibraryPath());
    if ( !rootURL.isParentOf(url) )
    {
        kdWarning() << k_funcinfo << "URL not in the Digikam Album library" 
                    << endl;
    }
    
    // Is there a PAlbum for this url
       
    PAlbum *palbum = albumManager_->findPAlbum( KURL(url.directory()) );
    
    if ( palbum )
    {
        // delete the item from the database
        albumDB_->deleteItem( palbum, url.fileName() );
    }
    else 
    {
        kdWarning() << k_funcinfo
                    << "Cannot find Parent album in Digikam Album library" 
                    << endl;
    }   
}

void DigikamKipiInterface::slotSelectionChanged( bool b )
{
    emit selectionChanged( b );
    emit currentScopeChanged( b );
}

void DigikamKipiInterface::slotCurrentAlbumChanged( Album *album )
{
    bool b = false;
    
    if ( album )
       b = true;
    
    emit currentAlbumChanged( b );
    emit currentScopeChanged( b );
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


#include "kipiinterface.moc"

