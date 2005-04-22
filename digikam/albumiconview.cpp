/* ============================================================
 * Author: Renchi Raju <renchi at pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2002-02-21
 * Description :
 *
 * Copyright 2002-2004 by Renchi Raju and Gilles Caulier
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

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qpixmap.h>
#include <qimage.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qguardedptr.h>
#include <qdragobject.h>
#include <qcursor.h>
#include <qvaluevector.h>
#include <qptrlist.h>
#include <qdict.h>
#include <qdatastream.h>
#include <qtimer.h>

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
#include <kiconeffect.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(3,2,0)
#include <kcalendarsystem.h>
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

// LibKexif includes.

#include <libkexif/kexifutils.h>
#include <libkexif/kexifdata.h>

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "thumbnailjob.h"
#include "albumfilecopymove.h"
#include "digikamio.h"
#include "syncjob.h"
#include "albumlister.h"
#include "albumfiletip.h"
#include "tagspopupmenu.h"

#include "albumsettings.h"
#include "imagedescedit.h"
#include "imageproperties.h"
#include "imagewindow.h"
#include "thumbnailsize.h"
#include "themeengine.h"

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
        inFocus       = false;
    }

    AlbumLister         *imageLister;
    Album               *currentAlbum;
    const AlbumSettings *albumSettings;
    QGuardedPtr<ThumbnailJob> thumbJob;

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
    QPixmap bannerPixmap;

    QFont fnReg;
    QFont fnCom;
    QFont fnXtra;

    QDict<AlbumIconItem> itemDict;
    AlbumFileTip*        toolTip;
    QTimer*              rearrangeTimer;

    bool                 inFocus;
    QString              nextItemToSelect;
};


AlbumIconView::AlbumIconView(QWidget* parent)
             : ThumbView(parent)
{
    d = new AlbumIconViewPrivate;
    d->init();
    d->imageLister = new AlbumLister();

    d->toolTip = new AlbumFileTip(this);
    d->rearrangeTimer = new QTimer(this);

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

    connect(this, SIGNAL(signalShowToolTip(ThumbItem*)),
            this, SLOT(slotShowToolTip(ThumbItem*)));

    // -- Self connections ----------------------------------------------

    connect(this, SIGNAL(contentsMoving(int, int)),
            SLOT(slotContentsMoving(int, int)));

    // -- ThemeEngine connections ---------------------------------------

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            SLOT(slotThemeChanged()));

    // -- Timer connection ----------------------------------------------

    connect(d->rearrangeTimer, SIGNAL(timeout()),
            SLOT(slotRearrange()));

    // -- resource for broken image thumbnail ---------------------------
    KGlobal::dirs()->addResourceType("digikam_imagebroken",
                                     KGlobal::dirs()->kde_default("data")
                                     + "digikam/data");
}

AlbumIconView::~AlbumIconView()
{
    delete d->rearrangeTimer;

    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
    }

    if (!d->thumbJob.isNull())
        delete d->thumbJob;

    delete d->imageLister;
    delete d->toolTip;
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

    setEnableToolTips(d->albumSettings->getShowToolTips());

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
    if(d->currentAlbum->isRoot())
        return;

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

    // create a url list with items sorted according to the view
    KURL::List urlList;
    for (ThumbItem *it = firstItem(); it; it=it->nextItem()) {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
        urlList.append(iconItem->fileItem()->url());
    }

    if (d->thumbJob.isNull())
    {
        d->thumbJob = new ThumbnailJob(urlList,
                                       (int)d->thumbSize.size(),
                                       showMetaInfo());
        connect(d->thumbJob,
                SIGNAL(signalThumbnailMetaInfo(const KURL&,
                                               const QPixmap&,
                                               const KFileMetaInfo*)),
                SLOT(slotGotThumbnail(const KURL&,
                                      const QPixmap&,
                                      const KFileMetaInfo*)));
        connect(d->thumbJob,
                SIGNAL(signalFailed(const KURL&)),
                SLOT(slotFailedThumbnail(const KURL&)));

        connect(d->thumbJob,
                SIGNAL(signalCompleted()),
                SLOT(slotFinishedThumbnail()));
    }
    else
    {
        d->thumbJob->addItems(urlList);
        slotContentsMoving(contentsX(), contentsY());
    }

    if (!d->nextItemToSelect.isEmpty())
    {
        ThumbItem* item = findItem(d->nextItemToSelect);
        if (item)
            item->setSelected(true);
        d->nextItemToSelect = QString();
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
        d->thumbJob->removeItem(item->url());

    if( d->currentAlbum && d->currentAlbum->type() == Album::PHYSICAL )
    {
        PAlbum *album = dynamic_cast<PAlbum*>(d->currentAlbum);
        if(album && album->getIconKURL().equals(iconItem->fileItem()->url()))
        {
            QString err;
            AlbumManager::instance()->updatePAlbumIcon( album,  "",
                                                        true, err );
        }
    }

    delete iconItem;
    item->removeExtraData(this);

    KURL u(item->url());
    u.cleanPath();
    d->itemDict.remove(u.url());

    d->rearrangeTimer->start(0, true);
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

    QPopupMenu openWithMenu;

    KTrader::OfferList::Iterator iter;
    KService::Ptr ptr;
    int index = 100;

    for( iter = offers.begin(); iter != offers.end(); ++iter )
    {
        ptr = *iter;
        openWithMenu.insertItem( ptr->pixmap(KIcon::Small),
                                 ptr->name(), index++);
        serviceVector.push_back(ptr);
    }

    // --------------------------------------------------------

    QPopupMenu popmenu(this);
    popmenu.insertItem(SmallIcon("editimage"),
                       i18n("View/Edit..."), 10);
    popmenu.insertItem(i18n("Open With"), 
                       &openWithMenu, 11);
    popmenu.insertSeparator();
    popmenu.insertItem(SmallIcon("imagecomment"),
                       i18n("Edit Comments && Tags..."), 12);
    popmenu.insertItem(SmallIcon("exifinfo"), 
                       i18n("Properties"), 14);

    if( d->currentAlbum && d->currentAlbum->type() == Album::PHYSICAL )
        popmenu.insertItem(i18n("Set as Album Thumbnail"), 17);
    else
        popmenu.insertItem(i18n("Set as Tag Thumbnail"), 17);

    popmenu.insertSeparator();

    // Bulk assignment/removal of tags --------------------------

    TagsPopupMenu* assignTagsPopup = new TagsPopupMenu(this, 1000);
    TagsPopupMenu* removeTagsPopup = new TagsPopupMenu(this, 2000, true);
    connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
            SLOT(slotAssignTag(int)));
    connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
            SLOT(slotRemoveTag(int)));

    popmenu.insertItem(i18n("Assign Tag"), assignTagsPopup);
    popmenu.insertItem(i18n("Remove Tag"), removeTagsPopup);
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

    KActionMenu* batchMenu = new KActionMenu(i18n("Batch Processes"));

    const QPtrList<KAction>& BatchActions =
        DigikamApp::getinstance()->menuBatchActions();

    QPtrListIterator<KAction> it2(BatchActions);
    count = 0;

    while ( (action = it2.current()) != 0 )
    {
        batchMenu->insert(action);
        ++it2;
        count = 1;
    }

    // Don't insert a separator if we didn't plug in any actions

    if (count != 0)
    {
        batchMenu->plug(&popmenu);
        popmenu.insertSeparator();
    }

    // --------------------------------------------------------

    popmenu.insertItem(SmallIcon("pencil"),
                       i18n("Rename..."), 15);

    if (d->albumSettings->getUseTrash())
        popmenu.insertItem(SmallIcon("edittrash"),
                           i18n("Move to Trash"), 16);
    else
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

    case 17: {
        slotSetAlbumThumbnail(iconItem);
        break;
    }

    default:
        break;
    }

    //---------------------------------------------------------------

    if (id >= 100 && id < 1000) {
        KService::Ptr imageServicePtr = serviceVector[id-100];
        KURL::List urlList;
        for (ThumbItem *it = firstItem(); it; it=it->nextItem())
        {
            if (it->isSelected())
            {
                AlbumIconItem *selItem = static_cast<AlbumIconItem *>(it);
                urlList.append(selItem->fileItem()->url());
            }
        }
        if (urlList.count())
            KRun::run(*imageServicePtr, urlList);
    }

    serviceVector.clear();
    delete assignTagsPopup;
    delete removeTagsPopup;
    delete batchMenu;
}

void AlbumIconView::slotSetAlbumThumbnail(AlbumIconItem *iconItem)
{
    if(!d->currentAlbum)
        return;

    if(d->currentAlbum->type() == Album::PHYSICAL)
    {
        PAlbum *album = static_cast<PAlbum*>(d->currentAlbum);

        QString err;
        AlbumManager::instance()->updatePAlbumIcon( album,
                iconItem->fileItem()->url().filename(false), true, err );
    }
    else
    {
        TAlbum *album = static_cast<TAlbum*>(d->currentAlbum);

        QString err;
        AlbumManager::instance()->updateTAlbumIcon( album,
                iconItem->fileItem()->url().path(false), true, err );
    }
}

void AlbumIconView::slotEditImageComments(AlbumIconItem* iconItem)
{
    ImageDescEdit descEdit(this, iconItem, this);
    descEdit.exec();

    if (d->currentAlbum && d->currentAlbum->type() == Album::TAG)
    {
        d->imageLister->updateDirectory();
    }

    updateBanner();
    updateContents();
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

    bool renameAlbumIcon = false;
    if(album->getIcon() == item->fileItem()->url().prettyURL())
    {
        renameAlbumIcon = true;
    }

    bool ok;

#if KDE_IS_VERSION(3,2,0)
    QString newName = KInputDialog::getText(i18n("Rename Item"),
                                            i18n("Enter new name:"),
                                            item->fileItem()->url().fileName(),
                                            &ok, this);
#else
    QString newName = KLineEditDlg::getText(i18n("Rename Item"),
                                            i18n("Enter new name:"),
                                            item->fileItem()->url().fileName(),
                                            &ok, this);
#endif

    if (!ok)
        return;

    if (!AlbumFileCopyMove::rename(album, item->fileItem()->url().fileName(),
                                   newName))
        return;

    QFileInfo fi(newName);
    QString newExt(QString("*.") + fi.extension());
    AlbumSettings* settings = AlbumSettings::instance();

    if ( !(QStringList::split(" ", settings->getImageFileFilter()).contains(newExt) ||
           QStringList::split(" ", settings->getMovieFileFilter()).contains(newExt) ||
           QStringList::split(" ", settings->getAudioFileFilter()).contains(newExt) ||
           QStringList::split(" ", settings->getRawFileFilter()).contains(newExt)) )
    {
        settings->setImageFileFilter(settings->getImageFileFilter() +
                                     QString(" ") + newExt);
        d->imageLister->setNameFilter(d->albumSettings->getImageFileFilter() + " " +
                                      d->albumSettings->getMovieFileFilter() + " " +
                                      d->albumSettings->getAudioFileFilter() + " " +
                                      d->albumSettings->getRawFileFilter());
    }
    
    KURL newURL = item->fileItem()->url().upURL();
    newURL.addPath(newName);
    d->nextItemToSelect = newURL.url();

    if (d->currentAlbum)
        d->imageLister->updateDirectory();

    if( renameAlbumIcon )
    {
        QString err;
        AlbumManager::instance()->updatePAlbumIcon( album,
            item->fileItem()->url().filename(false), false, err );
    }
}

void AlbumIconView::slotDeleteSelectedItems()
{
    KURL::List  urlList;
    QStringList nameList;

    KURL url;
    for (ThumbItem *it = firstItem(); it; it=it->nextItem()) {
        if (it->isSelected()) {
            AlbumIconItem *iconItem =
                static_cast<AlbumIconItem *>(it);
            url = iconItem->fileItem()->url();
            urlList.append(url);
            nameList.append(iconItem->text());
        }
    }

    if (urlList.count() <= 0)
        return;

    if (!d->albumSettings->getUseTrash())
    {
        QString warnMsg = i18n("About to delete this image. Are you sure?",
                               "About to delete these %n images. Are you sure?",
                               nameList.count());

        if (KMessageBox::warningContinueCancelList(this,
                                                   warnMsg,
                                                   nameList,
                                                   i18n("Warning"),
                                                   i18n("Delete"))
            !=  KMessageBox::Continue)
        {
            return;
        }
    }

    AlbumManager* man = AlbumManager::instance();
    AlbumDB* db = man->albumDB();

    if (SyncJob::userDelete(urlList))
    {
        for (KURL::List::const_iterator it = urlList.begin();
             it != urlList.end(); ++it)
        {
            AlbumIconItem* iconItem = findItem((*it).url());
            if (!iconItem)
                continue;

            PAlbum* palbum =
                d->imageLister->findParentAlbum(iconItem->fileItem());
            if (palbum)
            {
                db->deleteItem(palbum, iconItem->text());
            }
        }
    }
    else
    {
        KMessageBox::sorry(0, i18n("Failed to delete files.\n%1")
                           .arg(SyncJob::lastErrorMsg()));
    }

    d->imageLister->updateDirectory();
    updateBanner();
}

void AlbumIconView::slotFilesModified()
{
    d->imageLister->updateDirectory();
}

void AlbumIconView::slotFilesModified(const KURL& url)
{
    refreshItems(url);
}

void AlbumIconView::slotDisplayItem(AlbumIconItem *item )
{
    if (!item) return;
    
    AlbumSettings *settings = AlbumSettings::instance();

    if (!settings) return;

    KIconEffect::visualActivate(viewport(), contentsRectToViewport(item->rect()));
        
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

    // Run Digikam ImageEditor with all image files in the current Album.

    KURL::List urlList;

    for (ThumbItem *it = firstItem() ; it ; it = it->nextItem())
    {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
        QString fileExtension = iconItem->fileItem()->url().fileName().section( '.', -1 );

        if ( imagefilter.find(fileExtension) != -1 )
            urlList.append(iconItem->fileItem()->url());
    }

    ImageWindow *imview = ImageWindow::imagewindow();

    imview->disconnect(this);
    
    connect(imview, SIGNAL(signalFileAdded(const KURL&)),
            SLOT(slotFilesModified()));
    connect(imview, SIGNAL(signalFileModified(const KURL&)),
            SLOT(slotFilesModified(const KURL&)));
    connect(imview, SIGNAL(signalFileDeleted(const KURL&)),
            SLOT(slotFilesModified()));

    imview->loadURL(urlList, 
                    item->fileItem()->url(),
                    d->currentAlbum ? d->currentAlbum->getTitle():QString(),
                    true,
                    this);  // Allow to use image properties and comments/tags dialogs
    
    if (imview->isHidden())
        imview->show();
        
    imview->raise();
    imview->setFocus();
}

void AlbumIconView::slotProperties(AlbumIconItem* item)
{
    if (!item) return;

    ImageProperties properties(this, item);
    
    properties.exec();

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

QStringList AlbumIconView::itemTagPaths(AlbumIconItem* item)
{
    PAlbum* album = d->imageLister->findParentAlbum(item->fileItem());
    if (!album)
    {
        kdWarning() << "Failed to find parent album for "
                    << item->fileItem()->url().prettyURL()
                    << endl;
        return QString("");
    }

    QStringList tagPaths;

    AlbumManager* man = AlbumManager::instance();
    AlbumDB*      db  = man->albumDB();

    IntList tagIDs(db->getItemTagIDs(album, item->text()));
    for (IntList::iterator it = tagIDs.begin(); it != tagIDs.end(); ++it)
    {
        TAlbum* ta = man->findTAlbum(*it);
        if (ta)
            tagPaths.append(ta->getURL());
    }

    return tagPaths;
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

void AlbumIconView::resizeEvent(QResizeEvent *e)
{
    ThumbView::resizeEvent(e);

    if (d->bannerPixmap.width() != frameRect().width())
        calcBanner();
}

void AlbumIconView::setInFocus(bool val)
{
    d->inFocus = val;
}

void AlbumIconView::focusInEvent(QFocusEvent* e)
{
    emit signalInFocus();
    ThumbView::focusInEvent(e);
}

void AlbumIconView::drawFrame(QPainter* p)
{
    if (d->inFocus)
        drawFrameRaised(p);
    else
        drawFrameSunken(p);
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

    d->bannerPixmap = ThemeEngine::instance()->bannerPixmap(banner.width(),
                                                            banner.height());
}

void AlbumIconView::paintBanner(QPainter *p)
{
    QRect r(contentsRectToViewport(bannerRect()));

    if (!p || r.isEmpty() || r.isNull())
        return;

    p->save();

    p->drawPixmap(r.x(), r.y(), d->bannerPixmap);

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
        d->albumComments = (static_cast<TAlbum*>(d->currentAlbum))->getPrettyURL();
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
        drag->drag();
    }
}

void AlbumIconView::contentsDragMoveEvent(QDragMoveEvent *event)
{
    if (!d->currentAlbum || (AlbumDrag::canDecode(event) ||
                             !QUriDrag::canDecode(event) &&
                             !CameraDragObject::canDecode(event) &&
                             !TagDrag::canDecode(event))
        || event->source() == this) {
        event->ignore();
        return;
    }
    event->accept();
}

void AlbumIconView::contentsDropEvent(QDropEvent *event)
{

    if (!d->currentAlbum || (AlbumDrag::canDecode(event) ||
                             !QUriDrag::canDecode(event) &&
                             !CameraDragObject::canDecode(event) &&
                             !TagDrag::canDecode(event))
         || event->source() == this)
    {
        event->ignore();
        return;
    }

    if (QUriDrag::canDecode(event) &&
        d->currentAlbum->type() == Album::PHYSICAL)
    {
        PAlbum* palbum = (PAlbum*)d->currentAlbum;
        KURL destURL(palbum->getKURL());

        KURL::List srcURLs;
        KURLDrag::decode(event, srcURLs);

        QPopupMenu popMenu(this);
        popMenu.insertItem( SmallIcon("goto"), i18n("&Move Here"), 10 );
        popMenu.insertItem( SmallIcon("editcopy"), i18n("&Copy Here"), 11 );
        popMenu.insertSeparator(-1);
        popMenu.insertItem( SmallIcon("cancel"), i18n("C&ancel") );

        popMenu.setMouseTracking(true);
        int id = popMenu.exec(QCursor::pos());
        switch(id) {
        case 10: {
            new DigikamIO(srcURLs, destURL, true);
            break;
        }
        case 11: {
            new DigikamIO(srcURLs, destURL, false);
            break;
        }
        default:
            break;
        }
    }
    else if (CameraDragObject::canDecode(event) &&
             d->currentAlbum->type() == Album::PHYSICAL)
    {

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
    else if(TagDrag::canDecode(event))
    {
        QByteArray ba = event->encodedData("digikam/tag-id");
        QDataStream ds(ba, IO_ReadOnly);
        int tagID;
        ds >> tagID;

        AlbumManager* man = AlbumManager::instance();
        AlbumDB* db = man->albumDB();

        TAlbum* talbum = man->findTAlbum(tagID);

        if(talbum)
        {
            QPopupMenu popMenu(this);
            popMenu.insertItem(i18n("&Assign Tag '%1' to Selected Images")
                .arg(talbum->getPrettyURL()), 10 );
            popMenu.insertSeparator(-1);
            popMenu.insertItem( SmallIcon("cancel"), i18n("C&ancel") );

            popMenu.setMouseTracking(true);
            int id = popMenu.exec(QCursor::pos());
            switch(id) {
            case 10:
            {
                AlbumIconItem *albumItem = findItem(event->pos());
                if(albumItem)
                {
                    PAlbum* palbum =
                        d->imageLister->findParentAlbum(albumItem->fileItem());
                    if (palbum)
                    {
                        db->setItemTag(palbum, albumItem->text(), talbum);
                    }
                }

                for (ThumbItem *it = firstItem(); it; it = it->nextItem())
                {
                    if (it->isSelected())
                    {
                        AlbumIconItem *albumItem = static_cast<AlbumIconItem *>(it);
                        PAlbum* palbum =
                            d->imageLister->findParentAlbum(albumItem->fileItem());
                        if (palbum)
                        {
                            db->setItemTag(palbum, albumItem->text(), talbum);
                        }
                    }
                }
                if (d->currentAlbum && d->currentAlbum->type() == Album::TAG)
                {
                    d->imageLister->updateDirectory();
                }
                updateContents();
                break;
            }
            default:
                break;
            }
        }
    }
    else {
        event->ignore();
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
        if (d->thumbJob->setNextItemToLoad(item->fileItem()->url()))
            return;
        if (item == lastItem)
            return;
        item = (AlbumIconItem*)item->nextItem();
    }
}

bool AlbumIconView::acceptToolTip(ThumbItem *item, const QPoint &mousePos)
{
    AlbumIconItem *iconItem = dynamic_cast<AlbumIconItem*>(item);
    
    if(iconItem && iconItem->thumbnailRect().contains(mousePos))
    {
        return true;
    }
    else
    {    
        return false;
    }
}

void AlbumIconView::slotShowToolTip(ThumbItem* item)
{
    d->toolTip->setIconItem(dynamic_cast<AlbumIconItem*>(item));
}

KURL::List AlbumIconView::allItems()
{
    KURL::List itemList;

    for (ThumbItem *it = firstItem(); it;
         it = it->nextItem()) {
        AlbumIconItem *item = (AlbumIconItem*) it;
        itemList.append(item->fileItem()->url());
    }

    return itemList;
}

KURL::List AlbumIconView::selectedItems()
{
    KURL::List itemList;

    for (ThumbItem *it = firstItem(); it;
         it = it->nextItem()) {
        if (it->isSelected()) {
            AlbumIconItem *item = (AlbumIconItem*) it;
            itemList.append(item->fileItem()->url());
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

void AlbumIconView::refreshItems(const KURL::List& urlList)
{
    if (!d->currentAlbum || urlList.empty())
        return;

    // we do two things here:
    // 1. refresh the timestamp
    // 2. refresh the thumbnails
    
    for (KURL::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        AlbumIconItem* iconItem = findItem((*it).url());
        if (!iconItem)
            continue;

        struct stat st;
        if (::stat(QFile::encodeName((*it).path()), &st) == 0)
        {
            iconItem->time_ = st.st_mtime;
        }
    }
    
    if (d->thumbJob.isNull())
    {
        d->thumbJob =
            new ThumbnailJob(urlList, (int)d->thumbSize.size());
        connect(d->thumbJob,
                SIGNAL(signalThumbnailMetaInfo(const KURL&,
                                               const QPixmap&,
                                               const KFileMetaInfo*)),
                SLOT(slotGotThumbnail(const KURL&,
                                      const QPixmap&,
                                      const KFileMetaInfo*)));
        connect(d->thumbJob,
                SIGNAL(signalFailed(const KURL&)),
                SLOT(slotFailedThumbnail(const KURL&)));
        connect(d->thumbJob,
                SIGNAL(signalCompleted()),
                SLOT(slotFinishedThumbnail()));
    }
    else
    {
        d->thumbJob->addItems(urlList);
    }

    // trigger a delayed update, in case we need to resort items
    triggerUpdate();
}

void AlbumIconView::slotGotThumbnail(const KURL& url, const QPixmap& pix,
                                     const KFileMetaInfo* metaInfo)
{
    if(!d->currentAlbum)
        return;

    AlbumSettings *settings = AlbumSettings::instance();

    if (!settings)
        return;

    AlbumIconItem *iconItem = d->itemDict.find(url.url());
    if (!iconItem)
        return;

    iconItem->setPixmap(pix, metaInfo);

    if( d->currentAlbum->type() == Album::PHYSICAL &&
        d->currentAlbum->getIcon().isEmpty())
    {
        QString err;
        PAlbum *album = static_cast<PAlbum*>(d->currentAlbum);
        AlbumManager::instance()->updatePAlbumIcon( album,
            iconItem->fileItem()->url().filename(false), true, err );
    }
}

// If we failed to generate a thumbnail using our thumbnail generator
// use kde thumbnail generator to generate one

void AlbumIconView::slotFailedThumbnail(const KURL& url)
{
    KURL::List urlList;
    urlList.append(url);

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

    slotGotThumbnail(iconItem->fileItem()->url(), pix, 0);
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

    slotGotThumbnail(iconItem->fileItem()->url(), QPixmap(img), 0);
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

void AlbumIconView::slotSetExifOrientation( int orientation )
{
    KURL::List urlList;

    for (ThumbItem *it = firstItem(); it; it=it->nextItem()) {
        if (it->isSelected()) {
            AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
            urlList.append(iconItem->fileItem()->url());
        }
    }

    if (urlList.count() <= 0) return;

    KURL::List::Iterator it;

    for( it = urlList.begin(); it != urlList.end(); ++it )
    {
        kdDebug() << "Setting Exif Orientation to " << orientation << endl;

        KExifData::ImageOrientation o = (KExifData::ImageOrientation)orientation;

        if (!KExifUtils::writeOrientation((*it).path(), o))
        {
            KMessageBox::sorry(0, i18n("Failed to correct Exif orientation for file %1.")
                    .arg((*it).filename()));
            return;
        }

    }

    refreshItems(urlList);
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

    int margin  = 5;
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

    d->itemRegPixmap = ThemeEngine::instance()->thumbRegPixmap(d->itemRect.width(),
                                                               d->itemRect.height());

    d->itemSelPixmap = ThemeEngine::instance()->thumbSelPixmap(d->itemRect.width(),
                                                               d->itemRect.height());
}

void AlbumIconView::slotThemeChanged()
{
    QPalette plt(palette());
    QColorGroup cg(plt.active());
    cg.setColor(QColorGroup::Base, ThemeEngine::instance()->baseColor());
    cg.setColor(QColorGroup::Text, ThemeEngine::instance()->textRegColor());
    cg.setColor(QColorGroup::HighlightedText, ThemeEngine::instance()->textSelColor());
    plt.setActive(cg);
    plt.setInactive(cg);
    setPalette(plt);

    updateItemRectsPixmap();
    updateBanner();

    viewport()->update();
}

bool AlbumIconView::showMetaInfo()
{
    return (d->albumSettings->getIconShowResolution() ||
            d->albumSettings->getIconShowFileComments());
}

AlbumIconItem* AlbumIconView::findItem(const QPoint& pos)
{
    return dynamic_cast<AlbumIconItem*>(ThumbView::findItem(pos));
}

AlbumIconItem* AlbumIconView::findItem(const QString& url) const
{
    return d->itemDict.find(url);
}

void AlbumIconView::slotAlbumModified()
{
    updateBanner();
}

void AlbumIconView::slotAssignTag(int tagID)
{
    AlbumManager* man = AlbumManager::instance();
    AlbumDB* db = man->albumDB();

    TAlbum* talbum = man->findTAlbum(tagID);

    if (talbum)
    {
        for (ThumbItem *it = firstItem(); it; it = it->nextItem())
        {
            if (it->isSelected())
            {
                AlbumIconItem *albumItem = static_cast<AlbumIconItem *>(it);
                PAlbum* palbum =
                    d->imageLister->findParentAlbum(albumItem->fileItem());
                if (palbum)
                {
                    db->setItemTag(palbum, albumItem->text(), talbum);
                }
            }
        }
    }

    if (d->currentAlbum && d->currentAlbum->type() == Album::TAG)
    {
        d->imageLister->updateDirectory();
    }
    updateContents();

}

void AlbumIconView::slotRemoveTag(int tagID)
{
    AlbumManager* man = AlbumManager::instance();
    AlbumDB* db = man->albumDB();

    TAlbum* talbum = man->findTAlbum(tagID);

    if (talbum)
    {
        for (ThumbItem *it = firstItem(); it; it = it->nextItem())
        {
            if (it->isSelected())
            {
                AlbumIconItem *albumItem = static_cast<AlbumIconItem *>(it);
                PAlbum* palbum =
                    d->imageLister->findParentAlbum(albumItem->fileItem());
                if (palbum)
                {
                    db->removeItemTag(palbum, albumItem->text(), talbum);
                }
            }
        }
    }

    if (d->currentAlbum && d->currentAlbum->type() == Album::TAG)
    {
        d->imageLister->updateDirectory();
    }
    updateContents();
}

void AlbumIconView::slotRearrange()
{
    if (d->rearrangeTimer->isActive())
        return;

    updateBanner();
    slotUpdate();
}

#include "albumiconview.moc"
