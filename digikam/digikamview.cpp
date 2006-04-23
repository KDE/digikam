/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2002-16-10
 * Description : 
 * 
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
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

// KDE includes.

#include <kurl.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <krun.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

// Local includes.

#include "albummanager.h"
#include "album.h"
#include "albumfolderview.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumsettings.h"
#include "albumhistory.h"
#include "sidebar.h"
#include "imagepropertiessidebardb.h"
#include "datefolderview.h"
#include "tagfolderview.h"
#include "searchfolderview.h"
#include "tagfilterview.h"
#include "thumbnailsize.h"
#include "dio.h"
#include "digikamapp.h"
#include "digikamview.h"

namespace Digikam
{

DigikamView::DigikamView(QWidget *parent)
           : QHBox(parent)
{
    mParent      = static_cast<DigikamApp *>(parent);
    mAlbumMan    = AlbumManager::instance();
    mMainSidebar = new Sidebar(this, "Digikam Left Sidebar", Sidebar::Left);

    mSplitter    = new QSplitter(this);
    mSplitter->setFrameStyle( QFrame::NoFrame );
    mSplitter->setFrameShadow( QFrame::Plain );
    mSplitter->setFrameShape( QFrame::NoFrame );
    mSplitter->setOpaqueResize(false);

    mMainSidebar->setSplitter(mSplitter);
    
    mIconView    = new AlbumIconView(mSplitter);
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    mIconView->setSizePolicy(rightSzPolicy);

    mRightSidebar = new ImagePropertiesSideBarDB(this, "Digikam Right Sidebar", mSplitter, 
                                                 Sidebar::Right, true, true);
    
    // To the left.
    mFolderView       = new AlbumFolderView(this);
    mDateFolderView   = new DateFolderView(this);
    mTagFolderView    = new TagFolderView(this);
    mSearchFolderView = new SearchFolderView(this);
    
    // To the right.
    mTagFilterView    = new TagFilterView(this);    
    
    mMainSidebar->appendTab(mFolderView, SmallIcon("folder"), i18n("Albums"));    
    mMainSidebar->appendTab(mDateFolderView, SmallIcon("date"), i18n("Dates"));
    mMainSidebar->appendTab(mTagFolderView, SmallIcon("tag"), i18n("Tags"));    
    mMainSidebar->appendTab(mSearchFolderView, SmallIcon("find"), i18n("Searches"));    

    mRightSidebar->appendTab(mTagFilterView, SmallIcon("tag"), i18n("Tag Filters"));
    
    setupConnections();

    mAlbumMan->setItemHandler(mIconView);

    mAlbumHistory = new AlbumHistory();    
}

DigikamView::~DigikamView()
{
    saveViewState();
    
    delete mAlbumHistory;
    mAlbumMan->setItemHandler(0);
}

void DigikamView::applySettings(const AlbumSettings* settings)
{
    mIconView->applySettings(settings);
}

void DigikamView::setupConnections()
{
    // -- AlbumManager connections --------------------------------

    connect(mAlbumMan, SIGNAL(signalAlbumCurrentChanged(Album*)),
            this, SLOT(slot_albumSelected(Album*)));
            
    connect(mAlbumMan, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slot_albumsCleared()));
            
    connect(mAlbumMan, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));
            
    connect(mAlbumMan, SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotAllAlbumsLoaded()));    
    
    connect(mAlbumMan, SIGNAL(signalAlbumItemsSelected(bool) ),
            this, SLOT(slot_imageSelected()));

    connect(mAlbumMan, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    // -- IconView Connections -------------------------------------

    connect(mIconView, SIGNAL(signalItemsAdded()),
            this, SLOT(slot_imageSelected()));

    connect(mIconView, SIGNAL(signalItemsAdded()),
            this, SLOT(slot_albumHighlight()));

    connect(mIconView, SIGNAL(signalItemDeleted(AlbumIconItem*)),
            this, SIGNAL(signal_noCurrentItem()));

    connect(mFolderView, SIGNAL(signalAlbumModified()),
            mIconView, SLOT(slotAlbumModified()));

    // -- Sidebar Connections -------------------------------------

    connect(mMainSidebar, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotLeftSidebarChangedTab(QWidget*)));

    connect(mRightSidebar, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));
    
    connect(mRightSidebar, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));
                
    connect(mRightSidebar, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));                
    
    connect(mRightSidebar, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));                

    connect(this, SIGNAL(signal_noCurrentItem()),
            mRightSidebar, SLOT(slotNoCurrentItem()));
}

void DigikamView::loadViewState()
{
    KConfig *config = kapp->config();
    config->setGroup("MainWindow");

    if(config->hasKey("SplitterSizes"))
        mSplitter->setSizes(config->readIntListEntry("SplitterSizes"));
    
    mInitialAlbumID = config->readNumEntry("InitialAlbumID", 0);
}

void DigikamView::saveViewState()
{
    KConfig *config = kapp->config();
    config->setGroup("MainWindow");
    config->writeEntry("SplitterSizes", mSplitter->sizes());

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

void DigikamView::slotFirstItem(void)
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(mIconView->firstItem());
    mIconView->clearSelection();
    mIconView->updateContents();
    if (currItem) 
       mIconView->setCurrentItem(currItem);
}

void DigikamView::slotPrevItem(void)
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(mIconView->currentItem());
    mIconView->clearSelection();
    mIconView->updateContents();
    if (currItem)
       mIconView->setCurrentItem(currItem->prevItem());
}

void DigikamView::slotNextItem(void)
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(mIconView->currentItem());
    mIconView->clearSelection();
    mIconView->updateContents();
    if (currItem) 
       mIconView->setCurrentItem(currItem->nextItem());
}

void DigikamView::slotLastItem(void)
{
    AlbumIconItem *currItem = dynamic_cast<AlbumIconItem*>(mIconView->lastItem());
    mIconView->clearSelection();
    mIconView->updateContents();
    if (currItem) 
       mIconView->setCurrentItem(currItem);
}

void DigikamView::slotAllAlbumsLoaded()
{
    disconnect(mAlbumMan, SIGNAL(signalAllAlbumsLoaded()),
               this, SLOT(slotAllAlbumsLoaded()));
    
    loadViewState();
    Album *album = mAlbumMan->findAlbum(mInitialAlbumID);
    mAlbumMan->setCurrentAlbum(album);
    
    mMainSidebar->loadViewState();
    mRightSidebar->loadViewState();
    mRightSidebar->populateTags();

    slot_albumSelected(album);
}

void DigikamView::slot_sortAlbums(int order)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;
    settings->setAlbumSortOrder(
        (AlbumSettings::AlbumSortOrder) order);
    mFolderView->resort();
}

void DigikamView::slot_newAlbum()
{
    mFolderView->albumNew();
}

void DigikamView::slot_deleteAlbum()
{
    mFolderView->albumDelete();
}

void DigikamView::slotNewTag()
{
    mTagFolderView->tagNew();
}

void DigikamView::slotDeleteTag()
{
    mTagFolderView->tagDelete();
}

void DigikamView::slotEditTag()
{
    mTagFolderView->tagEdit();
}

void DigikamView::slotNewQuickSearch()
{
    if (mMainSidebar->getActiveTab() != mSearchFolderView)
        mMainSidebar->setActiveTab(mSearchFolderView);
    mSearchFolderView->quickSearchNew();
}

void DigikamView::slotNewAdvancedSearch()
{
    if (mMainSidebar->getActiveTab() != mSearchFolderView)
        mMainSidebar->setActiveTab(mSearchFolderView);
    mSearchFolderView->extendedSearchNew();
}

// ----------------------------------------------------------------

void DigikamView::slotAlbumDeleted(Album *delalbum)
{
    mAlbumHistory->deleteAlbum(delalbum);
    
    Album *album;
    QWidget *widget;
    mAlbumHistory->getCurrentAlbum(&album, &widget);
    
    if (album && widget)
    {
        QListViewItem *item;
        item = (QListViewItem*)album->extraData(widget);
        if(!item)
            return;

        if (FolderView *v = dynamic_cast<FolderView*>(widget))
        {
            v->setSelected(item, true);
            v->ensureItemVisible(item);
        } 
        else if (DateFolderView *v = dynamic_cast<DateFolderView*>(widget))
        {
            v->setSelected(item);
        }
    
        mMainSidebar->setActiveTab(widget);
        
        mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
        mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());            
    }
}

void DigikamView::slotAlbumRenamed(Album *album)
{
    // display changed names

    if (album == mAlbumMan->currentAlbum())
        mIconView->updateContents();
}

void DigikamView::slotAlbumHistoryBack(int steps)
{
    Album *album;
    QWidget *widget;
    
    mAlbumHistory->back(&album, &widget, steps);

    if (album && widget)
    {
        QListViewItem *item;
        item = (QListViewItem*)album->extraData(widget);
        if(!item)
            return;
 
        if (FolderView *v = dynamic_cast<FolderView*>(widget))
        {
            v->setSelected(item, true);
            v->ensureItemVisible(item);            
        } 
        else if (DateFolderView *v = dynamic_cast<DateFolderView*>(widget))
        {
            v->setSelected(item);
        }
        
        mMainSidebar->setActiveTab(widget);
        
        mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
        mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());            
     }
}

void DigikamView::slotAlbumHistoryForward(int steps)
{
    Album *album;
    QWidget *widget;
    
    mAlbumHistory->forward(&album, &widget, steps);

    if (album && widget)
    {
        QListViewItem *item;
        item = (QListViewItem*)album->extraData(widget);
        if(!item)
            return;
 
        if (FolderView *v = dynamic_cast<FolderView*>(widget))
        {
            v->setSelected(item, true);
            v->ensureItemVisible(item);
        } 
        else if (DateFolderView *v = dynamic_cast<DateFolderView*>(widget))
        {
            v->setSelected(item);
        }
        
        mMainSidebar->setActiveTab(widget);
        
        mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
        mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());
    }
}

void DigikamView::clearHistory()
{
    mAlbumHistory->clearHistory();
    mParent->enableAlbumBackwardHistory(false);
    mParent->enableAlbumForwardHistory(false);
}

void DigikamView::getBackwardHistory(QStringList &titles)
{
    mAlbumHistory->getBackwardHistory(titles);
}

void DigikamView::getForwardHistory(QStringList &titles)
{
    mAlbumHistory->getForwardHistory(titles);
}

void DigikamView::slotSelectAlbum(const KURL &)
{
    /* TODO
    if (url.isEmpty())
        return;
    
    Album *album = mAlbumMan->findPAlbum(url);
    if(album && album->getViewItem())
    {
        AlbumFolderItem_Deprecated *item;
        item = static_cast<AlbumFolderItem_Deprecated*>(album->getViewItem());
        mFolderView_Deprecated->setSelected(item);
        mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
        mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());
    }
    */
}

// ----------------------------------------------------------------

void DigikamView::slot_albumSelected(Album* album)
{
    emit signal_noCurrentItem();
    
    if (!album)
    {
        mIconView->setAlbum(0);
        emit signal_albumSelected(false);
        emit signal_tagSelected(false);
        return;
    }

    if (album->type() == Album::PHYSICAL)
    {
        emit signal_albumSelected(true);
        emit signal_tagSelected(false);
    }
    else if (album->type() == Album::TAG)
    {
        emit signal_albumSelected(false);
        emit signal_tagSelected(true);
    }
    
    mAlbumHistory->addAlbum(album, mMainSidebar->getActiveTab());
    mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
    mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());    
    
    mIconView->setAlbum(album);
}

void DigikamView::slot_albumOpenInKonqui()
{
    Album *album = mAlbumMan->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    PAlbum* palbum = dynamic_cast<PAlbum*>(album);
       
    new KRun(palbum->folderPath()); // KRun will delete itself.
}

void DigikamView::slot_imageSelected()
{
    bool selected = false;

    for (IconItem* item=mIconView->firstItem(); item; item=item->nextItem())
    {
        if (item->isSelected())
        {
            selected = true;
            AlbumIconItem *firstSelectedItem = mIconView->firstSelectedItem();
            mRightSidebar->itemChanged(firstSelectedItem->imageInfo()->kurl(), mIconView, firstSelectedItem, 0, 0);

            break;
        }
    }

    if (!selected)
       emit signal_noCurrentItem();

    emit signal_imageSelected(selected);
}

void DigikamView::slot_albumsCleared()
{
    mIconView->clear();
    emit signal_albumSelected(false);
}

// ----------------------------------------------------------------

void DigikamView::slot_thumbSizePlus()
{
    emit signal_noCurrentItem();

    ThumbnailSize thumbSize;

    switch(mIconView->thumbnailSize().size())
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
        mParent->enableThumbSizePlusAction(false);
    }
    mParent->enableThumbSizeMinusAction(true);

    mIconView->setThumbnailSize(thumbSize);

    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setDefaultIconSize( (int)thumbSize.size() );
}

void DigikamView::slot_thumbSizeMinus()
{
    emit signal_noCurrentItem();
    
    ThumbnailSize thumbSize;

    switch(mIconView->thumbnailSize().size())
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
        mParent->enableThumbSizeMinusAction(false);
    }
    mParent->enableThumbSizePlusAction(true);

    mIconView->setThumbnailSize(thumbSize);

    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setDefaultIconSize( (int)thumbSize.size() );
}

void DigikamView::slot_albumPropsEdit()
{
    mFolderView->albumEdit();
}

void DigikamView::slot_albumAddImages()
{
    Album *album = mAlbumMan->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    PAlbum* palbum = dynamic_cast<PAlbum*>(album);

    KFileDialog dlg(QString::null,
                    AlbumSettings::instance()->getImageFileFilter(),
                    this, "filedialog", true);
    dlg.setOperationMode(KFileDialog::Opening );
    dlg.setCaption(i18n("Add Images"));
    dlg.setMode(KFile::Files);
    dlg.okButton()->setText(i18n("&Add"));
    dlg.exec();          

    KURL::List urls = dlg.selectedURLs();
    
    if (!urls.isEmpty())
    {
        KIO::Job* job = DIO::copy(urls, palbum->kurl());
        connect(job, SIGNAL(result(KIO::Job *) ),
                this, SLOT(slot_imageCopyResult(KIO::Job *)));
    }
}

void DigikamView::slotAlbumImportFolder()
{
    mFolderView->albumImportFolder();
}

void DigikamView::slot_albumHighlight()
{
    // TODO:
    // Don't know what this is supposed to do.
    // Perhaps some flashing or other eye kandy
    /*
    Album *album = mAlbumMan->currentAlbum();
    if (!album || !album->type() == Album::PHYSICAL)
        return;

    mFolderView->setAlbumThumbnail(dynamic_cast<PAlbum*>(album));
    */
}

void DigikamView::slot_imageCopyResult(KIO::Job* job)
{
    if (job->error())
        job->showErrorDialog(this);
}

// ----------------------------------------------------------------

void DigikamView::slot_imageView(AlbumIconItem *iconItem)
{
    AlbumIconItem *item;

    if (!iconItem)
    {
        item = mIconView->firstSelectedItem();
        if (!item) return;
    }
    else
    {
        item = iconItem;
    }

    mIconView->slotDisplayItem(item);
}

void DigikamView::slot_imageExifOrientation(int orientation)
{
    mIconView->slotSetExifOrientation(orientation);
}

void DigikamView::slot_imageRename(AlbumIconItem *iconItem)
{
    AlbumIconItem *item;

    if (!iconItem)
    {
        item = mIconView->firstSelectedItem();
        if (!item) return;
    }
    else
    {
        item = iconItem;
    }

    mIconView->slotRename(item);
}

void DigikamView::slot_imageDelete()
{
    mIconView->slotDeleteSelectedItems();
}

void DigikamView::slotSelectAll()
{
    mIconView->selectAll();
}

void DigikamView::slotSelectNone()
{
    mIconView->clearSelection();
}

void DigikamView::slotSelectInvert()
{
    mIconView->invertSelection();
}

void DigikamView::slotSortImages(int order)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setImageSortOrder((AlbumSettings::ImageSortOrder) order);
    mIconView->slotUpdate();
}

void DigikamView::slotLeftSidebarChangedTab(QWidget* w)
{
    mDateFolderView->setActive(w == mDateFolderView);
    mFolderView->setActive(w == mFolderView);
    mTagFolderView->setActive(w == mTagFolderView);
    mSearchFolderView->setActive(w == mSearchFolderView);
}

void DigikamView::slotAssignRating(int rating)
{
    mRightSidebar->slotAssignRating(rating);
}

void DigikamView::slotAssignRatingNoStar()
{
    mRightSidebar->slotAssignRating(0);
}

void DigikamView::slotAssignRatingOneStar()
{
    mRightSidebar->slotAssignRating(1);
}

void DigikamView::slotAssignRatingTwoStar()
{
    mRightSidebar->slotAssignRating(2);
}

void DigikamView::slotAssignRatingThreeStar()
{
    mRightSidebar->slotAssignRating(3);
}

void DigikamView::slotAssignRatingFourStar()
{
    mRightSidebar->slotAssignRating(4);
}

void DigikamView::slotAssignRatingFiveStar()
{
    mRightSidebar->slotAssignRating(5);
}

}  // namespace Digikam

#include "digikamview.moc"
