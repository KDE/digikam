//////////////////////////////////////////////////////////////////////////////
//
//    ALBUMICONVIEW.CPP
//
//    Copyright (C) 2002-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles Caulier <caulier dot gilles at free.fr>
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

// Qt includes.

#include <qpixmap.h>
#include <qimage.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qguardedptr.h>
#include <qdragobject.h>
#include <qcursor.h>
#include <qvaluevector.h>
#include <qptrlist.h>
#include <qguardedptr.h>
#include <qdict.h>

// KDE includes.

#include <kio/previewjob.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kurl.h>
#include <kurldrag.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpropsdlg.h>
#include <ktrader.h>
#include <kservice.h>
#include <krun.h>
#include <kaction.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <libkexif/kexif.h>
#include <libkexif/kexifutils.h>
#include <libkexif/kexifdata.h>

#include <cstdio>

#include <kdeversion.h>
#if KDE_IS_VERSION(3,2,0)
#include <kcalendarsystem.h>
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "thumbnailjob.h"
#include "albumfilecopymove.h"
#include "albumlister.h"

#include "albumsettings.h"
#include "imagedescedit.h"
#include "imagewindow.h"
#include "thumbnailsize.h"

#include "cameratype.h"
#include "cameradragobject.h"
#include "dragobjects.h"

#include "albumiconitem.h"
#include "digikamapp.h"
#include "albumiconview.h"

class AlbumIconViewPrivate 
{
public:

    void init() {
        imageLister   = 0;
        currentAlbum  = 0;
        albumSettings = 0;
    }
    
    AlbumLister         *imageLister;
    Album               *currentAlbum;
    const AlbumSettings *albumSettings;
    QGuardedPtr<Digikam::ThumbnailJob> thumbJob;

    ThumbnailSize thumbSize;

    QString albumTitle;
    QString albumDate;
    QString albumComments;

    QRect itemRect;
    QRect itemDateRect;
    QRect itemPixmapRect;
    QRect itemNameRect;
    QRect itemCommentsRect;
    QRect itemFileCommentsRect;
    QRect itemResolutionRect;
    QRect itemSizeRect;
    QRect itemTagRect;

    QPixmap itemRegPixmap;
    QPixmap itemSelPixmap;

    QFont fnReg;
    QFont fnCom;
    QFont fnXtra;

    QDict<AlbumIconItem> itemDict;
};


AlbumIconView::AlbumIconView(QWidget* parent)
             : ThumbView(parent) 
{
    d = new AlbumIconViewPrivate;
    d->init();
    d->imageLister = new AlbumLister();

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    // -- ImageLister connections -------------------------------------

    connect(d->imageLister, SIGNAL(signalNewItems(const KFileItemList&)),
            this, SLOT(slotImageListerNewItems(const KFileItemList&)));

    connect(d->imageLister, SIGNAL(signalDeleteItem(KFileItem*)),
            this, SLOT(slotImageListerDeleteItem(KFileItem*)) );

    connect(d->imageLister, SIGNAL(signalClear()),
            this, SLOT(slotImageListerClear()));

    connect(d->imageLister, SIGNAL(signalCompleted()),
            this, SLOT(slotImageListerCompleted()));

    connect(d->imageLister, SIGNAL(signalRefreshItems(const KFileItemList&)),
            this, SLOT(slotImageListerRefreshItems(const KFileItemList&)));

    // -- Icon connections --------------------------------------------

    connect(this, SIGNAL(signalDoubleClicked(ThumbItem *)),
            this, SLOT(slotDoubleClicked(ThumbItem *)));
            
    connect(this, SIGNAL(signalReturnPressed(ThumbItem *)),
            this, SLOT(slotDoubleClicked(ThumbItem *)));
            
    connect(this, SIGNAL(signalRightButtonClicked(ThumbItem *, const QPoint &)),
            this, SLOT(slotRightButtonClicked(ThumbItem *, const QPoint &)));
            
    connect(this, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    // -- Self connections ----------------------------------------------

    connect(this, SIGNAL(contentsMoving(int, int)),
            SLOT(slotContentsMoving(int, int)));

    // -- resource for broken image thumbnail ---------------------------
    KGlobal::dirs()->addResourceType("digikam_imagebroken",
                                     KGlobal::dirs()->kde_default("data") 
                                     + "digikam/data");
}

AlbumIconView::~AlbumIconView() 
{
    if (!d->thumbJob.isNull()) 
    {
        d->thumbJob->kill();
    }

    if (!d->thumbJob.isNull())
        delete d->thumbJob;
    
    delete d->imageLister;
    delete d;
}

void AlbumIconView::applySettings(const AlbumSettings* settings)
{
    if (!settings)
        return;
    
    d->albumSettings = settings;

    d->imageLister->setNameFilter(d->albumSettings->getImageFileFilter() + " " + 
                                  d->albumSettings->getMovieFileFilter() + " " +
                                  d->albumSettings->getAudioFileFilter() + " " +
                                  d->albumSettings->getRawFileFilter());

    d->thumbSize = (ThumbnailSize::Size)d->albumSettings->getDefaultIconSize(); 

    updateItemRectsPixmap();

    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
    }

    d->imageLister->stop();
    d->itemDict.clear();
    clear();

    if (d->currentAlbum)
    {
        updateBanner();
        d->imageLister->openAlbum(d->currentAlbum);
    }
}


void AlbumIconView::albumDescChanged()
{
    updateBanner();
}

void AlbumIconView::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    if ( d->thumbSize != thumbSize)
    {

        if (!d->thumbJob.isNull())
        {
            d->thumbJob->kill();
        }

        d->imageLister->stop();
        d->itemDict.clear();
        clear();
        
        d->thumbSize = thumbSize;

        updateItemRectsPixmap();
        
        d->imageLister->openAlbum(d->currentAlbum);
    }
}

void AlbumIconView::setAlbum(Album* album)
{
    if (!album) 
    {
        d->currentAlbum = 0;
        d->itemDict.clear();
        clear();

        d->imageLister->stop();
    
        if (!d->thumbJob.isNull())
        {
            d->thumbJob->kill();
        }
        return;
    }
    
    if (d->currentAlbum == album)
        return;

    d->imageLister->stop();
    
    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
    }
    
    d->currentAlbum = album;
    d->imageLister->openAlbum(d->currentAlbum); 

    updateItemRectsPixmap();
    updateBanner();
}

void AlbumIconView::refreshIcon(AlbumIconItem* item)
{
    if (!item)
        return;

    emit signalSelectionChanged();
}


void AlbumIconView::slotImageListerNewItems(const KFileItemList& itemList)
{
    KFileItem* item;

    for (KFileItemListIterator it(itemList); (item = it.current()); ++it)
    {
        if (item->isDir())
            continue;

        KURL url( item->url() );
        url.cleanPath();
        
        AlbumIconItem* iconItem = new AlbumIconItem(this, url.filename(), item);
        item->setExtraData(this, iconItem);

        d->itemDict.insert(url.url(), iconItem);
    }

    updateBanner();

    slotUpdate();

    // create a new list with items sorted according to the view
    KFileItemList fileList;
    for (ThumbItem *it = firstItem(); it; it=it->nextItem()) {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
        fileList.append(iconItem->fileItem());
    }
    
    if (d->thumbJob.isNull())
    {
        d->thumbJob = new Digikam::ThumbnailJob(fileList,
                                                (int)d->thumbSize.size(),
                                                showMetaInfo());
        connect(d->thumbJob, 
                SIGNAL(signalThumbnailMetaInfo(const KFileItem*,
                                               const QPixmap&,
                                               const KFileMetaInfo*)),
                SLOT(slotGotThumbnail(const KFileItem*,
                                      const QPixmap&,
                                      const KFileMetaInfo*)));
        connect(d->thumbJob,
                SIGNAL(signalFailed(const KFileItem*)),
                SLOT(slotFailedThumbnail(const KFileItem*)));

        connect(d->thumbJob,
                SIGNAL(signalCompleted()),
                SLOT(slotFinishedThumbnail()));
    }
    else
    {
        d->thumbJob->addItems(fileList);
        slotContentsMoving(contentsX(), contentsY());
    }
    
    emit signalItemsAdded();
}

void AlbumIconView::slotImageListerDeleteItem(KFileItem* item)
{
    if (item->isDir())
        return;

    AlbumIconItem* iconItem =
        static_cast<AlbumIconItem*>(item->extraData(this));
        
    if (!iconItem)
        return;

    if (!d->thumbJob.isNull())
        d->thumbJob->removeItem(item);

    PAlbum* album = d->imageLister->findParentAlbum(item);
    if (album)
    {
        AlbumManager::instance()->albumDB()->deleteItem(album,
                                                        item->url().fileName());
    }
    
    delete iconItem;
    item->removeExtraData(this);

    KURL u(item->url());
    u.cleanPath();
    d->itemDict.remove(u.url());

    rearrangeItems();
    updateBanner();
}

void AlbumIconView::slotImageListerClear()
{
    d->itemDict.clear();
    clear();
    emit signalSelectionChanged();
}

void AlbumIconView::slotImageListerCompleted()
{
}

void AlbumIconView::slotImageListerRefreshItems(const KFileItemList& itemList)
{
    KFileItemListIterator iterator(itemList);
    KFileItem *fileItem;

    KFileItemList newItemList;
    
    while ((fileItem = iterator.current()) != 0) {
        ++iterator;
        
        if (fileItem->isDir()) continue;
        
        if (!fileItem->extraData(this))
        {
            // hey - a new item
            newItemList.append(fileItem);
        }
        else
        {
            AlbumIconItem* iconItem =
                static_cast<AlbumIconItem*>(fileItem->extraData(this));
            iconItem->setText(fileItem->text());
            refreshIcon(iconItem);
        }
    }

    if (!newItemList.isEmpty())
        slotImageListerNewItems(newItemList);
}


void AlbumIconView::slotDoubleClicked(ThumbItem *item)
{
    if (!item) return;

    slotDisplayItem(static_cast<AlbumIconItem *>(item));

}

void AlbumIconView::slotRightButtonClicked(ThumbItem *item,
                                           const QPoint& pos)
{
    if (!item) return;

    AlbumIconItem* iconItem
        = static_cast<AlbumIconItem *>(item);

    // --------------------------------------------------------

    QValueVector<KService::Ptr> serviceVector;
    KTrader::OfferList offers =
        KTrader::self()->query(iconItem->fileItem()->mimetype(),
                               "Type == 'Application'");
    
    QPopupMenu *exifOrientationMenu = new QPopupMenu();

    exifOrientationMenu->insertItem(i18n("Normal"), 201);
    exifOrientationMenu->insertItem(i18n("Flipped horizontally"), 202);
    exifOrientationMenu->insertItem(i18n("Rotated 180 degrees"), 203);
    exifOrientationMenu->insertItem(i18n("Rotated 90 degrees / horiz. flipped"), 204);
    exifOrientationMenu->insertItem(i18n("Rotated 90 degrees"), 205);
    exifOrientationMenu->insertItem(i18n("Rotated 90 degrees / vert. flipped"), 206);
    exifOrientationMenu->insertItem(i18n("Rotated 270 degrees"), 207);

    QPopupMenu *openWithMenu = new QPopupMenu();

    KTrader::OfferList::Iterator iter;
    KService::Ptr ptr;
    int index = 100;
    
    for( iter = offers.begin(); iter != offers.end(); ++iter ) 
    {
        ptr = *iter;
        openWithMenu->insertItem( ptr->pixmap(KIcon::Small),
                                  ptr->name(), index++);
        serviceVector.push_back(ptr);
    }
    
    // --------------------------------------------------------

    QPopupMenu popmenu(this);
    popmenu.insertItem(SmallIcon("image"),
                       i18n("View/Edit"), 10);
    popmenu.insertItem(i18n("Open With ..."), openWithMenu, 11);
    popmenu.insertSeparator();
    popmenu.insertItem(SmallIcon("text_block"),
                       i18n("Edit Comments and Tags ..."), 12);
    popmenu.insertItem(SmallIcon("text_italic"),
                       i18n("View Exif Information ..."), 13);
    popmenu.insertItem(SmallIcon("text_italic"),
                       i18n("Set Exif Orientation"), exifOrientationMenu, 17);
    popmenu.insertItem(i18n("Properties ..."), 14);
    popmenu.insertSeparator();

    // Merge in the KIPI plugins actions ----------------------------

    const QPtrList<KAction>& ImageActions = DigikamApp::getinstance()->menuImageActions();
    
    QPtrListIterator<KAction> it1(ImageActions);
    KAction *action;
    bool count =0;
    
    while ( (action = it1.current()) != 0 ) 
        {
        action->plug(&popmenu);
        ++it1;
        count = 1;
        }

    // Don't insert a separator if we didn't plug in any actions
    
    if (count != 0)
        popmenu.insertSeparator();

    KActionMenu* menu = new KActionMenu(i18n("Batch processes"));  
            
    const QPtrList<KAction>& BatchActions = DigikamApp::getinstance()->menuBatchActions();

    QPtrListIterator<KAction> it2(BatchActions);
    count = 0;
    
    while ( (action = it2.current()) != 0 ) 
    {
        menu->insert(action);
        ++it2;
        count = 1;
    }

    // Don't insert a separator if we didn't plug in any actions
    
    if (count != 0)
    {
        menu->plug(&popmenu);
        popmenu.insertSeparator();
    }

    // --------------------------------------------------------

    popmenu.insertItem(SmallIcon("pencil"),
                       i18n("Rename"), 15);
    popmenu.insertItem(SmallIcon("editdelete"),
                       i18n("Delete"), 16);

    int id = popmenu.exec(pos);

    switch(id) {

    case 10: {
        slotDisplayItem(iconItem);
        break;
    }

    case 12: {
        slotEditImageComments(iconItem);
        break;
    }

    case 13: {
        slotShowExifInfo(iconItem);
        break;
    }

    case 14: {
        slotProperties(iconItem);
        break;
    }

    case 15: {
        slotRename(iconItem);
        break;
    }

    case 16: {
        slotDeleteSelectedItems();
        break;
    }

    default:
        break;
    }

    //---------------------------------------------------------------

    if( id >= 200) {
        slotSetExifOrientation( iconItem->fileItem()->url().path(), id -200 );
    } else if (id >= 100) {
        KService::Ptr imageServicePtr = serviceVector[id-100];
        KRun::run(*imageServicePtr, iconItem->fileItem()->url());
    }

    serviceVector.clear();
    delete openWithMenu;
    delete exifOrientationMenu;
}

void AlbumIconView::slotEditImageComments(AlbumIconItem* iconItem)
{
    ImageDescEdit descEdit(this, iconItem);
    descEdit.exec();

    if (d->currentAlbum && d->currentAlbum->type() == Album::TAG)
    {
        d->imageLister->updateDirectory();        
    }
    else
    {
        updateContents();
    }
}

void AlbumIconView::slotShowExifInfo(AlbumIconItem* item)
{
    if ( !item ) return;

    KExif *exif = new KExif(0);
    
    if ( exif->loadFile(item->fileItem()->url().path()) == 0 )
        exif->show();
    else 
    {
        delete exif;
        KMessageBox::sorry(0, i18n("This item has no Exif Information"));
    }
}

void AlbumIconView::slotRename(AlbumIconItem* item)
{
    if (!item)
        return;

    PAlbum* album = d->imageLister->findParentAlbum(item->fileItem());
    if (!album)
    {
        kdWarning() << "Failed to find parent album for "
                    << item->fileItem()->url().prettyURL()
                    << endl;
        return;
    }

    bool ok;
    
#if KDE_IS_VERSION(3,2,0)
    QString newName = KInputDialog::getText(i18n("Rename Item"),
                                            i18n("Enter New Name"),
                                            item->fileItem()->url().fileName(),
                                            &ok, this);
#else
    QString newName = KLineEditDlg::getText(i18n("Rename Item"),
                                            i18n("Enter New Name"),
                                            item->fileItem()->url().fileName(),
                                            &ok, this);
#endif

    if (!ok)
        return;
    
   AlbumFileCopyMove::rename(album, item->fileItem()->url().fileName(),
                             newName);
   if (d->currentAlbum && d->currentAlbum->type() == Album::TAG)
       d->imageLister->updateDirectory();
}

void AlbumIconView::slotDeleteSelectedItems()
{

    KURL::List urlList;
    QStringList nameList;

    for (ThumbItem *it = firstItem(); it; it=it->nextItem()) {
        if (it->isSelected()) {
            AlbumIconItem *iconItem =
                static_cast<AlbumIconItem *>(it);
            urlList.append(iconItem->fileItem()->url());
            nameList.append(iconItem->text());
        }
    }

    if (urlList.count() <= 0) return;

    QString warnMsg(i18n("About to delete these Image(s)\nAre you sure?"));
    if (KMessageBox::warningContinueCancelList(this,
                                               warnMsg,
                                               nameList,
                                               i18n("Warning"),
                                               i18n("Delete"))
        ==  KMessageBox::Continue)
    {

        KIO::DeleteJob* job = KIO::del(urlList, false, true);

        connect(job, SIGNAL(result(KIO::Job*)),
                this, SLOT(slotOnDeleteSelectedItemsFinished(KIO::Job*)));
    }

    // TODO: use a native delete to remove items from database if current
    // album is a tag album OR use a kdirwatch on all the items in the
    // tag album.
}

void AlbumIconView::slotOnDeleteSelectedItemsFinished(KIO::Job* job)
{
    if (job->error())
        job->showErrorDialog(this);

    if (d->currentAlbum && d->currentAlbum->type() == Album::TAG)
        d->imageLister->updateDirectory();

    updateBanner();
}
void AlbumIconView::slotFilesModified()
{
    d->imageLister->updateDirectory();
}

void AlbumIconView::slotFilesModified(const KURL& url)
{
    refreshItems(QStringList(url.path()));    
}

void AlbumIconView::slotDisplayItem(AlbumIconItem *item )
{
    AlbumSettings *settings = AlbumSettings::instance();
    
    if (!settings) return;
    
    if (!item) return;
    
    QString currentFileExtension =
        item->fileItem()->url().fileName().section( '.', -1 );
    QString imagefilter = settings->getImageFileFilter().lower() +
                          settings->getImageFileFilter().upper();

    // If the current item isn't an image file.
    if ( imagefilter.find(currentFileExtension) == -1 )     
    {
       KTrader::OfferList offers = KTrader::self()->query(item->fileItem()->mimetype(),
                                                          "Type == 'Application'");          

       if (offers.isEmpty())
           return;
       
       KService::Ptr ptr = offers.first();
       // Run the dedicated app to show the item.
       KRun::run(*ptr, item->fileItem()->url());     
       return;                                                  
    }

    // Run Digikam ImageViewer with all image files in the current Album.
    
    KURL::List urlList;

    for (ThumbItem *it = firstItem() ; it ; it = it->nextItem()) 
    {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
        QString fileExtension = iconItem->fileItem()->url().fileName().section( '.', -1 );
        
        if ( imagefilter.find(fileExtension) != -1 )
            urlList.append(iconItem->fileItem()->url());
    }

    //ImageView *view = new ImageView(0, urlList, item->fileItem()->url());
    //view->show();

    ImageWindow *imview = ImageWindow::instance();

    imview->disconnect(this);
    connect(imview, SIGNAL(signalFileAdded(const KURL&)),
            SLOT(slotFilesModified()));
    connect(imview, SIGNAL(signalFileModified(const KURL&)),
            SLOT(slotFilesModified(const KURL&)));
    connect(imview, SIGNAL(signalFileDeleted(const KURL&)),
            SLOT(slotFilesModified()));
        
    imview->loadURL(urlList, item->fileItem()->url(),
		    d->currentAlbum ? d->currentAlbum->getTitle():QString());
    if (imview->isHidden())
        imview->show();
    imview->raise();
    imview->setFocus();
}

void AlbumIconView::slotProperties(AlbumIconItem* item)
{
    if (!item) return;    

    KPropertiesDialog dlg(item->fileItem()->url(), this, 0, true, false);
    if (dlg.exec())
    {
        item->repaint();
    }
}

// ------------------------------------------------------------------------------

QString AlbumIconView::itemComments(AlbumIconItem *item)
{
    PAlbum* album = d->imageLister->findParentAlbum(item->fileItem());
    if (!album)
    {
        kdWarning() << "Failed to find parent album for "
                    << item->fileItem()->url().prettyURL()
                    << endl;
        return QString("");
    }

    AlbumDB* db  = AlbumManager::instance()->albumDB();
    return db->getItemCaption(album, item->text());
}

QStringList AlbumIconView::itemTagNames(AlbumIconItem* item)
{
    PAlbum* album = d->imageLister->findParentAlbum(item->fileItem());
    if (!album)
    {
        kdWarning() << "Failed to find parent album for "
                    << item->fileItem()->url().prettyURL()
                    << endl;
        return QString("");
    }

    AlbumDB* db  = AlbumManager::instance()->albumDB();
    return db->getItemTagNames(album, item->text());
}

AlbumIconItem* AlbumIconView::firstSelectedItem()
{
    AlbumIconItem *iconItem = 0;
    for (ThumbItem *it = firstItem(); it; it = it->nextItem())
    {
        if (it->isSelected())
        {
            iconItem = static_cast<AlbumIconItem *>(it);
            break;
        }
    }

    return iconItem;
}

AlbumLister* AlbumIconView::albumLister() const
{
    return d->imageLister;
}

const AlbumSettings* AlbumIconView::settings()
{
    return d->albumSettings;
}

ThumbnailSize AlbumIconView::thumbnailSize()
{
    return d->thumbSize;    
}

void AlbumIconView::calcBanner()
{
    QRect banner(0, 0, 0, 0);
    
    if (!d->currentAlbum)
    {
        setBannerRect(banner);
        return;
    }

    // Title --------------------------------------------------------
    
    QFont fn(font());
    int fnSize = fn.pointSize();
    bool usePointSize;
    if (fnSize > 0)
    {
        fn.setPointSize(fnSize+10);
        usePointSize = true;
    }
    else
    {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+10);
        usePointSize = false;
    }
    
    fn.setBold(true);
    QFontMetrics fm(fn);
    QRect tr = fm.boundingRect(0, 0, frameRect().width(),
                               0xFFFFFFFF, Qt::AlignLeft |
                               Qt::WordBreak | Qt::BreakAnywhere
                               | Qt::AlignVCenter,
                               d->albumTitle);
    banner.setHeight(tr.height());

    // Date and Comments --------------------------------------------
    
    if (usePointSize)
        fn.setPointSize(font().pointSize());
    else
        fn.setPixelSize(font().pixelSize());
    
    fn.setBold(false);
    fm = QFontMetrics(fn);

    QString dateAndComments(d->albumDate);
    if (!d->albumComments.isEmpty()) 
        dateAndComments += " - " + d->albumComments;

    tr = fm.boundingRect(0, 0,
                         frameRect().width() - 20,
                         0xFFFFFFFF, Qt::AlignLeft |
                         Qt::WordBreak | Qt::BreakAnywhere
                         | Qt::AlignVCenter,
                         dateAndComments);

    banner.setHeight(banner.height() + tr.height() + 20);
    banner.setWidth(frameRect().width());

    setBannerRect(banner);
}

void AlbumIconView::paintBanner(QPainter *p)
{
    QRect r(contentsRectToViewport(bannerRect()));

    if (!p || r.isEmpty() || r.isNull())
        return;

    p->save();

    QRegion oR(r);

    r.setHeight(r.height() - 5);
    p->fillRect(r, colorGroup().highlight());

    oR -= QRegion(r);
    p->save();
    p->setClipRegion(QRegion(oR));
    p->fillRect(oR.boundingRect(), colorGroup().base());
    p->restore();

    // Title --------------------------------------------------------
    
    r.setX(r.x() + 5);
    r.setY(r.y() + 5);

    QFont fn(font());
    int fnSize = fn.pointSize();
    bool usePointSize;
    if (fnSize > 0) {
        fn.setPointSize(fnSize+10);
        usePointSize = true;
    }
    else {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+10);
        usePointSize = false;
    }

    fn.setBold(true);
    p->setFont(fn);
    p->setPen(colorGroup().highlightedText());

    QRect tr;
    p->drawText(r, Qt::AlignLeft |
                Qt::WordBreak | Qt::BreakAnywhere
                | Qt::AlignTop,
                d->albumTitle, -1, &tr);

    r.setY(r.y() + tr.height() + 5);

    // Date and Comments --------------------------------------------

    if (usePointSize)
        fn.setPointSize(font().pointSize());
    else
        fn.setPixelSize(font().pixelSize());

    fn.setBold(false);
    p->setFont(fn);
 
    QString dateAndComments(d->albumDate);
    if (!d->albumComments.isEmpty()) 
        dateAndComments += " - " + d->albumComments;

    p->drawText(r, Qt::AlignLeft |
                Qt::WordBreak | Qt::BreakAnywhere
                | Qt::AlignVCenter,
                dateAndComments);
    
    p->restore();
}

void AlbumIconView::updateBanner()
{
    if (!d->currentAlbum)
    {
        setBannerRect(QRect(0, 0, 0, 0));
        return;
    }

    d->albumTitle = d->currentAlbum->getTitle();
    d->albumComments = "";
    QDate date;

    if (d->currentAlbum->type() == Album::PHYSICAL)
    {
        PAlbum* album = dynamic_cast<PAlbum*>(d->currentAlbum);
        
        d->albumComments = album->getCaption();
        date             = album->getDate();

#if KDE_IS_VERSION(3,2,0)
        d->albumDate = i18n("%1 %2 - 1 Item", "%1 %2 - %n Items", count())
                       .arg(KGlobal::locale()->calendar()->monthName(date, false))
                       .arg(KGlobal::locale()->calendar()->year(date));
#else
        d->albumDate = i18n("%1 %2 - 1 Item", "%1 %2 - %n Items", count())
                       .arg(KGlobal::locale()->monthName(date.month()))
                       .arg(QString::number(date.year()));
#endif
    }
    else if (d->currentAlbum->type() == Album::TAG)
    {
        d->albumComments = (static_cast<TAlbum*>(d->currentAlbum))->getURL();
        d->albumDate     = i18n("1 Item", "%n Items", count());
    }
    else
    {
        d->albumComments = QString("");
        d->albumDate     = i18n("1 Item", "%n Items", count());
    }
    
    calcBanner();
    repaintBanner();
}

// -- DnD ---------------------------------------------------

void AlbumIconView::startDrag()
{
    if (!d->currentAlbum)
        return;

    KURL::List      urls;
    QValueList<int> dirIDs;

    
    for (ThumbItem *it = firstItem(); it; it=it->nextItem()) {
        if (it->isSelected())
        {
            AlbumIconItem *albumItem = static_cast<AlbumIconItem *>(it);
            urls.append(albumItem->fileItem()->url());
            PAlbum* album = d->imageLister->findParentAlbum(albumItem->fileItem());
            if (album)
                dirIDs.append(album->getID());
        }
    }

    if (urls.isEmpty())
        return;

    if (urls.size() != dirIDs.size())
    {
        kdWarning() << "Mismatch between sizes of lists for urls and dirids"
                    << endl;
        return;
    }
        
    QPixmap icon(DesktopIcon("image", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4,h+4);
    QString text(QString::number(urls.count()));
    
    QPainter p(&pix);
    p.fillRect(0, 0, w+4, h+4, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, w+4, h+4);
    p.drawPixmap(2, 2, icon);
    QRect r = p.boundingRect(2,2,w,h,Qt::AlignLeft|Qt::AlignTop,text);
    r.setWidth(QMAX(r.width(),r.height()));
    r.setHeight(QMAX(r.width(),r.height()));
    p.fillRect(r, QColor(0,80,0));
    p.setPen(Qt::white);
    QFont f(font());
    f.setBold(true);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, text);
    p.end();

    QDragObject* drag = 0;

    if (d->currentAlbum->type() == Album::PHYSICAL)
        drag = new AlbumItemsDrag(urls, dirIDs, this);
    else if (d->currentAlbum->type() == Album::TAG)
        drag = new TagItemsDrag(urls, dirIDs, this);

    if (drag)
    {
        drag->setPixmap(pix);
        drag->dragCopy();
    }
}

void AlbumIconView::contentsDragMoveEvent(QDragMoveEvent *event)
{
    if (!d->currentAlbum || (!QUriDrag::canDecode(event) &&
                             !CameraDragObject::canDecode(event))
        || event->source() == this) {
        event->ignore();
        return;
    }
    event->accept();
}

void AlbumIconView::contentsDropEvent(QDropEvent *event)
{
    
    if (!d->currentAlbum || (!QUriDrag::canDecode(event) &&
                             !CameraDragObject::canDecode(event))
        || event->source() == this) {
        event->ignore();
        return;
    }

    if (QUriDrag::canDecode(event)) {

        KURL destURL(d->currentAlbum->getURL());

        KURL::List srcURLs;
        KURLDrag::decode(event, srcURLs);
        
        QPopupMenu popMenu(this);
        popMenu.insertItem( i18n("&Copy"), 10 );
        popMenu.insertItem( i18n("&Move"), 11 );

        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());
        switch(id) {
        case 10: {
            KIO::copy(srcURLs,destURL,true);
            break;
        }
        case 11: {
            KIO::move(srcURLs,destURL,true);
            break;
        }
        default:
            break;
        }
    }
    else if (CameraDragObject::canDecode(event)) {

        QPopupMenu popMenu(this);
        popMenu.insertItem( i18n("&Download"), 10 );
        popMenu.setMouseTracking(true);

        int id = popMenu.exec(QCursor::pos());
        switch(id) {
        case 10: {

            CameraType ctype;
            CameraDragObject::decode(event, ctype);

            QByteArray arg1;
            QDataStream stream1(arg1, IO_WriteOnly);
            stream1 << d->currentAlbum->getTitle();
        
            DCOPClient *client = kapp->dcopClient();
            client->send("digikamcameraclient", "DigikamCameraClient",
                         "cameraChangeDownloadAlbum(QString)",
                         arg1);
            
            QByteArray arg2;

            client->send("digikamcameraclient", "DigikamCameraClient",
                         "cameraDownloadSelected()",
                         arg2);
            break;
        }
        default:
            break;
        }

    }
}

// make sure that we load thumbnail for items which are visible first
void AlbumIconView::slotContentsMoving(int x, int y)
{
    if (d->thumbJob.isNull())
        return;
    QRect r(x, y, visibleWidth(), visibleHeight());
    ThumbItem *fItem = findFirstVisibleItem(r);
    ThumbItem *lItem = findLastVisibleItem(r);
    if (!fItem || !lItem)
        return;
    AlbumIconItem* firstItem = static_cast<AlbumIconItem*>(fItem);
    AlbumIconItem* lastItem  = static_cast<AlbumIconItem*>(lItem);
    AlbumIconItem* item = firstItem;
    while (item) {
        if (d->thumbJob->setNextItemToLoad(item->fileItem()))
            return;
        if (item == lastItem)
            return;
        item = (AlbumIconItem*)item->nextItem();
    }
}

QStringList AlbumIconView::allItems()
{
    QStringList itemList;

    for (ThumbItem *it = firstItem(); it;
         it = it->nextItem()) {
        itemList.append(it->text());
    }

    return itemList;
}

QStringList AlbumIconView::selectedItems()
{
    QStringList itemList;

    for (ThumbItem *it = firstItem(); it;
         it = it->nextItem()) {
        if (it->isSelected()) 
            itemList.append(it->text());
    }

    return itemList;
}

QStringList AlbumIconView::allItemsPath()
{
    QStringList itemList;

    for (ThumbItem *it = firstItem(); it;
         it = it->nextItem()) {
        AlbumIconItem *item = (AlbumIconItem*) it;
        itemList.append(item->fileItem()->url().path());
    }

    return itemList;
}

QStringList AlbumIconView::selectedItemsPath()
{
    QStringList itemList;

    for (ThumbItem *it = firstItem(); it;
         it = it->nextItem()) {
        if (it->isSelected()) {
            AlbumIconItem *item = (AlbumIconItem*) it;
            itemList.append(item->fileItem()->url().path());
        }
    }

    return itemList;
}

void AlbumIconView::refresh()
{
    if (!d->thumbJob.isNull())
        d->thumbJob->kill();
    d->imageLister->stop();
    d->itemDict.clear();
    clear();
    
    d->imageLister->openAlbum(d->currentAlbum);
}

void AlbumIconView::refreshItems(const QStringList& itemList)
{
    if (!d->currentAlbum || itemList.empty())
        return;

    KFileItemList fList;
    for ( QStringList::const_iterator it = itemList.begin();
          it != itemList.end(); ++it )
    {
        KURL u(*it);
        u.cleanPath();

        AlbumIconItem* iconItem = findItem(u.url());
        if (iconItem)
            fList.append(iconItem->fileItem());
    }
        
    if (fList.isEmpty())
        return;

    if (d->thumbJob.isNull())
    {
        d->thumbJob =
            new Digikam::ThumbnailJob(fList, (int)d->thumbSize.size());
        connect(d->thumbJob, 
                SIGNAL(signalThumbnailMetaInfo(const KFileItem*,
                                               const QPixmap&,
                                               const KFileMetaInfo*)),
                SLOT(slotGotThumbnail(const KFileItem*,
                                      const QPixmap&,
                                      const KFileMetaInfo*)));
        connect(d->thumbJob,
                SIGNAL(signalFailed(const KFileItem*)),
                SLOT(slotFailedThumbnail(const KFileItem*)));
        connect(d->thumbJob,
                SIGNAL(signalCompleted()),
                SLOT(slotFinishedThumbnail()));
    }
    else
    {
        d->thumbJob->addItems(fList);
    }
}

void AlbumIconView::slotGotThumbnail(const KFileItem* fileItem, const QPixmap& pix,
                                     const KFileMetaInfo* metaInfo)
{
   AlbumSettings *settings = AlbumSettings::instance();

   if (!settings || !fileItem)
       return;

   AlbumIconItem *iconItem = (AlbumIconItem *)fileItem->extraData(this);
   if (!iconItem)
       return;

   if(settings->getExifRotate())
   {
      QPixmap rotPix(pix);
      exifRotate(fileItem->url().path(), rotPix);
      iconItem->setPixmap(rotPix, metaInfo);
   }
   else
   {
      iconItem->setPixmap(pix, metaInfo);
   }
}

// If we failed to generate a thumbnail using our thumbnail generator
// use kde thumbnail generator to generate one

void AlbumIconView::slotFailedThumbnail(const KFileItem* item)
{
    KURL::List urlList;	
    urlList.append(item->url());
    
    KIO::PreviewJob* job = KIO::filePreview(urlList, (int)d->thumbSize.size());
                                            
    connect(job, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)),
            SLOT(slotGotThumbnailKDE(const KFileItem*, const QPixmap&)));
    connect(job, SIGNAL(failed(const KFileItem*)),
            SLOT(slotFailedThumbnailKDE(const KFileItem*)));
}

void AlbumIconView::slotGotThumbnailKDE(const KFileItem* item, const QPixmap& pix)
{
    AlbumIconItem* iconItem = findItem(item->url().url());
    if (!iconItem)
        return;
    
    slotGotThumbnail(iconItem->fileItem(), pix, 0);
}

void AlbumIconView::slotFailedThumbnailKDE(const KFileItem* item)
{
    AlbumIconItem* iconItem = findItem(item->url().url());
    if (!iconItem)
        return;

    QString dir = KGlobal::dirs()->findResourceDir("digikam_imagebroken",
                                                   "image_broken.png");
    dir = dir + "/image_broken.png";

    int size = (int)d->thumbSize.size();
    
    QImage img(dir);
    img = img.smoothScale(size, size);

    slotGotThumbnail(iconItem->fileItem(), QPixmap(img), 0);
}

void AlbumIconView::slotFinishedThumbnail()
{
    if (!d->thumbJob.isNull())
        delete d->thumbJob;    
}

void AlbumIconView::slotSelectionChanged()
{
    if (firstSelectedItem())
        emitItemsSelected(true);
    else
        emitItemsSelected(false);
}

void AlbumIconView::focusInEvent(QFocusEvent *)
{
    unsetPalette();
}

void AlbumIconView::focusOutEvent(QFocusEvent *)
{
    QPalette plt(palette());
    QColorGroup cg(plt.active());
    cg.setColor(QColorGroup::Base, QColor(245,245,245));
    plt.setActive(cg);
    plt.setInactive(cg);
    setPalette(plt);
}

void AlbumIconView::slotSetExifOrientation( const QString filename, int orientation )
{
    KExifData::ImageOrientation o = (KExifData::ImageOrientation)orientation;

    KExifUtils::writeOrientation(filename, o);

    refreshItems(filename); 
}

void AlbumIconView::exifRotate(QString filename, QPixmap& pixmap)
{
   // Rotate thumbnail based on EXIF rotate tag
    QWMatrix matrix;

    KExifData *exifData = new KExifData;

    if(!exifData->readFromFile(filename)) return;

    KExifData::ImageOrientation orientation = exifData->getImageOrientation();

    bool doXform = (orientation != KExifData::NORMAL);

    switch (orientation) {
       case KExifData::NORMAL:
       case KExifData::UNSPECIFIED:
          break;

       case KExifData::HFLIP:
          matrix.scale(-1,1);
          break;

       case KExifData::ROT_180:
          matrix.rotate(180);
          break;

       case KExifData::VFLIP:
          matrix.scale(1,-1);
          break;

       case KExifData::ROT_90_HFLIP:
          matrix.scale(-1,1);
          matrix.rotate(90);
          break;

       case KExifData::ROT_90:
          matrix.rotate(90);
          break;

       case KExifData::ROT_90_VFLIP:
          matrix.scale(1,-1);
          matrix.rotate(90);
          break;

       case KExifData::ROT_270:
          matrix.rotate(270);
          break;
    }

    //transform accordingly
    if ( doXform )
       pixmap = pixmap.xForm( matrix );

    delete exifData;

}


QRect AlbumIconView::itemRect() const
{
    return d->itemRect;    
}

QRect AlbumIconView::itemDateRect() const
{
    return d->itemDateRect;    
}

QRect AlbumIconView::itemPixmapRect() const
{
    return d->itemPixmapRect;
}

QRect AlbumIconView::itemNameRect() const
{
    return d->itemNameRect;
}

QRect AlbumIconView::itemCommentsRect() const
{
    return d->itemCommentsRect;
}

QRect AlbumIconView::itemFileCommentsRect() const
{
    return d->itemFileCommentsRect;    
}

QRect AlbumIconView::itemResolutionRect() const
{
    return d->itemResolutionRect;
}

QRect AlbumIconView::itemTagRect() const
{
    return d->itemTagRect;
}


QRect AlbumIconView::itemSizeRect() const
{
    return d->itemSizeRect;
}

QPixmap* AlbumIconView::itemBaseRegPixmap() const
{
    return &d->itemRegPixmap;
}

QPixmap* AlbumIconView::itemBaseSelPixmap() const
{
    return &d->itemSelPixmap;
}

QFont AlbumIconView::itemFontReg() const
{
    return d->fnReg;    
}

QFont AlbumIconView::itemFontCom() const
{
    return d->fnCom;    
}

QFont AlbumIconView::itemFontXtra() const
{
    return d->fnXtra;
}

void AlbumIconView::updateItemRectsPixmap()
{
    d->itemRect = QRect(0,0,0,0);
    d->itemDateRect = QRect(0,0,0,0);
    d->itemPixmapRect = QRect(0,0,0,0);
    d->itemNameRect = QRect(0,0,0,0);
    d->itemCommentsRect = QRect(0,0,0,0);
    d->itemResolutionRect = QRect(0,0,0,0);
    d->itemSizeRect = QRect(0,0,0,0);
    d->itemTagRect = QRect(0,0,0,0);

    d->fnReg  = font();
    d->fnCom  = font();
    d->fnXtra = font();
    d->fnCom.setItalic(true);

    int fnSz = d->fnReg.pointSize();
    if (fnSz > 0)
    {
        d->fnCom.setPointSize(fnSz-1);
        d->fnXtra.setPointSize(fnSz-2);
    }
    else
    {
        fnSz = d->fnReg.pixelSize();
        d->fnCom.setPixelSize(fnSz-1);
        d->fnXtra.setPixelSize(fnSz-2);
    }

    int margin  = 2;
    int w = QMAX(d->thumbSize.size(), 100) + 2*margin;

    QFontMetrics fm(d->fnReg);
    QRect oneRowRegRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fnCom);
    QRect oneRowComRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fnXtra);
    QRect oneRowXtraRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                           Qt::AlignTop | Qt::AlignHCenter,
                                           "XXXXXXXXX");

    int y = margin;

    d->itemPixmapRect = QRect(margin, y, w, d->thumbSize.size()+margin);
    y = d->itemPixmapRect.bottom();

    if (d->albumSettings->getIconShowName())
    {
        d->itemNameRect = QRect(margin, y, w, oneRowRegRect.height());
        y = d->itemNameRect.bottom();
    }

    if (d->albumSettings->getIconShowComments())
    {
        d->itemCommentsRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->itemCommentsRect.bottom();
    }

    if (d->albumSettings->getIconShowFileComments())
    {
        d->itemFileCommentsRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->itemFileCommentsRect.bottom();
    }

    if (d->albumSettings->getIconShowDate())
    {
        d->itemDateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemDateRect.bottom();
    }

    if (d->albumSettings->getIconShowResolution())
    {
        d->itemResolutionRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemResolutionRect.bottom() ;
    }
    
    if (d->albumSettings->getIconShowSize())
    {
        d->itemSizeRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemSizeRect.bottom();
    }

    if (d->albumSettings->getIconShowTags())
    {
        d->itemTagRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->itemTagRect.bottom();
    }

    d->itemRect = QRect(0, 0, w+2*margin, y+margin);

    d->itemRegPixmap.resize(d->itemRect.width(), d->itemRect.height());
    d->itemRegPixmap.fill(Qt::white);

    QRegion reg(d->itemRect);
    reg -= d->itemPixmapRect;

    {
        QPainter p(&d->itemRegPixmap);
        p.setClipRegion(reg);
        p.fillRect(d->itemRect, QColor("#ededed"));
        p.setPen(QColor("#fdfdfd"));
        p.drawLine(0,0,0,d->itemRect.height()-1);
        p.drawLine(0,0,d->itemRect.width()-1,0);
        p.setPen(QColor("#adadad"));
        p.drawLine(0,d->itemRect.height()-1,d->itemRect.width()-1,d->itemRect.height()-1);
        p.drawLine(d->itemRect.width()-1,0,d->itemRect.width()-1,d->itemRect.height()-1);
        p.end();
    }

    d->itemSelPixmap.resize(d->itemRect.width(), d->itemRect.height());
    d->itemSelPixmap.fill(Qt::white);

    {
        QPainter p(&d->itemSelPixmap);
        p.setClipRegion(reg);
        p.fillRect(d->itemRect, colorGroup().highlight());
        p.setPen(QColor("#666666"));
        p.drawRect(d->itemRect);
        p.end();
    }
}


bool AlbumIconView::showMetaInfo()
{
    return (d->albumSettings->getIconShowResolution() ||
            d->albumSettings->getIconShowFileComments());
}

AlbumIconItem* AlbumIconView::findItem(const QString& url) const
{
    return d->itemDict.find(url);    
}

#include "albumiconview.moc"
