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
#include "albuminfo.h"
#include "albumsettings.h"
#include "digikamio.h"
#include "kipiinterface.h"


/////////////////////////////// IMAGE INFO IMPLEMENTATION CLASS ////////////////////////////////////////

DigikamImageInfo::DigikamImageInfo( KIPI::Interface* interface, const KURL& url )
                : KIPI::ImageInfoShared( interface, url )
{
    imageName_ = url.fileName();
    imageUrl_  = url.path();
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

       // store as JPEG Exif comment
           
       AlbumSettings *settings = AlbumSettings::instance();

       QString fileName(imageUrl_);
       KFileMetaInfo metaInfo(fileName, "image/jpeg",KFileMetaInfo::Fastest);

       if(settings->getSaveExifComments() && metaInfo.isValid () && metaInfo.mimeType() == "image/jpeg")
           {
           // set Jpeg comment
               
           if (metaInfo.containsGroup("Jpeg EXIF Data"))
               {
               metaInfo["Jpeg EXIF Data"].item("Comment").setValue(description);
               metaInfo.applyChanges();
               }

           // set EXIF UserComment
              
           KExifUtils::writeComment(fileName,description);
           }
       }
}

void DigikamImageInfo::setTime(const QDateTime& time, KIPI::TimeSpec spec)
{
    FILE * f;
    struct utimbuf * t = new utimbuf();
    struct tm tmp;
    struct stat st;
    
    QDate newDate = time.date();
    QTime newTime = time.time();

    time_t ti;

    f = fopen(imageUrl_.ascii(), "r");

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

    if( stat(imageUrl_.ascii(), &st ) == -1 )
       {
       kdWarning() << "DigikamImageInfo::setTime() : Cannot stat image file !!!" << endl;
       return;
       }

    // Change Access and modification date.
       
    t->actime = ti;
    t->modtime = ti;

    if( utime(imageUrl_.ascii(), t ) != 0 )
       kdWarning() << "DigikamImageInfo::setTime() : Cannot change image file date and time !!!" << endl;
}

void DigikamImageInfo::cloneData( ImageInfoShared* other )
{
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


////////////////////////////// IMAGE COLLECTION IMPLEMENTATION CLASS ////////////////////////////////////

DigikamImageCollection::DigikamImageCollection( Type tp, QString filter, Digikam::AlbumInfo *album )
                      : tp_( tp ), imgFilter_(filter)
{
    if (album)           // A specific Album has been specified !
       album_ = album;
    else                 // Using the curent selected Album !
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

QString DigikamImageCollection::category()
{
    if (album_) 
        {
        album_->openDB();
        QString category = album_->getCollection();
        album_->closeDB();    
        return (category);    
        }
    else
        return QString::null;
}

QDate DigikamImageCollection::date()
{
    if (album_) 
        {
        album_->openDB();
        QDate date = album_->getDate();
        album_->closeDB();    
        return (date);    
        }
    else
        return QDate();
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
        switch ( tp_ ) 
           {
           case AllAlbumItems:
              {
              album_->openDB();
              
              QDir albumDir(album_->getPath(), imgFilter_.latin1(),
                            QDir::Name|QDir::IgnoreCase, QDir::Files|QDir::Readable);

              QStringList Files = albumDir.entryList();
        
              for ( QStringList::Iterator it = Files.begin() ; it != Files.end() ; ++it )
                  items.append(album_->getPath() + "/" + *it);

              album_->closeDB();        
          
              if ( items.isEmpty() == true )
                 kdWarning() << "DigikamImageCollection::images()::AllAlbumItems : images list is empty!!!" 
                             << endl;

              break;
              }
              
           case AlbumItemsSelection:
              {
              album_->openDB();   
              items = album_->getSelectedItemsPath();
              album_->closeDB();        

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
    
    if (!album_) return url;
        
    album_->openDB();
    url.setPath( album_->getPath() );
    album_->closeDB();        
    return url;
}

KURL DigikamImageCollection::uploadRoot()
{
    KURL url;
    
    url.setPath( Digikam::AlbumManager::instance()->getLibraryPath() );
    return (url);
}

QString DigikamImageCollection::uploadRootName()
{
    return (i18n("My Albums"));
}

///////////////////////// KIPI INTERFACE IMPLEMENTATION CLASS //////////////////////////////////////////

DigikamKipiInterface::DigikamKipiInterface( QObject *parent, const char *name)
                     :KIPI::Interface( parent, name )
{
    readSettings();

    // Get AlbumManger instance.
    
    albumManager_ = Digikam::AlbumManager::instance();
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
    if ( albumManager_->currentAlbum()->getSelectedItems().isEmpty() )
       {
       kdWarning() << "DigikamKipiInterface::currentSelection() : no current selection!!!" 
                   << endl;
       return KIPI::ImageCollection(0);
       }
    else
       return KIPI::ImageCollection( new DigikamImageCollection
                                         (
                                         DigikamImageCollection::AlbumItemsSelection,
                                         imagesFileFilter_
                                         ) );
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
    
    for (Digikam::AlbumInfo *album = albumManager_->firstAlbum() ;
         album ; album = album->nextAlbum())
        {
        album->openDB();
        DigikamImageCollection* col = new DigikamImageCollection
                                          ( 
                                          DigikamImageCollection::AllAlbumItems, 
                                          imagesFileFilter_,
                                          album 
                                          );
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
           KIPI::ImageTitlesWritable  | KIPI::ImagesHasTime |
           KIPI::AlbumsHaveCategory   | KIPI::AlbumsHaveCreationDate;
}

bool DigikamKipiInterface::addImage( const KURL& url, QString& errmsg )
{
    m_sourceAlbum = 0;
    m_targetAlbum = 0;
    
    // The root path is the Digikam Album library path ?
    // If it's true, do nothing because the image is already in the Albums library.
    
    if ( url.path().section('/', 0, -3) == albumManager_->getLibraryPath() )
       return true;    
    
    m_sourceAlbum = albumManager_->findAlbum(url.path().section('/', -2, -2));
    m_targetAlbum = albumManager_->currentAlbum();
    
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
       KIO::CopyJob* job = KIO::copy(url, KURL(m_targetAlbum->getPath() + "/" + m_imageFileName), true);
       
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
       {  // Copy the image comments if the source image is in the Albums library.
       if (m_sourceAlbum)
          {
          m_sourceAlbum->openDB();
          QString comments = m_sourceAlbum->getItemComments(m_imageFileName);
          m_sourceAlbum->closeDB();

          m_targetAlbum->openDB();
          m_targetAlbum->setItemComments(m_imageFileName, comments);
          m_targetAlbum->closeDB();
          }
       }
    
    albumManager_->refreshItemHandler(); 
}


void DigikamKipiInterface::delImage( const KURL& url )
{
    // The root path is the Digikam Album library path ?
    
    if ( url.path().section('/', 0, -3) == albumManager_->getLibraryPath() )
       {
       // There is an Album with the Album name include in the 'url' ?
       
       Digikam::AlbumInfo *album = albumManager_->findAlbum( url.path().section('/', -2, -2));
    
       if ( album )
          {
          if ( KIO::NetAccess::del(url) == false )
             {
             kdWarning() << "DigikamKipiInterface::delImage() : Cannot delete an image !!!" 
                         << endl;
             }
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

void DigikamKipiInterface::slotCurrentAlbumChanged( Digikam::AlbumInfo *album )
{
    bool b = false;
    
    if ( album )
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

