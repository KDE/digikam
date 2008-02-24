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
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <Q3Header>
#include <QHBoxLayout>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QRegExp>

// KDE includes.

#include <ktabwidget.h>
#include <klocale.h>
#include <kconfig.h>
#include <kfilemetainfo.h>
#include <kio/netaccess.h>

// libKipi Includes.

#include <libkipi/version.h>

// Local includes.

#include "constants.h"
#include "ddebug.h"
#include "folderview.h"
#include "folderitem.h"
#include "albumitemhandler.h"
#include "album.h"
#include "albumdb.h"
#include "albumthumbnailloader.h"
#include "albumsettings.h"
#include "collectionmanager.h"
#include "databaseaccess.h"
#include "dmetadata.h"
#include "imageattributeswatch.h"
#include "imagelister.h"
#include "namefilter.h"
#include "thumbnailloadthread.h"
#include "kipiinterface.h"
#include "kipiinterface.moc"

namespace Digikam
{

//-- Image Info ------------------------------------------------------------------

DigikamImageInfo::DigikamImageInfo( KIPI::Interface* interface, const KUrl& url )
                : KIPI::ImageInfoShared( interface, url )
{
    m_info = ImageInfo(url);
}

DigikamImageInfo::~DigikamImageInfo()
{
}

PAlbum* DigikamImageInfo::parentAlbum()
{
    return AlbumManager::instance()->findPAlbum(m_info.albumId());
}

QString DigikamImageInfo::title()
{
    return m_info.name();
}

QString DigikamImageInfo::description()
{
    return m_info.comment();
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
    DatabaseAccess access;
    ImageComments comments = m_info.imageComments(access);
    // we set a comment with default language, author and date null
    comments.addComment(description);
}

QDateTime DigikamImageInfo::time( KIPI::TimeSpec /*spec*/ )
{
    return m_info.dateTime();
}

void DigikamImageInfo::setTime(const QDateTime& time, KIPI::TimeSpec)
{
    if ( !time.isValid() )
    {
        DWarning() << "Invalid datetime specified" << endl;
        return;
    }

    m_info.setDateTime(time);
    AlbumManager::instance()->refreshItemHandler( _url );
}

void DigikamImageInfo::cloneData( ImageInfoShared* other )
{
    setDescription( other->description() );
    setTime( other->time(KIPI::FromInfo), KIPI::FromInfo );
    addAttributes( other->attributes() );
}

QMap<QString, QVariant> DigikamImageInfo::attributes()
{
    QMap<QString, QVariant> res;

    PAlbum* p = parentAlbum();
    if (p)
    {
        QList<int> tagIds = m_info.tagIds();
        // Get digiKam Tags Path list of picture from database.
        // Ex.: "City/Paris/Monuments/Notre Dame"

        QStringList tagspath = AlbumManager::instance()->tagPaths(tagIds, false);
        res["tagspath"]      = tagspath;

        // Get digiKam Tags name list of picture from database.
        // Ex.: "Notre Dame"
        QStringList tags     = AlbumManager::instance()->tagNames(tagIds);
        res["tags"]          = tags;

        // Get digiKam Rating of picture from database.
        int rating           = m_info.rating();
        res["rating"]        = rating;

        // Get GPS location of picture from database.
        ImagePosition pos = m_info.imagePosition();
        if (!pos.isEmpty())
        {
            double lat           = pos.latitudeNumber();
            double lng           = pos.longitudeNumber();
            double alt           = pos.altitude();
            res["latitude"]      = lat;
            res["longitude"]     = lng;
            res["altitude"]      = alt;
        }

        // TODO: add here a kipi-plugins access to future picture attributes stored by digiKam database
    }
    return res;
}

void DigikamImageInfo::addAttributes(const QMap<QString, QVariant>& res)
{
    PAlbum* p = parentAlbum();
    if (p)
    {
        QMap<QString, QVariant> attributes = res;

        // Set digiKam Tags Path list of picture into database. 
        // Ex.: "City/Paris/Monuments/Notre Dame"
        if (attributes.find("tagspath") != attributes.end())
        {
            QStringList tagspaths = attributes["tagspath"].toStringList();

            DatabaseAccess access;
            // get tag ids, create if necessary
            QList<int> tagIds = access.db()->getTagsFromTagPaths(tagspaths, true);
            access.db()->addTagsToItems(QList<qlonglong>() << m_info.id(), tagIds);
        }

        // Set digiKam Rating of picture into database.
        if (attributes.contains("rating"))
        {
            int rating = attributes["rating"].toInt();
            if (rating >= RatingMin && rating <= RatingMax)
                m_info.setRating(rating);
        }

        // GPS location management from plugins.

        if (attributes.contains("latitude") ||
            attributes.contains("longitude") ||
            attributes.contains("altitude"))
        {
            ImagePosition position = m_info.imagePosition();

            // Set GPS latitude location of picture into database.
            if (attributes.contains("latitude"))
            {
                double lat = attributes["latitude"].toDouble();
                if (lat >= -90.0 && lat <= 90.0)
                {
                    position.setLatitude(lat);
                }
            }

            // Set GPS longitude location of picture into database.
            if (attributes.contains("longitude"))
            {
                double lng = attributes["longitude"].toDouble();
                if (lng >= -180.0 && lng <= 180.0)
                {
                    position.setLongitude(lng);
                }
            }

            // Set GPS altitude location of picture into database.
            if (attributes.contains("altitude"))
            {
                double alt = attributes["altitude"].toDouble();
                position.setAltitude(alt);
            }

            position.apply();
        }

        // TODO: add here a kipi-plugins access to future picture attributes stored by digiKam database
    }

    // To update sidebar content. Some kipi-plugins use this way to refresh sidebar 
    // using an empty QMap(). 
    ImageAttributesWatch::instance()->fileMetadataChanged(_url);
}

void DigikamImageInfo::delAttributes(const QStringList& res)
{
    // TODO: implement me.
}

void DigikamImageInfo::clearAttributes()
{
    // TODO: implement me.
}

int DigikamImageInfo::angle()
{
    AlbumSettings *settings = AlbumSettings::instance();
    if (settings->getExifRotate())
    {
        //TODO: read from DB
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

DigikamImageCollection::DigikamImageCollection(Type tp, Album* album, const QString& filter)
{
    m_tp        = tp;
    m_album     = album;
    m_imgFilter = filter;

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
    if ( m_album->type() == Album::TAG )
    {
        return i18n("Tag: %1",m_album->title());
    }
    else
        return m_album->title();
}

QString DigikamImageCollection::category()
{
    if ( m_album->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
        return p->collection();
    }
    else if ( m_album->type() == Album::TAG )
    {
        TAlbum *p = dynamic_cast<TAlbum*>(m_album);
        return i18n("Tag: %1",p->tagPath());
    }
    else
        return QString();
}

QDate DigikamImageCollection::date()
{
    if ( m_album->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
        return p->date();
    }
    else
        return QDate();
}

QString DigikamImageCollection::comment()
{
    if ( m_album->type() == Album::PHYSICAL )
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
        return p->caption();
    }
    else
        return QString();
}

KUrl::List DigikamImageCollection::images()
{
    switch ( m_tp )
    {
        case AllItems:
        {
            if (m_album->type() == Album::PHYSICAL)
            {
                return imagesFromPAlbum(dynamic_cast<PAlbum*>(m_album));
            }
            else if (m_album->type() == Album::TAG)
            {
                return imagesFromTAlbum(dynamic_cast<TAlbum*>(m_album));
            }
            else if (m_album->type() == Album::DATE || 
                    m_album->type() == Album::SEARCH)
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

    NameFilter nameFilter(m_imgFilter);

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

    NameFilter nameFilter(m_imgFilter);

    for (QStringList::iterator it = urls.begin(); it != urls.end(); ++it)
    {
        if (nameFilter.matches(*it))
            urlList.append(*it);
    }

    return urlList;
}

KUrl DigikamImageCollection::path()
{
    if (m_album->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
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
    if (m_album->type() == Album::PHYSICAL)
    {
        PAlbum *p = dynamic_cast<PAlbum*>(m_album);
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
    if (m_album->type() == Album::PHYSICAL)
        return true;
    else
        return false;
}

bool DigikamImageCollection::operator==(ImageCollectionShared& imgCollection)
{
    DigikamImageCollection* thatCollection = static_cast<DigikamImageCollection*>(&imgCollection);
    return (m_album == thatCollection->m_album);
}

//-- LibKipi interface -----------------------------------------------------------

DigikamKipiInterface::DigikamKipiInterface( QObject *parent, const char *name)
                    : KIPI::Interface( parent, name )
{
    m_thumbLoadThread = new ThumbnailLoadThread();

    // Set cache size to 256 to have the max quality thumb.
    m_thumbLoadThread->setThumbnailSize(256);          
    m_thumbLoadThread->setSendSurrogatePixmap(true);
    m_thumbLoadThread->setExifRotate(AlbumSettings::instance()->getExifRotate());

    m_albumManager = AlbumManager::instance();

    connect(m_albumManager, SIGNAL( signalAlbumItemsSelected( bool ) ),
            this, SLOT( slotSelectionChanged( bool ) ));

    connect(m_albumManager, SIGNAL( signalAlbumCurrentChanged(Album*) ),
            this, SLOT( slotCurrentAlbumChanged(Album*) ));

    connect(m_thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));
}

DigikamKipiInterface::~DigikamKipiInterface()
{
    delete m_thumbLoadThread;
}

KIPI::ImageCollection DigikamKipiInterface::currentAlbum()
{
    Album* currAlbum = m_albumManager->currentAlbum();
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
    Album* currAlbum = m_albumManager->currentAlbum();
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

    AlbumList palbumList = m_albumManager->allPAlbums();
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

    AlbumList talbumList = m_albumManager->allTAlbums();
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
    m_albumManager->refreshItemHandler(urls);
}

int DigikamKipiInterface::features() const
{
    return (
           KIPI::HostSupportsTags           | KIPI::HostSupportsRating         |
           KIPI::HostAcceptNewImages        | KIPI::HostSupportsThumbnails     |
           KIPI::ImagesHasComments          | 
           KIPI::ImagesHasTime              | KIPI::ImagesHasTitlesWritable    |
           KIPI::AlbumsHaveComments         | KIPI::AlbumsHaveCategory         |
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

    PAlbum *targetAlbum = m_albumManager->findPAlbum(url.directory());

    if ( !targetAlbum )
    {
        errmsg = i18n("Target album is not in the album library.");
        return false;
    }

    m_albumManager->refreshItemHandler( url );

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

    PAlbum *palbum = m_albumManager->findPAlbum( KUrl(url.directory()) );

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

void DigikamKipiInterface::thumbnail(const KUrl& url, int /*size*/)
{
    // NOTE: size is not used here. Cache use the max pixmap size to store thumbs (256).
    m_thumbLoadThread->find(url.path());
}

void DigikamKipiInterface::thumbnails(const KUrl::List& list, int size)
{
    for (KUrl::List::const_iterator it = list.begin() ; it != list.end() ; ++it)
        thumbnail((*it).path(), size);
}

void DigikamKipiInterface::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    emit gotThumbnail( KUrl(desc.filePath), pix );
}

KIPI::ImageCollectionSelector* DigikamKipiInterface::imageCollectionSelector(QWidget *parent)
{
    return (new DigikamImageCollectionSelector(this, parent));
}

//-- Image Collection Selector Widget --------------------------------------------

class ImageCollectionSelectorItem : public FolderCheckListItem
{
public:

    ImageCollectionSelectorItem(Q3ListView* parent, Album* tag);
    ImageCollectionSelectorItem(Q3ListViewItem* parent, Album* tag);

    Album* album() const;

private:

    Album *m_album;
};

ImageCollectionSelectorItem::ImageCollectionSelectorItem(Q3ListView* parent, Album* album)
                           : FolderCheckListItem(parent, album->title(), Q3CheckListItem::CheckBox)
{
    m_album = album;
    m_album->setExtraData(listView(), this);
}

ImageCollectionSelectorItem::ImageCollectionSelectorItem(Q3ListViewItem* parent, Album* album)
                           : FolderCheckListItem(parent, album->title(), Q3CheckListItem::CheckBox)
{
    m_album = album;
    m_album->setExtraData(listView(), this);
}

Album* ImageCollectionSelectorItem::album() const
{
    return m_album;
}

DigikamImageCollectionSelector::DigikamImageCollectionSelector(DigikamKipiInterface* iface, QWidget *parent)
                              : KIPI::ImageCollectionSelector(parent)
{
    m_iface      = iface;
    m_tab        = new KTabWidget(this);
    m_albumsView = new FolderView(m_tab);
    m_albumsView->addColumn(i18n("My Albums"));
    m_albumsView->setColumnWidthMode(0, Q3ListView::Maximum);
    m_albumsView->setResizeMode(Q3ListView::AllColumns);
    m_albumsView->setRootIsDecorated(true);
    m_albumsView->header()->hide();
   
    m_tagsView = new FolderView(m_tab);
    m_tagsView->addColumn(i18n("My Tags"));
    m_tagsView->setColumnWidthMode(0, Q3ListView::Maximum);
    m_tagsView->setResizeMode(Q3ListView::AllColumns);
    m_tagsView->setRootIsDecorated(true);
    m_tagsView->header()->hide();

    m_tab->addTab(m_albumsView, i18n("My Albums"));
    m_tab->addTab(m_tagsView, i18n("My Tags"));

    QHBoxLayout *hlay = new QHBoxLayout(this);
    hlay->addWidget(m_tab);
    hlay->setMargin(0);
    hlay->setSpacing(0);

    // -- Load all Album views-------------------------------------------------------------

    loadTreeView(AlbumManager::instance()->allPAlbums(), m_albumsView); 
    loadTreeView(AlbumManager::instance()->allTAlbums(), m_tagsView); 
}

DigikamImageCollectionSelector::~DigikamImageCollectionSelector() 
{
}

void DigikamImageCollectionSelector::loadTreeView(const AlbumList& aList, FolderView *view)
{
    for (AlbumList::const_iterator it = aList.begin(); it != aList.end(); ++it)
    {
        Album* album = *it;

        ImageCollectionSelectorItem* item = 0;
        
        if (album->isRoot())
        {
            item = new ImageCollectionSelectorItem(view, album);
            item->setOpen(true);
        }
        else
        {
            ImageCollectionSelectorItem* pitem = (ImageCollectionSelectorItem*)(album->parent()->extraData(view));
            if (!pitem)
            {
                DWarning() << "Failed to find parent for Album " << album->title() << endl;
                continue;
            }

            item = new ImageCollectionSelectorItem(pitem, album);
        }

        if (item)
        {
            PAlbum* palbum = dynamic_cast<PAlbum*>(album);
            if (palbum)
                item->setPixmap(0, AlbumThumbnailLoader::instance()->getStandardAlbumIcon(palbum));
            else
            {
                TAlbum* talbum = dynamic_cast<TAlbum*>(album);
                if (talbum)
                    item->setPixmap(0, AlbumThumbnailLoader::instance()->getStandardTagIcon(talbum));
            }

            album->setExtraData(view, item);

            if (album == AlbumManager::instance()->currentAlbum())
            {
                item->setOpen(true);
                view->setSelected(item, true);
                view->ensureItemVisible(item);
            }
        }
    }
}

QList<KIPI::ImageCollection> DigikamImageCollectionSelector::selectedImageCollections() const
{
    QString ext = m_iface->fileExtensions();
    QList<KIPI::ImageCollection> list; 
 
    Q3ListViewItemIterator it(m_albumsView, Q3ListViewItemIterator::Checked);
    while (it.current())
    {
        ImageCollectionSelectorItem* item = dynamic_cast<ImageCollectionSelectorItem*>(*it);
        if (item)
        {
            DigikamImageCollection *col = new DigikamImageCollection(DigikamImageCollection::AllItems, item->album(), ext);
            list.append(col);
         }
         ++it;
    }

    Q3ListViewItemIterator it2(m_tagsView, Q3ListViewItemIterator::Checked);
    while (it2.current())
    {
        ImageCollectionSelectorItem* item = dynamic_cast<ImageCollectionSelectorItem*>(*it2);
        if (item)
        {
            DigikamImageCollection *col = new DigikamImageCollection(DigikamImageCollection::AllItems, item->album(), ext);
            list.append(col);
         }
         ++it2;
    }

    DDebug() << list.count() << " collection items selected" << endl;

    return list;
}

}  // namespace Digikam
