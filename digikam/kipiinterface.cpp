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
#include <stdio.h>
#include <sys/types.h>
#include <utime.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
}

// Qt includes.

#include <qdir.h>

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
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "kipiinterface.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// IMAGE INFO IMPLEMENTATION CLASS ////////////////////////////////////////

DigikamImageInfo::DigikamImageInfo( KIPI::Interface* interface, const KURL& url )
                : KIPI::ImageInfoShared( interface, url )
{
    albumManager_ = AlbumManager::instance();
    imageName_    = url.fileName();
    imageUrl_     = url;
    albumURL_     = KURL::KURL(url.directory());
    albumDB_      = albumManager_->albumDB();
    
    if (albumURL_.isValid() == true && imageName_.isEmpty() == false)
       {
       palbum_ = albumManager_->findPAlbum(albumURL_);
       
       if (palbum_)
          imageComments_ = albumDB_->getItemCaption(palbum_, imageName_);
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
    if (palbum_ && name.isEmpty() == false)
       albumDB_->moveItem(palbum_, imageName_, palbum_, name);
}

void DigikamImageInfo::setDescription( const QString& description )
{
    if (palbum_)
       {
       albumDB_->setItemCaption(palbum_, imageName_, description);
                        
       // store as JPEG Exif comment
           
       AlbumSettings *settings = AlbumSettings::instance();

       KFileMetaInfo metaInfo(imageUrl_, "image/jpeg",KFileMetaInfo::Fastest);

       if(settings->getSaveExifComments() && metaInfo.isValid () && metaInfo.mimeType() == "image/jpeg")
           {
           // set Jpeg comment
               
           if (metaInfo.containsGroup("Jpeg EXIF Data"))
               {
               metaInfo["Jpeg EXIF Data"].item("Comment").setValue(description);
               metaInfo.applyChanges();
               }

           // set EXIF UserComment
              
           QString fileName(imageUrl_.path());                  // PENDING (gilles): Ralph, LibKexif must 
                                                                // support KURL in the future...
           KExifUtils::writeComment(fileName, description);
           }
       }
}

void DigikamImageInfo::setTime(const QDateTime& time, KIPI::TimeSpec spec)
{
    // PENDING (Gilles) : This fonction must support KURL in the future !!!...
    //                    Or the best way is a new AlbumDB method for to set the time of items.
    
    FILE * f;
    struct utimbuf * t = new utimbuf();
    struct tm tmp;
    struct stat st;
    
    QDate newDate = time.date();
    QTime newTime = time.time();

    time_t ti;

    f = fopen(imageUrl_.path().ascii(), "r");

    if ( f == NULL )
       {
       kdWarning() << "DigikamImageInfo::setTime() : Cannot open image file !!!" << endl;
       return;
       }

    fclose( f );

    tmp.tm_mday = newDate.day();
    tmp.tm_mon = newDate.month() - 1;
    tmp.tm_year = newDate.year() - 1900;

    tmp.tm_hour = newTime.hour();
    tmp.tm_min = newTime.minute();
    tmp.tm_sec = newTime.second();
    tmp.tm_isdst = -1;

    ti = mktime( &tmp );

    if( ti == -1 )
       {
       kdWarning() << "DigikamImageInfo::setTime() : Cannot eval local time !!!" << endl;
       return;
       }

    if( stat(imageUrl_.path().ascii(), &st ) == -1 )
       {
       kdWarning() << "DigikamImageInfo::setTime() : Cannot stat image file !!!" << endl;
       return;
       }

    // Change Access and modification date.
       
    t->actime = ti;
    t->modtime = ti;

    if( utime(imageUrl_.path().ascii(), t ) != 0 )
       kdWarning() << "DigikamImageInfo::setTime() : Cannot change image file date and time !!!" << endl;
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

void DigikamImageInfo::addAttributes( const QMap<QString,QVariant>& map )
{
    // TODO ! This will used for the futures tags Digikam features.
}

int DigikamImageInfo::angle()
{
    // TODO ! This will a libKExif implementation call ?
    
    return 0;
}

void DigikamImageInfo::setAngle( int angle )
{
    // TODO ! This will a libKExif implementation call ?
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// IMAGE COLLECTION IMPLEMENTATION CLASS ////////////////////////////////////

DigikamImageCollection::DigikamImageCollection( Type tp, QString filter, PAlbum *album )
                      : tp_( tp ), imgFilter_(filter)
{
    albumManager_ = AlbumManager::instance();
    
    if (album)           
       {
       // A specific Album has been specified !
       palbum_  = album;
       }
    else                 
       {   
       // Using the curent selected Album !
       Album *current = albumManager_->currentAlbum();
       palbum_ = albumManager_->findPAlbum(KURL::KURL(current->getURL()));       
       }
}

DigikamImageCollection::~DigikamImageCollection()
{
}

QString DigikamImageCollection::name()
{
    if (palbum_) 
        {
        // PENDING : Renchi, Why the GetTitle() method isn't available in PAlbum ?
        
        QString title = palbum_->getTitle();    
        return (title);    
        }
    else
        return QString::null;
}

QString DigikamImageCollection::category()
{
    if (palbum_) 
        {
        QString category = palbum_->getCollection();
        return (category);    
        }
    else
        return QString::null;
}

QDate DigikamImageCollection::date()
{
    if (palbum_) 
        {
        QDate date = palbum_->getDate();
        return (date);    
        }
    else
        return QDate();
}

QString DigikamImageCollection::comment()
{
    if (palbum_) 
        {
        QString comments = palbum_->getCaption();
        return (comments);
        }
    else
        return QString::null;
}

KURL::List DigikamImageCollection::images()
{
    QStringList items;

    if (palbum_) 
        {
        switch ( tp_ ) 
           {
           case AllAlbumItems:       // PENDING (Gilles) : Support KURL !...
                                     // Or a new method on PAlbum for to get all KURL items from an Album.
              {
              QDir albumDir(palbum_->getKURL().path(), imgFilter_.latin1(),
                            QDir::Name|QDir::IgnoreCase, QDir::Files|QDir::Readable);

              QStringList Files = albumDir.entryList();
        
              for ( QStringList::Iterator it = Files.begin() ; it != Files.end() ; ++it )
                  items.append(palbum_->getKURL().path() + "/" + *it);

              if ( items.isEmpty() == true )
                 kdWarning() << "DigikamImageCollection::images()::AllAlbumItems : images list is empty!!!" 
                             << endl;

              break;
              }
              
           case AlbumItemsSelection:
              {
              // PENDING (gilles) : Renchi, _WHERE_I_CAN_FIND_ the very 
              // important getSelectedItemsPath() method ???
              
              //items = album_->getSelectedItemsPath();

              if ( items.isEmpty() == true )
                 kdWarning() << "DigikamImageCollection::images()::AlbumItemsSelection : images list is empty!!!"
                             << endl;
              
              break;
              }
              
           default:
              break;
           }
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

KURL DigikamImageCollection::uploadPath()
{
    return commonRoot();
}

KURL DigikamImageCollection::commonRoot()
{
    KURL url;
    
    if (!palbum_) return url;
        
    url = palbum_->getKURL();
    return url;
}

KURL DigikamImageCollection::uploadRoot()
{
    KURL url;
    
    // PENDING (gilles) : Renchi, why getLibraryPath() method don't return KURL instead...
    
    url.setPath( albumManager_->getLibraryPath() );
    return (url);
}

QString DigikamImageCollection::uploadRootName()
{
    return (i18n("My Albums"));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// KIPI INTERFACE IMPLEMENTATION CLASS //////////////////////////////////////////

DigikamKipiInterface::DigikamKipiInterface( QObject *parent, const char *name)
                     :KIPI::Interface( parent, name )
{
    albumManager_ = AlbumManager::instance();
    albumDB_      = albumManager_->albumDB();
    readSettings();
}

DigikamKipiInterface::~DigikamKipiInterface()
{
}

KIPI::ImageCollection DigikamKipiInterface::currentAlbum()
{
    if ( albumManager_->currentAlbum() )
       return KIPI::ImageCollection( new DigikamImageCollection
                                         ( 
                                         DigikamImageCollection::AllAlbumItems, 
                                         imagesFileFilter_
                                         ) );
    else
       {
       kdWarning() << "DigikamKipiInterface::currentAlbum() : no current album!!!" 
                   << endl;
       return KIPI::ImageCollection(0);
       }
}

KIPI::ImageCollection DigikamKipiInterface::currentSelection()
{
    // PENDING (gilles) : Renchi, _WHERE_I_CAN_FIND_ the very important getSelectedItemsPath() method ???

/*    if ( albumManager_->currentAlbum()->getSelectedItems().isEmpty() )
       {*/
       kdWarning() << "DigikamKipiInterface::currentSelection() : no current selection!!!" 
                   << endl;
       return KIPI::ImageCollection(0);
       /*}
    else
       return KIPI::ImageCollection( new DigikamImageCollection
                                         (
                                         DigikamImageCollection::AlbumItemsSelection,
                                         imagesFileFilter_
                                         ) );*/
}

KIPI::ImageCollection DigikamKipiInterface::currentScope()
{
    if ( albumManager_->currentAlbum() )
       {
       DigikamImageCollection *images = new DigikamImageCollection
                                            ( 
                                            DigikamImageCollection::AlbumItemsSelection,
                                            imagesFileFilter_
                                            );
    
       if ( images->images().isEmpty() == false )
           return currentSelection();
       else
           return currentAlbum();
       }       
    else
       {
       kdWarning() << "DigikamKipiInterface::currentScope() : no current album!!!" 
                   << endl;
       return KIPI::ImageCollection(0);
       }
}

QValueList<KIPI::ImageCollection> DigikamKipiInterface::allAlbums()
{
    QValueList<KIPI::ImageCollection> result;
    
    PAlbumList palbumList = albumManager_->pAlbums();
    
    for(  QValueList<PAlbum*>::Iterator it = palbumList.begin(); it != palbumList.end(); ++it ) 
        {
        DigikamImageCollection* col = new DigikamImageCollection
                                          ( 
                                          DigikamImageCollection::AllAlbumItems, 
                                          imagesFileFilter_,
                                          *it 
                                          );
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

bool DigikamKipiInterface::addImage( const KURL& url, QString& errmsg )
{
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
    
    if ( m_targetAlbum ) 
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
}

void DigikamKipiInterface::slot_onAddImageFinished(KIO::Job* job)
{
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
}


void DigikamKipiInterface::delImage( const KURL& url )
{
    // The root path is the Digikam Album library path ?

    if ( url.path().section('/', 0, -3) == albumManager_->getLibraryPath() )
       {
       // There is an Album with the Album name include in the 'url' ?
       
       PAlbum *palbum = albumManager_->findPAlbum( KURL::KURL(url.directory()) );
    
       if ( palbum )
          {
          albumDB_->deleteItem( palbum, url.fileName() );
          
          // PENDING (gilles) : Renchi, HowTo get succes information after deleteItem() running ?
          
/*          kdWarning() << "DigikamKipiInterface::delImage() : Cannot delete an image !!!" 
                         << endl;*/
          }
       else 
          {
          kdWarning() << "DigikamKipiInterface::delImage() : "
                         "cannot find the Album in the Digikam Album library !!!" 
                      << endl;
          }   
       }
    else 
       {
       kdWarning() << "DigikamKipiInterface::delImage() : url isn't in the Digikam Album library !!!" 
                   << endl;
       }   
}

void DigikamKipiInterface::slotSelectionChanged( bool b )
{
    emit selectionChanged( b );
    emit currentScopeChanged( b );
}

void DigikamKipiInterface::slotCurrentAlbumChanged( PAlbum *palbum )
{
    bool b = false;
    
    if ( palbum )
       b = true;
    
    emit currentAlbumChanged( b );
    emit currentScopeChanged( b );
}

QString DigikamKipiInterface::fileExtensions()
{
  return (imagesFileFilter_);
}

void DigikamKipiInterface::readSettings()
{
    // Read File Filter settings in digikamrc file.

    KConfig *config = new KConfig("digikamrc");
    config->setGroup("Album Settings");
    QString Temp = config->readEntry("File Filter", "*.jpg *.jpeg *.tif *.tiff *.gif *.png *.bmp");
    delete config;
  
    imagesFileFilter_ = Temp.lower() + " " + Temp.upper();
}


#include "kipiinterface.moc"

