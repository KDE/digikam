/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : implementation of album view interface. 
 *
 * Copyright (C) 2002-2005 by Renchi Raju  <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qvbox.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qstrlist.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qlabel.h>
#include <qimage.h>
#include <qevent.h>
#include <qapplication.h>
#include <qsplitter.h>
#include <qtimer.h>
#include <qlistview.h>

// KDE includes.

#include <kurl.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <krun.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
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
#include "imagepreviewview.h"
#include "datefolderview.h"
#include "tagfolderview.h"
#include "searchfolderview.h"
#include "searchtextbar.h"
#include "statusprogressbar.h"
#include "tagfilterview.h"
#include "timelineview.h"
#include "timelinefolderview.h"
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
        folderBox             = 0;
        tagBox                = 0;
        searchBox             = 0;
        tagFilterBox          = 0;
        folderSearchBar       = 0;
        tagSearchBar          = 0;
        searchSearchBar       = 0;
        tagFilterSearchBar    = 0;
        splitter              = 0;
        parent                = 0;
        iconView              = 0;
        folderView            = 0;
        albumManager          = 0;
        albumHistory          = 0;
        leftSideBar           = 0;
        rightSideBar          = 0;
        dateFolderView        = 0;
        timeLineView          = 0;
        tagFolderView         = 0;
        searchFolderView      = 0;
        tagFilterView         = 0;
        albumWidgetStack      = 0;
        selectionTimer        = 0;
        thumbSizeTimer        = 0;
        needDispatchSelection = false;
        cancelSlideShow       = false;
        thumbSize             = ThumbnailSize::Medium;
    }

    bool                      needDispatchSelection;
    bool                      cancelSlideShow;

    int                       initialAlbumID;
    int                       thumbSize;

    QSplitter                *splitter;

    QTimer                   *selectionTimer;
    QTimer                   *thumbSizeTimer;

    QVBox                    *folderBox;
    QVBox                    *tagBox;
    QVBox                    *searchBox;
    QVBox                    *tagFilterBox;

    SearchTextBar            *folderSearchBar;
    SearchTextBar            *tagSearchBar;
    SearchTextBar            *searchSearchBar;
    SearchTextBar            *tagFilterSearchBar;

    DigikamApp               *parent;

    AlbumIconView            *iconView;
    AlbumFolderView          *folderView;
    AlbumManager             *albumManager;
    AlbumHistory             *albumHistory;
    AlbumWidgetStack         *albumWidgetStack;

    Sidebar                  *leftSideBar;
    ImagePropertiesSideBarDB *rightSideBar;

    DateFolderView           *dateFolderView;
    TimeLineView             *timeLineView;
    TagFolderView            *tagFolderView;
    SearchFolderView         *searchFolderView;
    TagFilterView            *tagFilterView;
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

    // Folders sidebar tab contents.
    d->folderBox       = new QVBox(this);
    d->folderView      = new AlbumFolderView(d->folderBox);
    d->folderSearchBar = new SearchTextBar(d->folderBox, "DigikamViewFolderSearchBar");
    d->folderBox->setSpacing(KDialog::spacingHint());
    d->folderBox->setMargin(0);

    // Tags sidebar tab contents.
    d->tagBox           = new QVBox(this);
    d->tagFolderView    = new TagFolderView(d->tagBox);
    d->tagSearchBar     = new SearchTextBar(d->tagBox, "DigikamViewTagSearchBar");
    d->tagBox->setSpacing(KDialog::spacingHint());
    d->tagBox->setMargin(0);

    // Search sidebar tab contents.
    d->searchBox        = new QVBox(this);
    d->searchFolderView = new SearchFolderView(d->searchBox);
    d->searchSearchBar  = new SearchTextBar(d->searchBox, "DigikamViewSearchSearchBar");
    d->searchBox->setSpacing(KDialog::spacingHint());
    d->searchBox->setMargin(0);

    d->dateFolderView   = new DateFolderView(this);
    d->timeLineView     = new TimeLineView(this);

    d->leftSideBar->appendTab(d->folderBox, SmallIcon("folder_image"), i18n("Albums"));
    d->leftSideBar->appendTab(d->dateFolderView, SmallIcon("date"), i18n("Calendar"));
    d->leftSideBar->appendTab(d->tagBox, SmallIcon("tag"), i18n("Tags"));
    d->leftSideBar->appendTab(d->timeLineView, SmallIcon("clock"), i18n("Timeline"));
    d->leftSideBar->appendTab(d->searchBox, SmallIcon("find"), i18n("Searches"));

    // To the right.

    // Tags Filter sidebar tab contents.
    d->tagFilterBox       = new QVBox(this);
    d->tagFilterView      = new TagFilterView(d->tagFilterBox);
    d->tagFilterSearchBar = new SearchTextBar(d->tagFilterBox, "DigikamViewTagFilterSearchBar");
    d->tagFilterBox->setSpacing(KDialog::spacingHint());
    d->tagFilterBox->setMargin(0);

    d->rightSideBar->appendTab(d->tagFilterBox, SmallIcon("tag-assigned"), i18n("Tag Filters"));

    d->selectionTimer = new QTimer(this);

    setupConnections();

    d->albumManager->setItemHandler(d->iconView);
    d->albumHistory = new AlbumHistory();
}

DigikamView::~DigikamView()
{
    if (d->thumbSizeTimer)
        delete d->thumbSizeTimer;

    saveViewState();

    delete d->albumHistory;
    d->albumManager->setItemHandler(0);
    delete d;
}

void DigikamView::applySettings()
{
    AlbumSettings *settings = AlbumSettings::instance();
    d->iconView->applySettings(settings);
    d->albumWidgetStack->imagePreviewView()->setLoadFullImageSize(settings->getPreviewLoadFullImageSize());
    refreshView();
}

void DigikamView::refreshView()
{
    d->folderView->refresh();
    d->dateFolderView->refresh();
    d->tagFolderView->refresh();
    d->tagFilterView->refresh();
    d->rightSideBar->refreshTagsView();
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

    connect(d->parent, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotCancelSlideShow()));

    // -- AlbumManager connections --------------------------------

    connect(d->albumManager, SIGNAL(signalAlbumCurrentChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->albumManager, SIGNAL(signalAllAlbumsLoaded()),
            this, SLOT(slotAllAlbumsLoaded()));

    connect(d->albumManager, SIGNAL(signalAlbumItemsSelected(bool) ),
            this, SLOT(slotImageSelected()));

    connect(d->albumManager, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));

    connect(d->albumManager, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));

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

    connect(d->iconView, SIGNAL(signalGotoAlbumAndItem(AlbumIconItem *)),
            this, SLOT(slotGotoAlbumAndItem(AlbumIconItem *)));

    connect(d->iconView, SIGNAL(signalGotoDateAndItem(AlbumIconItem *)),
            this, SLOT(slotGotoDateAndItem(AlbumIconItem *)));

    connect(d->iconView, SIGNAL(signalGotoTagAndItem(int)),
            this, SLOT(slotGotoTagAndItem(int)));

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

    connect(d->parent, SIGNAL(signalResetTagFilters()),
            d->tagFilterView, SLOT(slotResetTagFilters()));

    // -- Filter Bars Connections ---------------------------------

    connect(d->folderSearchBar, SIGNAL(signalTextChanged(const QString&)),
            d->folderView, SLOT(slotTextFolderFilterChanged(const QString&)));

    connect(d->tagSearchBar, SIGNAL(signalTextChanged(const QString&)),
            d->tagFolderView, SLOT(slotTextTagFilterChanged(const QString&)));

    connect(d->searchSearchBar, SIGNAL(signalTextChanged(const QString&)),
            d->searchFolderView, SLOT(slotTextSearchFilterChanged(const QString&)));

    connect(d->tagFilterSearchBar, SIGNAL(signalTextChanged(const QString&)),
            d->tagFilterView, SLOT(slotTextTagFilterChanged(const QString&)));

    connect(d->folderView, SIGNAL(signalTextFolderFilterMatch(bool)),
            d->folderSearchBar, SLOT(slotSearchResult(bool)));

    connect(d->tagFolderView, SIGNAL(signalTextTagFilterMatch(bool)),
            d->tagSearchBar, SLOT(slotSearchResult(bool)));

    connect(d->searchFolderView, SIGNAL(signalTextSearchFilterMatch(bool)),
            d->searchSearchBar, SLOT(slotSearchResult(bool)));

    connect(d->tagFilterView, SIGNAL(signalTextTagFilterMatch(bool)),
            d->tagFilterSearchBar, SLOT(slotSearchResult(bool)));

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

    connect(d->albumWidgetStack, SIGNAL(signalSlideShow()),
            this, SLOT(slotSlideShowAll()));

    connect(d->albumWidgetStack, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->albumWidgetStack, SIGNAL(signalInsert2LightTable()),
            this, SLOT(slotImageLightTable()));

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
    if (d->leftSideBar->getActiveTab() != d->searchBox)
        d->leftSideBar->setActiveTab(d->searchBox);
    d->searchFolderView->quickSearchNew();
}

void DigikamView::slotNewAdvancedSearch()
{
    if (d->leftSideBar->getActiveTab() != d->searchBox)
        d->leftSideBar->setActiveTab(d->searchBox);
    d->searchFolderView->extendedSearchNew();
}

void DigikamView::slotAlbumAdded(Album *album)
{
    if (!album->isRoot())
    {
        switch (album->type())
        {
            case Album::PHYSICAL:
            {
                d->folderSearchBar->lineEdit()->completionObject()->addItem(album->title());
                break;
            }
            case Album::TAG:
            {
                d->tagSearchBar->lineEdit()->completionObject()->addItem(album->title());
                d->tagFilterSearchBar->lineEdit()->completionObject()->addItem(album->title());
                break;
            }
            case Album::SEARCH:
            {
                d->searchSearchBar->lineEdit()->completionObject()->addItem(album->title());
                d->timeLineView->searchBar()->lineEdit()->completionObject()->addItem(album->title());
                break;
            }
            default:
            {
                // Nothing to do with Album::DATE
                break;
            }
        }
    }
}

void DigikamView::slotAlbumDeleted(Album *album)
{
    d->albumHistory->deleteAlbum(album);

    // display changed tags
    if (album->type() == Album::TAG)
        d->iconView->updateContents();

    /*
    // For what is this needed?
    Album *a;
    QWidget *widget;
    d->albumHistory->getCurrentAlbum(&a, &widget);

    changeAlbumFromHistory(a, widget);
    */

    if (!album->isRoot())
    {
        switch (album->type())
        {
            case Album::PHYSICAL:
            {
                d->folderSearchBar->lineEdit()->completionObject()->removeItem(album->title());
                break;
            }
            case Album::TAG:
            {
                d->tagSearchBar->lineEdit()->completionObject()->removeItem(album->title());
                d->tagFilterSearchBar->lineEdit()->completionObject()->removeItem(album->title());
                break;
            }
            case Album::SEARCH:
            {
                d->searchSearchBar->lineEdit()->completionObject()->removeItem(album->title());
                d->timeLineView->searchBar()->lineEdit()->completionObject()->removeItem(album->title());
                break;
            }
            default:
            {
                // Nothing to do with Album::DATE
                break;
            }
        }
    }
}

void DigikamView::slotAlbumRenamed(Album *album)
{
    // display changed names

    if (album == d->albumManager->currentAlbum())
        d->iconView->updateContents();

    if (!album->isRoot())
    {
        switch (album->type())
        {
            case Album::PHYSICAL:
            {
                d->folderSearchBar->lineEdit()->completionObject()->addItem(album->title());
                d->folderView->slotTextFolderFilterChanged(d->folderSearchBar->lineEdit()->text());
                break;
            }
            case Album::TAG:
            {
                d->tagSearchBar->lineEdit()->completionObject()->addItem(album->title());
                d->tagFolderView->slotTextTagFilterChanged(d->tagSearchBar->lineEdit()->text());

                d->tagFilterSearchBar->lineEdit()->completionObject()->addItem(album->title());
                d->tagFilterView->slotTextTagFilterChanged(d->tagFilterSearchBar->lineEdit()->text());
                break;
            }
            case Album::SEARCH:
            {
                d->searchSearchBar->lineEdit()->completionObject()->addItem(album->title());
                d->searchFolderView->slotTextSearchFilterChanged(d->searchSearchBar->lineEdit()->text());

                d->timeLineView->searchBar()->lineEdit()->completionObject()->addItem(album->title());
                d->timeLineView->folderView()->slotTextSearchFilterChanged(d->timeLineView->searchBar()->lineEdit()->text());
                break;
            }
            default:
            {
                // Nothing to do with Album::DATE
                break;
            }
        }
    }
}

void DigikamView::slotAlbumsCleared()
{
    d->iconView->clear();
    emit signalAlbumSelected(false);

    d->folderSearchBar->lineEdit()->completionObject()->clear();

    d->tagSearchBar->lineEdit()->completionObject()->clear();
    d->tagFilterSearchBar->lineEdit()->completionObject()->clear();

    d->searchSearchBar->lineEdit()->completionObject()->clear();
    d->timeLineView->searchBar()->lineEdit()->completionObject()->clear();
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
        QListViewItem *item = 0;

        // Check if widget is a vbox used to host folderview, tagview or searchview.
        if (QVBox *v = dynamic_cast<QVBox*>(widget))
        {
            if (v == d->folderBox)
            {
                item = (QListViewItem*)album->extraData(d->folderView);
                if(!item) return;

                d->folderView->setSelected(item, true);
                d->folderView->ensureItemVisible(item);
            }
            else if (v == d->tagBox)
            {
                item = (QListViewItem*)album->extraData(d->tagFolderView);
                if(!item) return;

                d->tagFolderView->setSelected(item, true);
                d->tagFolderView->ensureItemVisible(item);
            }
            else if (v == d->searchBox)
            {
                item = (QListViewItem*)album->extraData(d->searchFolderView);
                if(!item) return;

                d->searchFolderView->setSelected(item, true);
                d->searchFolderView->ensureItemVisible(item);
            }
        }
        else if (DateFolderView *v = dynamic_cast<DateFolderView*>(widget))
        {
            item = (QListViewItem*)album->extraData(v);
            if(!item) return;
            v->setSelected(item);
        }
        else if (TimeLineView *v = dynamic_cast<TimeLineView*>(widget))
        {
            item = (QListViewItem*)album->extraData(v->folderView());
            if(!item) return;

            v->folderView()->setSelected(item, true);
            v->folderView()->ensureItemVisible(item);
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

void DigikamView::slotGotoAlbumAndItem(AlbumIconItem* iconItem)
{
    KURL url( iconItem->imageInfo()->kurl() );
    url.cleanPath();

    emit signalNoCurrentItem();

    Album* album = iconItem->imageInfo()->album();

    // Change the current album in list view.
    d->folderView->setCurrentAlbum(album);

    // Change to (physical) Album view.
    // Note, that this also opens the side bar if it is closed; this is
    // considered as a feature, because it highlights that the view was changed.
    d->leftSideBar->setActiveTab(d->folderBox);

    // Set the activate item url to find in the Album View after  
    // all items have be reloaded.
    d->iconView->setAlbumItemToFind(url);

    // And finally toggle album manager to handle album history and 
    // reload all items.
    d->albumManager->setCurrentAlbum(album);
}

void DigikamView::slotGotoDateAndItem(AlbumIconItem* iconItem)
{
    KURL url( iconItem->imageInfo()->kurl() );
    url.cleanPath();
    QDate date = iconItem->imageInfo()->dateTime().date();

    emit signalNoCurrentItem();

    // Change to Date Album view.
    // Note, that this also opens the side bar if it is closed; this is
    // considered as a feature, because it highlights that the view was changed.
    d->leftSideBar->setActiveTab(d->dateFolderView);

    // Set the activate item url to find in the Album View after  
    // all items have be reloaded.
    d->iconView->setAlbumItemToFind(url);  

    // Change the year and month of the iconItem (day is unused).
    d->dateFolderView->gotoDate(date); 
}

void DigikamView::slotGotoTagAndItem(int tagID)
{
    // FIXME: Arnd: don't know yet how to get the iconItem passed through ...
    // FIXME: then we would know how to use the following ...
    //  KURL url( iconItem->imageInfo()->kurl() );
    //  url.cleanPath();

    emit signalNoCurrentItem();

    // Change to Tag Folder view.
    // Note, that this also opens the side bar if it is closed; this is
    // considered as a feature, because it highlights that the view was changed.
    d->leftSideBar->setActiveTab(d->tagBox);

    // Set the current tag in the tag folder view.
    d->tagFolderView->selectItem(tagID);

    // Set the activate item url to find in the Tag View after  
    // all items have be reloaded.
    // FIXME: see above
    // d->iconView->setAlbumItemToFind(url);  
}

void DigikamView::slotAlbumSelected(Album* album)
{
    emit signalNoCurrentItem();

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
        // no copy needed for this one, as this list is just used for counting
        // the total number of images
        KURL::List listAll = d->iconView->allItems();

        if (list.isEmpty())
        {
            d->albumWidgetStack->setPreviewItem();
            emit signalImageSelected(list, false, false, listAll);
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
            emit signalImageSelected(list, previousInfo, nextInfo, listAll);
        }

        d->needDispatchSelection = false;
    }
}

void DigikamView::setThumbSize(int size)
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        double h    = (double)ThumbnailSize::Huge;
        double s    = (double)ThumbnailSize::Small;
        double zmin = d->albumWidgetStack->zoomMin();
        double zmax = d->albumWidgetStack->zoomMax();
        double b    = (zmin-(zmax*s/h))/(1-s/h);
        double a    = (zmax-b)/h;
        double z    = a*size+b; 
        d->albumWidgetStack->setZoomFactorSnapped(z);   
    }
    else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        if (size > ThumbnailSize::Huge)
            d->thumbSize = ThumbnailSize::Huge;
        else if (size < ThumbnailSize::Small)
            d->thumbSize = ThumbnailSize::Small;
        else 
            d->thumbSize = size;

        emit signalThumbSizeChanged(d->thumbSize);

        if (d->thumbSizeTimer)
        {
            d->thumbSizeTimer->stop();
            delete d->thumbSizeTimer;
        }
    
        d->thumbSizeTimer = new QTimer( this );
        connect(d->thumbSizeTimer, SIGNAL(timeout()),
                this, SLOT(slotThumbSizeEffect()) );
        d->thumbSizeTimer->start(300, true);    
    }
}

void DigikamView::slotThumbSizeEffect()
{
    emit signalNoCurrentItem();

    d->iconView->setThumbnailSize(d->thumbSize);
    toggleZoomActions();

    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setDefaultIconSize(d->thumbSize);
}

void DigikamView::toggleZoomActions()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);
    
        if (d->albumWidgetStack->maxZoom())
            d->parent->enableZoomPlusAction(false);
    
        if (d->albumWidgetStack->minZoom())
            d->parent->enableZoomMinusAction(false);
    }  
    else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);
    
        if (d->thumbSize >= ThumbnailSize::Huge)
            d->parent->enableZoomPlusAction(false);
    
        if (d->thumbSize <= ThumbnailSize::Small)
            d->parent->enableZoomMinusAction(false);
    }
}

void DigikamView::slotZoomIn()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        setThumbSize(d->thumbSize + ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        d->albumWidgetStack->increaseZoom();
    }
}

void DigikamView::slotZoomOut()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        setThumbSize(d->thumbSize - ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }  
    else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        d->albumWidgetStack->decreaseZoom();
    }
}

void DigikamView::slotZoomTo100Percents()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    { 
        d->albumWidgetStack->toggleFitToWindowOr100();
    }
}

void DigikamView::slotFitToWindow()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
    {
        d->albumWidgetStack->fitToWindow();
    }
}

void DigikamView::slotZoomFactorChanged(double zoom)
{
    toggleZoomActions();

    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->albumWidgetStack->zoomMin();
    double zmax = d->albumWidgetStack->zoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    int size    = (int)((zoom - b) /a); 

    emit signalZoomChanged(zoom, size);
}

void DigikamView::slotAlbumPropsEdit()
{
    d->folderView->albumEdit();
}

void DigikamView::slotAlbumSyncPicturesMetadata()
{
    Album *album = d->albumManager->currentAlbum();
    if (!album)
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
    applySettings();
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

// This method toggle between AlbumView and ImagePreview Modes, depending of context.
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

void DigikamView::slotToggledToPreviewMode(bool b)
{
    toggleZoomActions();

    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
        emit signalThumbSizeChanged(d->iconView->thumbnailSize().size());
    else if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewImageMode)
        slotZoomFactorChanged(d->albumWidgetStack->zoomFactor());

    emit signalTogglePreview(b);
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

void DigikamView::slotLightTable()
{
    ImageInfoList empty;
    d->iconView->insertToLightTable(empty, 0, true);
}

void DigikamView::slotImageLightTable()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        // put images into an emptied light table 
        d->iconView->insertSelectionToLightTable(false);
    }
    else
    {
        ImageInfoList list;
        ImageInfo *info = d->albumWidgetStack->imagePreviewView()->getImageInfo();
        list.append(info);
        // put images into an emptied light table 
        d->iconView->insertToLightTable(list, info, false);
    }
}

void DigikamView::slotImageAddToLightTable()
{
    if (d->albumWidgetStack->previewMode() == AlbumWidgetStack::PreviewAlbumMode)
    {
        // add images to the existing images in the light table 
        d->iconView->insertSelectionToLightTable(true);
    }
    else
    {
        ImageInfoList list;
        ImageInfo *info = d->albumWidgetStack->imagePreviewView()->getImageInfo();
        list.append(info);
        // add images to the existing images in the light table 
        d->iconView->insertToLightTable(list, info, true);
    }
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
    d->iconView->slotRearrange();
}

void DigikamView::slotLeftSidebarChangedTab(QWidget* w)
{
    // setActive means that selection changes are propagated, nothing more.
    // Additionally, when it is set to true, the selectionChanged signal will be emitted.
    // So this is the place which causes the behavior that when the left sidebar
    // tab is changed, the current album is changed as well.
    d->dateFolderView->setActive(w == d->dateFolderView);
    d->folderView->setActive(w == d->folderBox);
    d->tagFolderView->setActive(w == d->tagBox);
    d->searchFolderView->setActive(w == d->searchBox);
    d->timeLineView->setActive(w == d->timeLineView);
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
    emit signalProgressBarMode(StatusProgressBar::CancelProgressBarMode, 
                               i18n("Preparing slideshow of %1 images. Please wait...")
			       .arg(infoList.count()));

    DMetadata         meta;
    SlideShowSettings settings;
    settings.exifRotate           = AlbumSettings::instance()->getExifRotate();
    settings.delay                = config->readNumEntry("SlideShowDelay", 5) * 1000;
    settings.printName            = config->readBoolEntry("SlideShowPrintName", true);
    settings.printDate            = config->readBoolEntry("SlideShowPrintDate", false);
    settings.printApertureFocal   = config->readBoolEntry("SlideShowPrintApertureFocal", false);
    settings.printExpoSensitivity = config->readBoolEntry("SlideShowPrintExpoSensitivity", false);
    settings.printMakeModel       = config->readBoolEntry("SlideShowPrintMakeModel", false);
    settings.printComment         = config->readBoolEntry("SlideShowPrintComment", false);
    settings.loop                 = config->readBoolEntry("SlideShowLoop", false);

    d->cancelSlideShow = false;
    for (ImageInfoList::iterator it = infoList.begin() ; 
         !d->cancelSlideShow && (it != infoList.end()) ; ++it)
    {
        ImageInfo *info = *it;
        settings.fileList.append(info->kurl());
        SlidePictureInfo pictInfo;
        pictInfo.comment = info->caption();

        // Perform optimizations: only read pictures metadata if necessary.
        if (settings.printApertureFocal || settings.printExpoSensitivity || settings.printMakeModel)
        {
            meta.load(info->kurl().path());
            pictInfo.photoInfo = meta.getPhotographInformations();
        }

        // In case of dateTime extraction from metadata failed 
        pictInfo.photoInfo.dateTime = info->dateTime(); 
        settings.pictInfoMap.insert(info->kurl(), pictInfo);

        emit signalProgressValue((int)((i++/cnt)*100.0));
        kapp->processEvents();
    }

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());   

    if (!d->cancelSlideShow)
    {
        SlideShow *slide = new SlideShow(settings);
        if (startWithCurrent)
        {
            AlbumIconItem* current = dynamic_cast<AlbumIconItem*>(d->iconView->currentItem());
            if (current) 
                slide->setCurrent(current->imageInfo()->kurl());
        }
    
        slide->show();
    }
}

void DigikamView::slotCancelSlideShow()
{
    d->cancelSlideShow = true;
}

}  // namespace Digikam
