/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2002-16-10
 * Description : implementation of album view interface. 
 *
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier
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

// Qt Includes.

#include <qstring.h>
#include <qstringlist.h>
#include <qstrlist.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qimage.h>
#include <qevent.h>
#include <qapplication.h>
#include <qsplitter.h>
#include <qtimer.h>

// KDE includes.

#include <kurl.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <krun.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kimageio.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "rawfiles.h"
#include "albummanager.h"
#include "album.h"
#include "albumwidgetstack.h"
#include "albumfolderview.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumsettings.h"
#include "albumhistory.h"
#include "batchsyncmetadata.h"
#include "slideshow.h"
#include "sidebar.h"
#include "imagepropertiessidebardb.h"
#include "imageinfoalbumsjob.h"
#include "datefolderview.h"
#include "tagfolderview.h"
#include "searchfolderview.h"
#include "statusprogressbar.h"
#include "tagfilterview.h"
#include "thumbnailsize.h"
#include "dio.h"
#include "digikamapp.h"
#include "digikamview.h"
#include "digikamview.moc"

namespace Digikam
{

class DigikamViewPriv
{
public:

    DigikamViewPriv()
    {
        splitter             = 0;
        parent               = 0;
        iconView             = 0;
        folderView           = 0;
        albumManager         = 0;
        albumHistory         = 0;
        leftSideBar          = 0;
        rightSideBar         = 0;
        dateFolderView       = 0;
        tagFolderView        = 0;
        searchFolderView     = 0;
        tagFilterView        = 0;
        albumWidgetStack     = 0;
        selectionTimer       = 0;
        needDispatchSelection= false;
    }

    int                       initialAlbumID;

    QSplitter                *splitter;

    QTimer                   *selectionTimer;

    DigikamApp               *parent;

    AlbumIconView            *iconView;
    AlbumFolderView          *folderView;
    AlbumManager             *albumManager;
    AlbumHistory             *albumHistory;
    AlbumWidgetStack         *albumWidgetStack;
    
    Sidebar                  *leftSideBar;
    ImagePropertiesSideBarDB *rightSideBar;

    DateFolderView           *dateFolderView;
    TagFolderView            *tagFolderView;
    SearchFolderView         *searchFolderView;
    TagFilterView            *tagFilterView;

    bool                      needDispatchSelection;
};

DigikamView::DigikamView(QWidget *parent)
           : QHBox(parent)
{
    d = new DigikamViewPriv;
    d->parent       = static_cast<DigikamApp *>(parent);
    d->albumManager = AlbumManager::instance();
    d->leftSideBar  = new Sidebar(this, "Digikam Left Sidebar", Sidebar::Left);

    d->splitter = new QSplitter(this);
    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );
    d->splitter->setOpaqueResize(false);

    d->leftSideBar->setSplitter(d->splitter);
    d->albumWidgetStack = new AlbumWidgetStack(d->splitter);
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    d->albumWidgetStack->setSizePolicy(rightSzPolicy);
    d->iconView = d->albumWidgetStack->albumIconView();

    d->rightSideBar = new ImagePropertiesSideBarDB(this, "Digikam Right Sidebar", d->splitter, 
                                                   Sidebar::Right, true);

    // To the left.
    d->folderView       = new AlbumFolderView(this);
    d->dateFolderView   = new DateFolderView(this);
    d->tagFolderView    = new TagFolderView(this);
    d->searchFolderView = new SearchFolderView(this);
    d->leftSideBar->appendTab(d->folderView, SmallIcon("folder_image"), i18n("Albums"));
    d->leftSideBar->appendTab(d->dateFolderView, SmallIcon("date"), i18n("Dates"));
    d->leftSideBar->appendTab(d->tagFolderView, SmallIcon("tag"), i18n("Tags"));
    d->leftSideBar->appendTab(d->searchFolderView, SmallIcon("find"), i18n("Searches"));

    // To the right.
    d->tagFilterView = new TagFilterView(this);
    d->rightSideBar->appendTab(d->tagFilterView, SmallIcon("tag-assigned"), i18n("Tag Filters"));

    d->selectionTimer = new QTimer(this);

    setupConnections();

    d->albumManager->setItemHandler(d->iconView);
    d->albumHistory = new AlbumHistory();
}

DigikamView::~DigikamView()
{
    saveViewState();

    delete d->albumHistory;
    d->albumManager->setItemHandler(0);
    delete d;
}

void DigikamView::applySettings(const AlbumSettings* settings)
{
    d->iconView->applySettings(settings);
}

void DigikamView::setupConnections()
{
    // -- DigikamApp connections ----------------------------------

    connect(d->parent, SIGNAL(signalEscapePressed()),
            this, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalEscapePressed()),
            d->albumWidgetStack, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->parent, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->parent, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->parent, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    connect(d->parent, SIGNAL(signalCopyAlbumItemsSelection()),
            d->iconView, SLOT(slotCopy()));

    connect(d->parent, SIGNAL(signalPasteAlbumItemsSelection()),
            d->iconView, SLOT(slotPaste()));

    connect(this, SIGNAL(signalProgressBarMode(int, const QString&)),
            d->parent, SLOT(slotProgressBarMode(int, const QString&)));

    connect(this, SIGNAL(signalProgressValue(int)),
            d->parent, SLOT(slotProgressValue(int)));
                        
    // -- AlbumManager connections --------------------------------

    connect(d->albumManager, SIGNAL(signalAlbumCurrentChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

    connect(d->albumManager, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(d->albumManager, SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotAllAlbumsLoaded()));

    connect(d->albumManager, SIGNAL(signalAlbumItemsSelected(bool) ),
            this, SLOT(slotImageSelected()));

    connect(d->albumManager, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    // -- IconView Connections -------------------------------------

    connect(d->iconView, SIGNAL(signalItemsUpdated(const KURL::List&)),
            d->albumWidgetStack, SLOT(slotItemsUpdated(const KURL::List&)));

    connect(d->iconView, SIGNAL(signalItemsAdded()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(signalItemsAdded()),
            this, SLOT(slotAlbumHighlight()));

    connect(d->iconView, SIGNAL(signalPreviewItem(AlbumIconItem*)),
            this, SLOT(slotTogglePreviewMode(AlbumIconItem*)));

    //connect(d->iconView, SIGNAL(signalItemDeleted(AlbumIconItem*)),
      //      this, SIGNAL(signalNoCurrentItem()));

    connect(d->folderView, SIGNAL(signalAlbumModified()),
            d->iconView, SLOT(slotAlbumModified()));

    connect(d->iconView, SIGNAL(signalProgressBarMode(int, const QString&)),
            d->parent, SLOT(slotProgressBarMode(int, const QString&)));

    connect(d->iconView, SIGNAL(signalProgressValue(int)),
            d->parent, SLOT(slotProgressValue(int)));

    // -- Sidebar Connections -------------------------------------

    connect(d->leftSideBar, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotLeftSidebarChangedTab(QWidget*)));

    connect(d->rightSideBar, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->rightSideBar, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->rightSideBar, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->rightSideBar, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSideBar, SLOT(slotNoCurrentItem()));

    connect(d->rightSideBar, SIGNAL(signalProgressBarMode(int, const QString&)),
            d->parent, SLOT(slotProgressBarMode(int, const QString&)));

    connect(d->rightSideBar, SIGNAL(signalProgressValue(int)),
            d->parent, SLOT(slotProgressValue(int)));

    connect(d->tagFilterView, SIGNAL(signalProgressBarMode(int, const QString&)),
            d->parent, SLOT(slotProgressBarMode(int, const QString&)));

    connect(d->tagFilterView, SIGNAL(signalProgressValue(int)),
            d->parent, SLOT(slotProgressValue(int)));

    connect(d->tagFolderView, SIGNAL(signalProgressBarMode(int, const QString&)),
            d->parent, SLOT(slotProgressBarMode(int, const QString&)));

    connect(d->tagFolderView, SIGNAL(signalProgressValue(int)),
            d->parent, SLOT(slotProgressValue(int)));

    // -- Preview image widget Connections ------------------------

    connect(d->albumWidgetStack, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->albumWidgetStack, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));
    
    connect(d->albumWidgetStack, SIGNAL(signalEditItem()),
            this, SLOT(slotImageEdit()));

    connect(d->albumWidgetStack, SIGNAL(signalDeleteItem()),
            this, SLOT(slotImageDelete()));

    connect(d->albumWidgetStack, SIGNAL(signalToggledToPreviewMode(bool)),
            this, SLOT(slotToggledToPreviewMode(bool)));

    connect(d->albumWidgetStack, SIGNAL(signalBack2Album()),
            this, SLOT(slotEscapePreview()));

    // -- Selection timer ---------------

    connect(d->selectionTimer, SIGNAL(timeout()),
            this, SLOT(slotDispatchImageSelected()));
}

void DigikamView::loadViewState()
{
    KConfig *config = kapp->config();
    config->setGroup("MainWindow");

    if(config->hasKey("SplitterSizes"))
        d->splitter->setSizes(config->readIntListEntry("SplitterSizes"));

    d->initialAlbumID = config->readNumEntry("InitialAlbumID", 0);
}

void DigikamView::saveViewState()
{
    KConfig *config = kapp->config();
    config->setGroup("MainWindow");
    config->writeEntry("SplitterSizes", d->splitter->sizes());

    Album *album = AlbumManager::instance()->currentAlbum();
    if(album)
    {
        config->writeEntry("InitialAlbumID", album->globalID());
    }
    else
    {
        config->writeEntry("InitialAlbumID", 0);
    }
}

void DigikamView::showSideBars()
{
    d->leftSideBar->restore();
    d->rightSideBar->restore();
}

void DigikamView::hideSideBars()
{
    d->leftSideBar->backup();
    d->rightSideBar->backup();
}

void DigikamView::slotFirstItem(void)
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(d->iconView->firstItem());
    d->iconView->clearSelection();
    d->iconView->updateContents();
    if (currItem)
       d->iconView->setCurrentItem(currItem);
}

void DigikamView::slotPrevItem(void)
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(d->iconView->currentItem());
    if (currItem)
    {
        if (currItem->prevItem())
        {
            d->iconView->clearSelection();
            d->iconView->updateContents();
            d->iconView->setCurrentItem(currItem->prevItem());
        }
    }
}

void DigikamView::slotNextItem(void)
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(d->iconView->currentItem());
    if (currItem)
    {
        if (currItem->nextItem())
        {
            d->iconView->clearSelection();
            d->iconView->updateContents();
            d->iconView->setCurrentItem(currItem->nextItem());
        }
    }
}

void DigikamView::slotLastItem(void)
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(d->iconView->lastItem());
    d->iconView->clearSelection();
    d->iconView->updateContents();
    if (currItem)
       d->iconView->setCurrentItem(currItem);
}

void DigikamView::slotAllAlbumsLoaded()
{
    disconnect(d->albumManager, SIGNAL(signalAllAlbumsLoaded()),
               this, SLOT(slotAllAlbumsLoaded()));

    loadViewState();
    Album *album = d->albumManager->findAlbum(d->initialAlbumID);
    d->albumManager->setCurrentAlbum(album);

    d->leftSideBar->loadViewState();
    d->rightSideBar->loadViewState();
    d->rightSideBar->populateTags();

    slotAlbumSelected(album);
}

void DigikamView::slotSortAlbums(int order)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;
    settings->setAlbumSortOrder( (AlbumSettings::AlbumSortOrder) order);
    d->folderView->resort();
}

void DigikamView::slotNewAlbum()
{
    d->folderView->albumNew();
}

void DigikamView::slotDeleteAlbum()
{
    d->folderView->albumDelete();
}

void DigikamView::slotNewTag()
{
    d->tagFolderView->tagNew();
}

void DigikamView::slotDeleteTag()
{
    d->tagFolderView->tagDelete();
}

void DigikamView::slotEditTag()
{
    d->tagFolderView->tagEdit();
}

void DigikamView::slotNewQuickSearch()
{
    if (d->leftSideBar->getActiveTab() != d->searchFolderView)
        d->leftSideBar->setActiveTab(d->searchFolderView);
    d->searchFolderView->quickSearchNew();
}

void DigikamView::slotNewAdvancedSearch()
{
    if (d->leftSideBar->getActiveTab() != d->searchFolderView)
        d->leftSideBar->setActiveTab(d->searchFolderView);
    d->searchFolderView->extendedSearchNew();
}

// ----------------------------------------------------------------

void DigikamView::slotAlbumDeleted(Album *delalbum)
{
    d->albumHistory->deleteAlbum(delalbum);

    // display changed tags
    if (delalbum->type() == Album::TAG)
        d->iconView->updateContents();

    /*
    // For what is this needed?
    Album *album;
    QWidget *widget;
    d->albumHistory->getCurrentAlbum(&album, &widget);

    changeAlbumFromHistory(album, widget);
    */
}

void DigikamView::slotAlbumRenamed(Album *album)
{
    // display changed names

    if (album == d->albumManager->currentAlbum())
        d->iconView->updateContents();
}

void DigikamView::slotAlbumHistoryBack(int steps)
{
    Album *album;
    QWidget *widget;

    d->albumHistory->back(&album, &widget, steps);

    changeAlbumFromHistory(album, widget);
}

void DigikamView::slotAlbumHistoryForward(int steps)
{
    Album *album;
    QWidget *widget;

    d->albumHistory->forward(&album, &widget, steps);

    changeAlbumFromHistory(album, widget);
}

void DigikamView::changeAlbumFromHistory(Album *album, QWidget *widget)
{
    if (album && widget)
    {
        QListViewItem *item;
        item = (QListViewItem*)album->extraData(widget);
        if(!item)
            return;

        // AlbumFolderview, TagFolderView, SearchFolderView inherit from FolderView
        if (FolderView *v = dynamic_cast<FolderView*>(widget))
        {
            v->setSelected(item, true);
            v->ensureItemVisible(item);
        }
        else if (DateFolderView *v = dynamic_cast<DateFolderView*>(widget))
        {
            v->setSelected(item);
        }

        d->leftSideBar->setActiveTab(widget);

        d->parent->enableAlbumBackwardHistory(!d->albumHistory->isBackwardEmpty());
        d->parent->enableAlbumForwardHistory(!d->albumHistory->isForwardEmpty());
    }
}

void DigikamView::clearHistory()
{
    d->albumHistory->clearHistory();
    d->parent->enableAlbumBackwardHistory(false);
    d->parent->enableAlbumForwardHistory(false);
}

void DigikamView::getBackwardHistory(QStringList &titles)
{
    d->albumHistory->getBackwardHistory(titles);
}

void DigikamView::getForwardHistory(QStringList &titles)
{
    d->albumHistory->getForwardHistory(titles);
}

void DigikamView::slotSelectAlbum(const KURL &)
{
    /* TODO
    if (url.isEmpty())
        return;

    Album *album = d->albumManager->findPAlbum(url);
    if(album && album->getViewItem())
    {
        AlbumFolderItem_Deprecated *item;
        item = static_cast<AlbumFolderItem_Deprecated*>(album->getViewItem());
        mFolderView_Deprecated->setSelected(item);
        d->parent->enableAlbumBackwardHistory(!d->albumHistory->isBackwardEmpty());
        d->parent->enableAlbumForwardHistory(!d->albumHistory->isForwardEmpty());
    }
    */
}

// ----------------------------------------------------------------

void DigikamView::slotAlbumSelected(Album* album)
{
    //emit signalNoCurrentItem();

    if (!album)
    {
        d->iconView->setAlbum(0);
        emit signalAlbumSelected(false);
        emit signalTagSelected(false);
        return;
    }

    if (album->type() == Album::PHYSICAL)
    {
        emit signalAlbumSelected(true);
        emit signalTagSelected(false);
    }
    else if (album->type() == Album::TAG)
    {
        emit signalAlbumSelected(false);
        emit signalTagSelected(true);
    }

    d->albumHistory->addAlbum(album, d->leftSideBar->getActiveTab());
    d->parent->enableAlbumBackwardHistory(!d->albumHistory->isBackwardEmpty());
    d->parent->enableAlbumForwardHistory(!d->albumHistory->isForwardEmpty());    

    d->iconView->setAlbum(album);
    if (album->isRoot())
        d->albumWidgetStack->setPreviewMode(AlbumWidgetStack::WelcomePageMode);
    else 
        d->albumWidgetStack->setPreviewMode(AlbumWidgetStack::PreviewAlbumMode);
}

void DigikamView::slotAlbumOpenInKonqui()
{
    Album *album = d->albumManager->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    PAlbum* palbum = dynamic_cast<PAlbum*>(album);

    new KRun(palbum->folderPath()); // KRun will delete itself.
}

void DigikamView::slotAlbumRefresh()
{
    d->iconView->refreshItems(d->iconView->allItems());
}

void DigikamView::slotImageSelected()
{
    // delay to slotDispatchImageSelected
    d->needDispatchSelection = true;
    d->selectionTimer->start(75, true);
}


void DigikamView::slotDispatchImageSelected()
{
    if (d->needDispatchSelection)
    {
        // the list of copies of ImageInfos of currently selected items, currentItem first
        QPtrList<ImageInfo> list = d->iconView->selectedImageInfos(true);

        if (list.isEmpty())
        {
            d->albumWidgetStack->setPreviewItem();
            emit signalImageSelected(list, false, false);
            emit signalNoCurrentItem();
        }
        else
        {
            d->rightSideBar->itemChanged(list);

            AlbumIconItem *selectedItem = d->iconView->firstSelectedItem();
            ImageInfo *previousInfo=0, *nextInfo=0;
            if (selectedItem->prevItem())
                previousInfo = static_cast<AlbumIconItem*>(selectedItem->prevItem())->imageInfo();
            if (selectedItem->nextItem())
                nextInfo = static_cast<AlbumIconItem*>(selectedItem->nextItem())->imageInfo();

            // we fed a list of copies
            d->rightSideBar->takeImageInfoOwnership(true);

            if (!d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
                d->albumWidgetStack->setPreviewItem(list.first(), previousInfo, nextInfo);

            emit signalImageSelected(list, previousInfo, nextInfo);
        }

        d->needDispatchSelection = false;
    }
}

void DigikamView::slotAlbumsCleared()
{
    d->iconView->clear();
    emit signalAlbumSelected(false);
}

// ----------------------------------------------------------------

void DigikamView::slotThumbSizePlus()
{
    emit signalNoCurrentItem();

    ThumbnailSize thumbSize;

    switch(d->iconView->thumbnailSize().size())
    {
        case (ThumbnailSize::Small):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Medium);
            break;
        }
        case (ThumbnailSize::Medium):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Large);
            break;
        }
        case (ThumbnailSize::Large):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Huge);
            break;
        }
        case (ThumbnailSize::Huge):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Huge);
            break;
        }
        default:
            return;
    }

    if (thumbSize.size() == ThumbnailSize::Huge)
    {
        d->parent->enableThumbSizePlusAction(false);
    }
    d->parent->enableThumbSizeMinusAction(true);

    d->iconView->setThumbnailSize(thumbSize);

    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setDefaultIconSize( (int)thumbSize.size() );
}

void DigikamView::slotThumbSizeMinus()
{
    emit signalNoCurrentItem();

    ThumbnailSize thumbSize;

    switch(d->iconView->thumbnailSize().size())
    {
        case (ThumbnailSize::Small):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Small);
            break;
        }
        case (ThumbnailSize::Medium):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Small);
            break;
        }
        case (ThumbnailSize::Large):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Medium);
            break;
        }
        case (ThumbnailSize::Huge):
        {
            thumbSize = ThumbnailSize(ThumbnailSize::Large);
            break;
        }
        default:
            return;
    }

    if (thumbSize.size() == ThumbnailSize::Small)
    {
        d->parent->enableThumbSizeMinusAction(false);
    }
    d->parent->enableThumbSizePlusAction(true);

    d->iconView->setThumbnailSize(thumbSize);

    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setDefaultIconSize( (int)thumbSize.size() );
}

void DigikamView::slotAlbumPropsEdit()
{
    d->folderView->albumEdit();
}

void DigikamView::slotAlbumSyncPicturesMetadata()
{
    Album *album = d->albumManager->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    BatchSyncMetadata *syncMetadata = new BatchSyncMetadata(this, album);
    
    connect(syncMetadata, SIGNAL(signalProgressBarMode(int, const QString&)),
            d->parent, SLOT(slotProgressBarMode(int, const QString&)));

    connect(syncMetadata, SIGNAL(signalProgressValue(int)),
            d->parent, SLOT(slotProgressValue(int)));

    connect(syncMetadata, SIGNAL(signalComplete()),
            this, SLOT(slotAlbumSyncPicturesMetadataDone()));

    connect(d->parent, SIGNAL(signalCancelButtonPressed()),
            syncMetadata, SLOT(slotAbort()));

    syncMetadata->parseAlbum();
}

void DigikamView::slotAlbumSyncPicturesMetadataDone()
{
    applySettings(AlbumSettings::instance());
}

void DigikamView::slotAlbumAddImages()
{
    Album *album = d->albumManager->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    PAlbum* palbum = dynamic_cast<PAlbum*>(album);

    QString fileformats;
    
    QStringList patternList = QStringList::split('\n', KImageIO::pattern(KImageIO::Reading));
    
    // All Pictures from list must been always the first entry given by KDE API
    QString allPictures = patternList[0];
    
    // Add RAW file format to All Pictures" type mime and remplace current.
    allPictures.insert(allPictures.find("|"), QString(raw_file_extentions));
    patternList.remove(patternList[0]);
    patternList.prepend(allPictures);
    
    // Added RAW file formats supported by dcraw program like a type mime. 
    // Nota: we cannot use here "image/x-raw" type mime from KDE because it uncomplete 
    // or unvailable(dcraw_0)(see file #121242 in B.K.O).
    patternList.append(QString("\n%1|Camera RAW files").arg(QString(raw_file_extentions)));
    
    fileformats = patternList.join("\n");

    DDebug () << "fileformats=" << fileformats << endl;   

    KURL::List urls = KFileDialog::getOpenURLs(AlbumManager::instance()->getLibraryPath(), 
                                               fileformats, this, i18n("Select Image to Add"));

    if (!urls.isEmpty())
    {
        KIO::Job* job = DIO::copy(urls, palbum->kurl());

        connect(job, SIGNAL(result(KIO::Job *) ),
                this, SLOT(slotImageCopyResult(KIO::Job *)));
    }
}

void DigikamView::slotImageCopyResult(KIO::Job* job)
{
    if (job->error())
        job->showErrorDialog(this);
}

void DigikamView::slotAlbumImportFolder()
{
    d->folderView->albumImportFolder();
}

void DigikamView::slotAlbumHighlight()
{
    // TODO:
    // Don't know what this is supposed to do.
    // Perhaps some flashing or other eye kandy
    /*
    Album *album = d->albumManager->currentAlbum();
    if (!album || !album->type() == Album::PHYSICAL)
        return;

    d->folderView->setAlbumThumbnail(dynamic_cast<PAlbum*>(album));
    */
}

// ----------------------------------------------------------------

void DigikamView::slotEscapePreview()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode ||
        d->albumWidgetStack->previewMode() == AlbumWidgetStack::WelcomePageMode)
        return;

    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(d->iconView->currentItem());
    slotTogglePreviewMode(currItem);
}

void DigikamView::slotImagePreview()
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(d->iconView->currentItem());
    if (currItem)
        slotTogglePreviewMode(currItem);
}

// This method toogle between AlbumView and ImagePreview Modes, depending of context.
void DigikamView::slotTogglePreviewMode(AlbumIconItem *iconItem)
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode && iconItem)
    {
        // We will go to ImagePreview Mode.
        ImageInfo *previousInfo=0, *nextInfo=0;
        if (iconItem->prevItem())
            previousInfo = static_cast<AlbumIconItem*>(iconItem->prevItem())->imageInfo();
        if (iconItem->nextItem())
            nextInfo = static_cast<AlbumIconItem*>(iconItem->nextItem())->imageInfo();
        d->albumWidgetStack->setPreviewItem(iconItem->imageInfo(), previousInfo, nextInfo);
    }
    else
    {
        // We go back to AlbumView Mode.
        d->albumWidgetStack->setPreviewMode( AlbumWidgetStack::PreviewAlbumMode );
    }
}

void DigikamView::slotToggledToPreviewMode(bool t)
{
    d->parent->toggledToPreviewMode(t);
}

void DigikamView::slotImageEdit()
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(d->iconView->currentItem());
    if (currItem)
        imageEdit(currItem);
}

void DigikamView::imageEdit(AlbumIconItem *iconItem)
{
    AlbumIconItem *item;

    if (!iconItem)
    {
        item = d->iconView->firstSelectedItem();
        if (!item) return;
    }
    else
    {
        item = iconItem;
    }

    d->iconView->slotDisplayItem(item);
}

void DigikamView::slotImageExifOrientation(int orientation)
{
    d->iconView->slotSetExifOrientation(orientation);
}

void DigikamView::slotImageRename(AlbumIconItem *iconItem)
{
    AlbumIconItem *item;

    if (!iconItem)
    {
        item = d->iconView->firstSelectedItem();
        if (!item) return;
    }
    else
    {
        item = iconItem;
    }

    d->iconView->slotRename(item);
}

void DigikamView::slotImageDelete()
{
    d->iconView->slotDeleteSelectedItems(false);
}

void DigikamView::slotImageDeletePermanently()
{
    d->iconView->slotDeleteSelectedItems(true);
}

void DigikamView::slotImageDeletePermanentlyDirectly()
{
    d->iconView->slotDeleteSelectedItemsDirectly(false);
}

void DigikamView::slotImageTrashDirectly()
{
    d->iconView->slotDeleteSelectedItemsDirectly(true);
}

void DigikamView::slotSelectAll()
{
    d->iconView->selectAll();
}

void DigikamView::slotSelectNone()
{
    d->iconView->clearSelection();
}

void DigikamView::slotSelectInvert()
{
    d->iconView->invertSelection();
}

void DigikamView::slotSortImages(int order)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setImageSortOrder((AlbumSettings::ImageSortOrder) order);
    d->iconView->slotUpdate();
}

void DigikamView::slotLeftSidebarChangedTab(QWidget* w)
{
    // setActive means that selection changes are propagated, nothing more.
    // Additionally, when it is set to true, the selectionChanged signal will be emitted.
    // So this is the place which causes the behavior that when the left sidebar
    // tab is changed, the current album is changed as well.
    d->dateFolderView->setActive(w == d->dateFolderView);
    d->folderView->setActive(w == d->folderView);
    d->tagFolderView->setActive(w == d->tagFolderView);
    d->searchFolderView->setActive(w == d->searchFolderView);
}

void DigikamView::slotAssignRating(int rating)
{
    d->iconView->slotAssignRating(rating);
}

void DigikamView::slotAssignRatingNoStar()
{
    d->iconView->slotAssignRating(0);
}

void DigikamView::slotAssignRatingOneStar()
{
    d->iconView->slotAssignRating(1);
}

void DigikamView::slotAssignRatingTwoStar()
{
    d->iconView->slotAssignRating(2);
}

void DigikamView::slotAssignRatingThreeStar()
{
    d->iconView->slotAssignRating(3);
}

void DigikamView::slotAssignRatingFourStar()
{
    d->iconView->slotAssignRating(4);
}

void DigikamView::slotAssignRatingFiveStar()
{
    d->iconView->slotAssignRating(5);
}

void DigikamView::slotSlideShowAll()
{
    ImageInfoList infoList;
    AlbumIconItem* item = dynamic_cast<AlbumIconItem*>(d->iconView->firstItem());
    while (item) 
    {
        infoList.append(item->imageInfo());
        item = dynamic_cast<AlbumIconItem*>(item->nextItem());
    }

    slideShow(infoList);
}

void DigikamView::slotSlideShowSelection()
{
    ImageInfoList infoList;
    AlbumIconItem* item = dynamic_cast<AlbumIconItem*>(d->iconView->firstItem());
    while (item) 
    {
        if (item->isSelected())
            infoList.append(item->imageInfo());
        item = dynamic_cast<AlbumIconItem*>(item->nextItem());
    }

    slideShow(infoList);
}

void DigikamView::slotSlideShowRecursive()
{
    Album *album = AlbumManager::instance()->currentAlbum();
    if(album)
    {
        AlbumList albumList;
        albumList.append(album);
        AlbumIterator it(album);
        while (it.current())
        {
            albumList.append(*it);
            ++it;
        }

        ImageInfoAlbumsJob *job = new ImageInfoAlbumsJob;
        connect(job, SIGNAL(signalCompleted(const ImageInfoList&)),
                this, SLOT(slotItemsInfoFromAlbums(const ImageInfoList&)));
        job->allItemsFromAlbums(albumList);       
    }
}

void DigikamView::slotItemsInfoFromAlbums(const ImageInfoList& infoList)
{
    ImageInfoList list = infoList; 
    slideShow(list);
}

void DigikamView::slideShow(ImageInfoList &infoList)
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    bool startWithCurrent = config->readBoolEntry("SlideShowStartCurrent", false);

    int     i = 0;
    float cnt = (float)infoList.count();
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                               i18n("Prepare image data for slideshow. Please wait..."));

    DMetadata         meta;
    SlideShowSettings settings;
    settings.exifRotate = AlbumSettings::instance()->getExifRotate();

    for (ImageInfoList::iterator it = infoList.begin(); it != infoList.end(); ++it)
    {
        ImageInfo *info = *it;
        settings.fileList.append(info->kurl());
        SlidePictureInfo pictInfo;
        meta.load(info->kurl().path());
        pictInfo.comment            = info->caption();
        pictInfo.photoInfo          = meta.getPhotographInformations();
        // In case of dateTime extraction from metadata failed 
        pictInfo.photoInfo.dateTime = info->dateTime(); 
        settings.pictInfoMap.insert(info->kurl(), pictInfo);

        emit signalProgressValue((int)((i++/cnt)*100.0));
        kapp->processEvents();
    }

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString::null);   

    settings.delay                = config->readNumEntry("SlideShowDelay", 5) * 1000;
    settings.printName            = config->readBoolEntry("SlideShowPrintName", true);
    settings.printDate            = config->readBoolEntry("SlideShowPrintDate", false);
    settings.printApertureFocal   = config->readBoolEntry("SlideShowPrintApertureFocal", false);
    settings.printExpoSensitivity = config->readBoolEntry("SlideShowPrintExpoSensitivity", false);
    settings.printComment         = config->readBoolEntry("SlideShowPrintComment", false);
    settings.loop                 = config->readBoolEntry("SlideShowLoop", false);

    SlideShow *slide = new SlideShow(settings);
    if (startWithCurrent)
    {
        AlbumIconItem* current = dynamic_cast<AlbumIconItem*>(d->iconView->currentItem());
        if (current) 
            slide->setCurrent(current->imageInfo()->kurl());
    }

    slide->show();
}

}  // namespace Digikam
