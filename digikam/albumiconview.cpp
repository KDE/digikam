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

// KDE includes.

#include <kio/previewjob.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kdirlister.h>
#include <kurl.h>
#include <kurldrag.h>
#include <klocale.h>
#include <kcalendarsystem.h>
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
#include <kexifdata.h>

// Local includes.

#include "albuminfo.h"
#include "albummanager.h"
#include "thumbnailjob.h"
#include "digikamio.h"

#include "albumsettings.h"
#include "imagedescedit.h"
#include "kexif.h"
#include "imageview.h"
#include "thumbnailsize.h"
#include "digikampluginmanager.h"

#include "cameratype.h"
#include "cameradragobject.h"

#include "albumiconitem.h"
#include "digikamapp.h"
#include "albumiconview.h"

class AlbumIconViewPrivate 
{
public:

    void init() 
        {
        imageLister   = 0;
        currentAlbum  = 0;
        albumSettings = 0;
        }
    
    KDirLister          *imageLister;
    Digikam::AlbumInfo  *currentAlbum;
    const AlbumSettings *albumSettings;
    QGuardedPtr<Digikam::ThumbnailJob> thumbJob;

    ThumbnailSize thumbSize;

    QString albumTitle;
    QString albumDate;
    QString albumComments;

    QString itemRenamedOld;
    QString itemRenamedNew;
};


AlbumIconView::AlbumIconView(QWidget* parent)
             : ThumbView(parent) 
{
    d = new AlbumIconViewPrivate;
    d->init();
    d->imageLister = new KDirLister();

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    // -- ImageLister connections -------------------------------------

    connect(d->imageLister, SIGNAL(newItems(const KFileItemList&)),
            this, SLOT(slotImageListerNewItems(const KFileItemList&)));

    connect(d->imageLister, SIGNAL(deleteItem(KFileItem*)),
            this, SLOT(slotImageListerDeleteItem(KFileItem*)) );

    connect(d->imageLister, SIGNAL(clear()),
            this, SLOT(slotImageListerClear()));

    connect(d->imageLister, SIGNAL(completed()),
            this, SLOT(slotImageListerCompleted()));

    connect(d->imageLister, SIGNAL(refreshItems(const KFileItemList&)),
            this, SLOT(slotImageListerRefreshItems(const KFileItemList&)));

    // -- Icon connections --------------------------------------------

    connect(this, SIGNAL(signalDoubleClicked(ThumbItem *)),
            this, SLOT(slotDoubleClicked(ThumbItem *)));
            
    connect(this, SIGNAL(signalReturnPressed(ThumbItem *)),
            this, SLOT(slotDoubleClicked(ThumbItem *)));
            
    connect(this, SIGNAL(signalRightButtonClicked(ThumbItem *, const QPoint &)),
            this, SLOT(slotRightButtonClicked(ThumbItem *, const QPoint &)));
            
    connect(this, SIGNAL(signalItemRenamed(ThumbItem *)),
            this, SLOT(slotItemRenamed(ThumbItem *)));
            
    connect(this, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));
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
    if (!settings) return;
    
    d->albumSettings = settings;

    d->imageLister->setNameFilter(d->albumSettings->getImageFileFilter() + " " + 
                                  d->albumSettings->getMovieFileFilter() + " " +
                                  d->albumSettings->getAudioFileFilter() + " " +
                                  d->albumSettings->getRawFileFilter());
    
    ThumbnailSize thumbSize((ThumbnailSize::Size)
                            d->albumSettings->getDefaultIconSize()); 

    setThumbnailSize(thumbSize);

    setUpdatesEnabled(false);
    viewport()->setUpdatesEnabled(false);

    for (ThumbItem *it = firstItem(); it ; it=it->nextItem()) 
        {
        AlbumIconItem *item = static_cast<AlbumIconItem *>(it);
        item->updateExtraText();
        item->calcRect();
        }

    setUpdatesEnabled(true);
    viewport()->setUpdatesEnabled(true);

    slotUpdate();
}


void AlbumIconView::albumDescChanged()
{
    updateBanner();
}

void AlbumIconView::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    if ( d->thumbSize != thumbSize) {

        if (!d->thumbJob.isNull())
            d->thumbJob->kill();

        d->imageLister->stop();
        clear();

        d->thumbSize = thumbSize;

        KURL url;
        if (d->currentAlbum)
            url = KURL(d->currentAlbum->getPath());

        if (url.isValid())
            d->imageLister->openURL(url);
    }
}

void AlbumIconView::setAlbum(Digikam::AlbumInfo* album)
{
    if (!album) 
        {
        d->currentAlbum = 0;
        clear();
        return;
        }
    
    if (d->currentAlbum == album) return;

    d->imageLister->stop();
    
    if (!d->thumbJob.isNull())
        d->thumbJob->kill();
    
    d->currentAlbum = album;

    if (KURL(album->getPath()).isValid()) 
        {
        d->imageLister->openURL(KURL(album->getPath())); 
        }

    updateBanner();
}

void AlbumIconView::refreshIcon(AlbumIconItem* item)
{
    if (!item) return;

//     d->thumbGen->addFile(item->fileItem()->url().path(),
//                          (int) d->thumbSize.size());
    emit signalSelectionChanged();
}


void AlbumIconView::slotImageListerNewItems(const KFileItemList& itemList)
{


    QPixmap thumbnail(d->thumbSize.size(), d->thumbSize.size());
    QPainter painter(&thumbnail);
    painter.fillRect(0, 0, d->thumbSize.size(),
                     d->thumbSize.size(),
                     QBrush(colorGroup().base()));
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, d->thumbSize.size(), d->thumbSize.size());
    painter.end();

    KFileItem* item;

    for (KFileItemListIterator it(itemList);
         (item = it.current()); ++it) {

        if (item->isDir()) continue;

        AlbumIconItem* iconItem =         
            new AlbumIconItem(this, item->url().filename(),
                              thumbnail, d->thumbSize.size(),
                              item);
        item->setExtraData(this, iconItem);
    }

    updateBanner();

    slotUpdate();

    KURL::List urlList;
    
    for (ThumbItem *it = firstItem(); it; it=it->nextItem()) {
        AlbumIconItem* iconItem = static_cast<AlbumIconItem *>(it);
        urlList.append(iconItem->fileItem()->url());
    }

    if (d->thumbJob.isNull()) {
        d->thumbJob =
            new Digikam::ThumbnailJob(urlList,
                                      (int)d->thumbSize.size());
        connect(d->thumbJob,
                SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));
                
        connect(d->thumbJob,
                SIGNAL(signalFailed(const KURL&)),
                SLOT(slotFailedThumbnail(const KURL&)));
    }
    else
        d->thumbJob->addItems(urlList);
    
    emit signalItemsAdded();
}

void AlbumIconView::slotImageListerDeleteItem(KFileItem* item)
{
    if (item->isDir()) return;

    AlbumIconItem* iconItem =
        static_cast<AlbumIconItem*>(item->extraData(this));
        
    if (!iconItem) return;

    if (d->currentAlbum)
        d->currentAlbum->deleteItemComments(iconItem->text());

    delete iconItem;
    item->removeExtraData(this);
    rearrangeItems();
    updateBanner();
}

void AlbumIconView::slotImageListerClear()
{
    clear();
    emit signalSelectionChanged();
}

void AlbumIconView::slotImageListerCompleted()
{

}

void AlbumIconView::slotImageListerRefreshItems(const
                                                 KFileItemList& itemList)
{
    KFileItemListIterator iterator(itemList);
    KFileItem *fileItem;

    KFileItemList newItemList;
    
    while ((fileItem = iterator.current()) != 0) {
        ++iterator;
        
        if (fileItem->isDir()) continue;
        
        if (!fileItem->extraData(this)) {
            // hey - a new item
            newItemList.append(fileItem);
        }
        else {
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
                       i18n("Edit Comments ..."), 12);
    popmenu.insertItem(SmallIcon("text_italic"),
                       i18n("View Exif Information ..."), 13);
    popmenu.insertItem(SmallIcon("text_italic"),
                       i18n("Set Exif Orientation ..."), exifOrientationMenu, 17);
    popmenu.insertItem(i18n("Properties"), 14);
    popmenu.insertSeparator();

    // Merge in the plugin actions ----------------------------

#ifdef HAVE_KIPI
    const QPtrList<KAction>& mergeActions = DigikamApp::getinstance()->menuMergeActions();
#else
    const QPtrList<KAction>& mergeActions = DigikamPluginManager::instance()->menuMergeActions();
#endif
    
    QPtrListIterator<KAction> it(mergeActions);
    KAction *action;
    bool count = 0;
    
    while ( (action = it.current()) != 0 ) 
    {
        action->plug(&popmenu);
        ++it;
        count++;
    }

    // Don't insert a separator if we didn't plug in any actions
    
    if (count != 0)
        popmenu.insertSeparator();
    
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
        slot_editImageComments(iconItem);
        break;
    }

    case 13: {
        slot_showExifInfo(iconItem);
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
        slot_deleteSelectedItems();
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

void AlbumIconView::slot_editImageComments(AlbumIconItem* iconItem)
{
    AlbumSettings *settings = AlbumSettings::instance();

    if (!settings) return;

    if (!iconItem || !d->currentAlbum) return;
    
    QString comments(d->currentAlbum->getItemComments(iconItem->text()));
    
    if (ImageDescEdit::editComments(iconItem->text(), comments)) {

        d->currentAlbum->setItemComments(iconItem->text(), comments);
        
       // store as JPEG Exif comment
        QString fileName(iconItem->fileItem()->url().path());
        KFileMetaInfo metaInfo(fileName, "image/jpeg",KFileMetaInfo::Fastest);

        if(settings->getSaveExifComments() && metaInfo.isValid () && metaInfo.mimeType() == "image/jpeg")
        {
            // set Jpeg comment
            if (metaInfo.containsGroup("Jpeg EXIF Data"))
            {
                kdDebug() << "Contains JPEG Exif data, setting comment" << endl;
                metaInfo["Jpeg EXIF Data"].item("Comment").setValue(comments);
                metaInfo.applyChanges();
            }

            // set EXIF UserComment
            KExifData *exifData = new KExifData;
            exifData->writeComment(fileName,comments);
            delete exifData;
        }

        int h = iconItem->height();
        iconItem->updateExtraText();
        iconItem->calcRect();
        iconItem->repaint();
        if (iconItem->height() != h)
            rearrangeItems();

    }
}

void AlbumIconView::slot_showExifInfo(AlbumIconItem* item)
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
    if (!item) return;
    item->rename();
}

void AlbumIconView::slotItemRenamed(ThumbItem *item)
{
    if (!item) return;

    AlbumIconItem *albumItem =
        static_cast<AlbumIconItem *>(item);
    if (!albumItem) return;

    QString oldName(albumItem->fileItem()->url().fileName());
    QString newName(albumItem->text());

    albumItem->setText(oldName);

    ThumbItem *existingItem = findItem(newName);

    if (DigikamIO::rename(d->currentAlbum, oldName, newName)) {
        if (existingItem) {
            ((AlbumIconItem*)existingItem)->updateExtraText();
            ((AlbumIconItem*)existingItem)->calcRect();
            ((AlbumIconItem*)existingItem)->repaint();
        }
    }
}

void AlbumIconView::slot_deleteSelectedItems()
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
        ==  KMessageBox::Continue) {

       KIO::DeleteJob* job = KIO::del(urlList, false, true);

       connect(job, SIGNAL(result(KIO::Job*)),
               this, SLOT(slot_onDeleteSelectedItemsFinished(KIO::Job*)));
       }
}

void AlbumIconView::slot_onDeleteSelectedItemsFinished(KIO::Job* job)
{
    if (job->error())
        job->showErrorDialog(this);

    updateBanner();
}

void AlbumIconView::slotDisplayItem(AlbumIconItem *item )
{
    AlbumSettings *settings = AlbumSettings::instance();
    
    if (!settings) return;
    
    if (!item) return;
    
    QString currentFileExtension = item->fileItem()->url().fileName().section( '.', -1 );
    QString imagefilter = settings->getImageFileFilter().lower() + settings->getImageFileFilter().upper();

    if ( imagefilter.find(currentFileExtension) == -1 )     // If the current item isn't an image file.
       {
       KTrader::OfferList offers = KTrader::self()->query(item->fileItem()->mimetype(),
                                                          "Type == 'Application'");          
       
       KService::Ptr ptr = offers.first();
       KRun::run(*ptr, item->fileItem()->url());     // Run the dedicaced app for to show the item.
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
               
    ImageView *view = new ImageView(0, urlList, item->fileItem()->url());
    view->show();
}

void AlbumIconView::slotProperties(AlbumIconItem* item)
{
    if (!item) return;    

    (void)new KPropertiesDialog(item->fileItem()->url());
}

// ---------------------------------------------------------------

void AlbumIconView::getItemComments(const QString& itemName,
                                    QString& comments)
{
    if (!d->currentAlbum) return;
    comments = d->currentAlbum->getItemComments(itemName);
}

AlbumIconItem* AlbumIconView::firstSelectedItem()
{
    AlbumIconItem *iconItem = 0;
    for (ThumbItem *it = firstItem(); it; it = it->nextItem()) {
        if (it->isSelected()) {
            iconItem = static_cast<AlbumIconItem *>(it);
            break;
        }
    }

    return iconItem;
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
    
    if (!d->currentAlbum) {
        setBannerRect(banner);
        return;
    }

    // Title --------------------------------------------------------
    
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
    if (!d->currentAlbum) {
        setBannerRect(QRect(0, 0, 0, 0));
        return;
    }

    d->albumTitle = d->currentAlbum->getTitle();
    d->albumComments = d->currentAlbum->getComments();

    QDate date = d->currentAlbum->getDate();

    d->albumDate = i18n("%1 %2 - %3 Items")
                   .arg(KGlobal::locale()->calendar()->monthName(date, true))
                   .arg(KGlobal::locale()->calendar()->year(date))
                   .arg(QString::number(count()));

    calcBanner();
    repaintBanner();
}

// -- DnD ---------------------------------------------------

void AlbumIconView::startDrag()
{
    KURL::List urlList;

    for (ThumbItem *it = firstItem(); it; it=it->nextItem()) {
        if (it->isSelected()) {
            AlbumIconItem *albumItem =
                static_cast<AlbumIconItem *>(it);
            urlList.append(albumItem->fileItem()->url());
        }
    }

    if (urlList.isEmpty()) return;

    QPixmap icon(DesktopIcon("image", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4,h+4);
    QString text(QString::number(urlList.count()));
    
    QPainter p(&pix);
    p.fillRect(0, 0, w+4, h+4, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 2));
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

    KURLDrag* drag = new KURLDrag( urlList, this);
    drag->setPixmap(pix);
    drag->dragCopy();
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

        KURL destURL(d->currentAlbum->getPath());

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
    clear();
    
    KURL url;
    if (d->currentAlbum)
        url = KURL(d->currentAlbum->getPath());
    
    if (url.isValid()) {
        d->imageLister->openURL(url);
    }
}

void AlbumIconView::refreshItems(const QStringList& itemList)
{
    if (!d->currentAlbum || itemList.empty()) return;
    
    KURL::List urlList;
    for ( QStringList::const_iterator it = itemList.begin();
          it != itemList.end(); ++it ) {
        urlList.append(d->currentAlbum->getPath() + QString("/")
                       + (*it));
    }

    if (d->thumbJob.isNull()) {

        d->thumbJob =
            new Digikam::ThumbnailJob(urlList,
                                      (int)d->thumbSize.size());
        connect(d->thumbJob,
                SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));
    }
    else
        d->thumbJob->addItems(urlList);
}

void AlbumIconView::slotGotThumbnail(const KURL& url, const QPixmap& pix)
{
   AlbumSettings *settings = AlbumSettings::instance();

   if (!settings) return;

   ThumbItem *item = findItem(url.filename());
   if (!item) return;
   AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(item);

   if(settings->getExifRotate()) {
      QPixmap rotPix(pix);
      exifRotate(url.path(), rotPix);
      iconItem->setPixmap(rotPix);
   } else {
      iconItem->setPixmap(pix);
   }
}

// If we failed to generate a thumbnail using our thumbnail generator
// use kde thumbnail generator to generate one

void AlbumIconView::slotFailedThumbnail(const KURL& url)
{
    KIO::PreviewJob* job = KIO::filePreview(KURL::List(url),
                                            (int)d->thumbSize.size());
                                            
    connect(job, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)),
            SLOT(slotGotThumbnailKDE(const KFileItem*, const QPixmap&)));
    connect(job, SIGNAL(failed(const KFileItem*)),
            SLOT(slotFailedThumbnailKDE(const KFileItem*)));            
}

void AlbumIconView::slotGotThumbnailKDE(const KFileItem* item, const QPixmap& pix)
{
    slotGotThumbnail(item->url(), pix);    
}

// If we failed to generate a thumbnail using kde thumbnail generator, 
// use a broken image instead.

void AlbumIconView::slotFailedThumbnailKDE(const KFileItem* item)
{
    QImage img;
    QPixmap pix;
    KGlobal::dirs()->addResourceType("digikam_imagebroken", KGlobal::dirs()->kde_default("data") 
                                                            + "digikam/data");
    QString dir = KGlobal::dirs()->findResourceDir("digikam_imagebroken", "image_broken.png");
    dir = dir + "image_broken.png";
    img.load(dir);
    const QImage scaleImg(img.smoothScale( (int)d->thumbSize.size(), (int)d->thumbSize.size() ));
    pix.convertFromImage(scaleImg);
    slotGotThumbnail(item->url(), pix);    
}

void AlbumIconView::slotSelectionChanged()
{
    if (firstSelectedItem())
        emitItemsSelected(true);
    else
        emitItemsSelected(false);
}

bool AlbumIconView::eventFilter(QObject *obj, QEvent *ev)
{
    return ThumbView::eventFilter(obj, ev);    
}

void AlbumIconView::slotSetExifOrientation( const QString filename, int orientation )
{
    KExifData::ImageOrientation o = (KExifData::ImageOrientation)orientation;

    KExifData *exifData = new KExifData;
    exifData->writeOrientation(filename, o);
    delete exifData;

    emit 
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

       case KExifData::VFlip:
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


#include "albumiconview.moc"
