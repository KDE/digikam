/*
 * leftsidebarwidgets.cpp
 *
 *  Created on: 14.11.2009
 *      Author: languitar
 */

#include "leftsidebarwidgets.h"
#include "leftsidebarwidgets.moc"

// QT includes
#include <qlayout.h>

// KDE includes
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

// Local includes
#include "albumfolderview.h"
#include "albumsettings.h"
#include "searchtextbar.h"
#include "albummodificationhelper.h"
#include "albummanager.h"
#include "tagfolderview.h"
#include "datefolderview.h"
#include "timelineview.h"
#include "timelinefolderview.h"
#include "searchfolderview.h"
#include "searchtabheader.h"
#include "fuzzysearchview.h"
#include "fuzzysearchfolderview.h"

namespace Digikam
{

class AlbumFolderViewSideBarWidgetPriv
{
public:
    AlbumFolderViewSideBarWidgetPriv() :
        albumFolderView(0),
        searchTextBar(0)
    {
    }

    AlbumModificationHelper *albumModificationHelper;
    AlbumFolderViewNew *albumFolderView;
    SearchTextBar *searchTextBar;
};

AlbumFolderViewSideBarWidget::AlbumFolderViewSideBarWidget(QWidget *parent,
                AlbumModel *model, AlbumModificationHelper *albumModificationHelper) :
    SideBarWidget(parent), d(new AlbumFolderViewSideBarWidgetPriv)
{

    d->albumModificationHelper = albumModificationHelper;

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->albumFolderView = new AlbumFolderViewNew(this, model, d->albumModificationHelper);
    d->searchTextBar   = new SearchTextBar(this, "DigikamViewFolderSearchBar");
    d->searchTextBar->setHighlightOnCompletion(true);
    d->searchTextBar->setModel(model, AbstractAlbumModel::AlbumIdRole);

    layout->addWidget(d->albumFolderView);
    layout->addWidget(d->searchTextBar);

    // setup connection
    connect(d->searchTextBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->albumFolderView->albumFilterModel(), SLOT(setSearchTextSettings(const SearchTextSettings&)));

    connect(d->albumFolderView, SIGNAL(signalFindDuplicatesInAlbum(Album*)),
            this, SIGNAL(signalFindDuplicatesInAlbum(Album*)));

}

AlbumFolderViewSideBarWidget::~AlbumFolderViewSideBarWidget()
{
    delete d;
}

void AlbumFolderViewSideBarWidget::setActive(bool active)
{
    if (active)
    {
        AlbumManager::instance()->setCurrentAlbum(
                        d->albumFolderView->currentAlbum());
    }
}

void AlbumFolderViewSideBarWidget::loadViewState(KConfigGroup &group)
{
    d->albumFolderView->loadViewState(group, "AlbumFolderView");
}

void AlbumFolderViewSideBarWidget::saveViewState(KConfigGroup &group)
{
    d->albumFolderView->saveViewState(group, "AlbumFolderView");
}

void AlbumFolderViewSideBarWidget::applySettings()
{
    kDebug() << "applying settings";
    AlbumSettings *settings = AlbumSettings::instance();
    d->albumFolderView->setEnableToolTips(settings->getShowAlbumToolTips());
}

void AlbumFolderViewSideBarWidget::changeAlbumFromHistory(Album *album)
{
    d->albumFolderView->slotSelectAlbum(album);
}

void AlbumFolderViewSideBarWidget::slotSelectAlbum(Album *album)
{

    kDebug() << "received request to go to album and item";

    // Change the current album in list view.
    d->albumFolderView->slotSelectAlbum(album);

}

QPixmap AlbumFolderViewSideBarWidget::getIcon()
{
    return SmallIcon("folder-image");
}

QString AlbumFolderViewSideBarWidget::getCaption()
{
    return i18n("Albums");
}

// -----------------------------------------------------------------------------

class TagViewSideBarWidgetPriv
{
public:
    TagViewSideBarWidgetPriv() :
        tagModel(0)
    {
    }

    TagModel *tagModel;
    SearchTextBar *tagSearchBar;
    TagFolderViewNew *tagFolderView;
    TagModificationHelper *tagModificationHelper;
};

TagViewSideBarWidget::TagViewSideBarWidget(QWidget *parent,
                TagModel *model, TagModificationHelper *tagModificationHelper) :
    SideBarWidget(parent), d(new TagViewSideBarWidgetPriv)
{

    d->tagModel = model;
    d->tagModificationHelper = tagModificationHelper;

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->tagFolderView = new TagFolderViewNew(this, model, tagModificationHelper);
    d->tagSearchBar  = new SearchTextBar(this, "DigikamViewTagSearchBar");
    d->tagSearchBar->setHighlightOnCompletion(true);
    d->tagSearchBar->setModel(model, AbstractAlbumModel::AlbumIdRole);

    layout->addWidget(d->tagFolderView);
    layout->addWidget(d->tagSearchBar);

    connect(d->tagSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->tagFolderView, SLOT(setSearchTextSettings(const SearchTextSettings&)));
    connect(d->tagFolderView, SIGNAL(signalFindDuplicatesInAlbum(Album*)),
            this, SIGNAL(signalFindDuplicatesInAlbum(Album*)));

}

TagViewSideBarWidget::~TagViewSideBarWidget()
{
    delete d;
}

void TagViewSideBarWidget::setActive(bool active)
{
    if (active)
    {
        AlbumManager::instance()->setCurrentAlbum(
                        d->tagFolderView->currentAlbum());
    }
}

void TagViewSideBarWidget::loadViewState(KConfigGroup &group)
{
    d->tagFolderView->loadViewState(group, "TagFolderView");
}

void TagViewSideBarWidget::saveViewState(KConfigGroup &group)
{
    d->tagFolderView->saveViewState(group, "TagFolderView");
}

void TagViewSideBarWidget::applySettings()
{
}

void TagViewSideBarWidget::changeAlbumFromHistory(Album *album)
{
    d->tagFolderView->slotSelectAlbum(album);
}

QPixmap TagViewSideBarWidget::getIcon()
{
    return SmallIcon("tag");
}

QString TagViewSideBarWidget::getCaption()
{
    return i18n("Tags");
}

void TagViewSideBarWidget::slotSelectAlbum(Album *album)
{
    d->tagFolderView->slotSelectAlbum(album);
}

// -----------------------------------------------------------------------------

class DateFolderViewSideBarWidgetPriv
{
public:
    DateFolderViewSideBarWidgetPriv() :
        dateFolderView(0)
    {
    }

    DateFolderView *dateFolderView;
};

DateFolderViewSideBarWidget::DateFolderViewSideBarWidget(QWidget *parent,
                DateAlbumModel *model, ImageAlbumFilterModel *imageFilterModel) :
    SideBarWidget(parent), d(new DateFolderViewSideBarWidgetPriv)
{

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->dateFolderView = new DateFolderView(this);
    d->dateFolderView->setImageModel(imageFilterModel);

    layout->addWidget(d->dateFolderView);

}

DateFolderViewSideBarWidget::~DateFolderViewSideBarWidget()
{
    delete d;
}

void DateFolderViewSideBarWidget::setActive(bool active)
{
    d->dateFolderView->setActive(active);
}

void DateFolderViewSideBarWidget::loadViewState(KConfigGroup &group)
{
}

void DateFolderViewSideBarWidget::saveViewState(KConfigGroup &group)
{
}

void DateFolderViewSideBarWidget::applySettings()
{
}

void DateFolderViewSideBarWidget::changeAlbumFromHistory(Album *album)
{
    Q3ListViewItem *item = (Q3ListViewItem*) album->extraData(d->dateFolderView);
    if (!item)
        return;
    d->dateFolderView->setSelected(item);
}

void DateFolderViewSideBarWidget::gotoDate(const QDate &date)
{
    d->dateFolderView->gotoDate(date);
}

void DateFolderViewSideBarWidget::refresh()
{
    d->dateFolderView->refresh();
}

QPixmap DateFolderViewSideBarWidget::getIcon()
{
    return SmallIcon("view-calendar-list");
}

QString DateFolderViewSideBarWidget::getCaption()
{
    return i18n("Calendar");
}

// -----------------------------------------------------------------------------

class TimelineSideBarWidgetPriv
{
public:
    TimelineSideBarWidgetPriv() :
        timeLineView(0)
    {
    }

    TimeLineView *timeLineView;
    SearchModel *searchModel;
};

TimelineSideBarWidget::TimelineSideBarWidget(QWidget *parent, SearchModel *searchModel) :
    SideBarWidget(parent), d(new TimelineSideBarWidgetPriv)
{

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->timeLineView = new TimeLineView(this);
    d->timeLineView->searchBar()->setModel(searchModel, AbstractAlbumModel::AlbumIdRole);

    layout->addWidget(d->timeLineView);

}

TimelineSideBarWidget::~TimelineSideBarWidget()
{
    delete d;
}

void TimelineSideBarWidget::setActive(bool active)
{
    d->timeLineView->setActive(active);
}

void TimelineSideBarWidget::loadViewState(KConfigGroup &group)
{
}

void TimelineSideBarWidget::saveViewState(KConfigGroup &group)
{
}

void TimelineSideBarWidget::applySettings()
{
}

void TimelineSideBarWidget::changeAlbumFromHistory(Album *album)
{
    Q3ListViewItem *item = (Q3ListViewItem*) album->extraData(d->timeLineView->folderView());
    if (!item)
        return;

    d->timeLineView->folderView()->setSelected(item, true);
    d->timeLineView->folderView()->ensureItemVisible(item);

}

QPixmap TimelineSideBarWidget::getIcon()
{
    return SmallIcon("player-time");
}

QString TimelineSideBarWidget::getCaption()
{
    return i18n("Timeline");
}

// -----------------------------------------------------------------------------

class SearchSideBarWidgetPriv
{
public:
    SearchSideBarWidgetPriv() :
        searchSearchBar(0),
        searchFolderView(0),
        searchTabHeader(0)
    {
    }

    SearchTextBar*            searchSearchBar;
    SearchFolderView*         searchFolderView;
    SearchTabHeader*          searchTabHeader;
    SearchModel *searchModel;
};

SearchSideBarWidget::SearchSideBarWidget(QWidget *parent, SearchModel *searchModel) :
    SideBarWidget(parent), d(new SearchSideBarWidgetPriv)
{

    d->searchModel = searchModel;

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->searchTabHeader  = new SearchTabHeader(this);
    d->searchFolderView = new SearchFolderView(this);
    d->searchSearchBar  = new SearchTextBar(this, "DigikamViewSearchSearchBar");
    d->searchSearchBar->setHighlightOnCompletion(true);
    d->searchSearchBar->setModel(searchModel, AbstractAlbumModel::AlbumIdRole);

    layout->addWidget(d->searchTabHeader);
    layout->addWidget(d->searchFolderView);
    layout->addWidget(d->searchSearchBar);

    connect(d->searchSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->searchFolderView, SLOT(slotTextSearchFilterChanged(const SearchTextSettings&)));

    connect(d->searchFolderView, SIGNAL(newSearch()),
            d->searchTabHeader, SLOT(newAdvancedSearch()));

    connect(d->searchFolderView, SIGNAL(editSearch(SAlbum *)),
            d->searchTabHeader, SLOT(editSearch(SAlbum *)));

    connect(d->searchFolderView, SIGNAL(selectedSearchChanged(SAlbum *)),
            d->searchTabHeader, SLOT(selectedSearchChanged(SAlbum *)));

    connect(d->searchTabHeader, SIGNAL(searchShallBeSelected(SAlbum *)),
            d->searchFolderView, SLOT(slotSelectSearch(SAlbum *)));

}

SearchSideBarWidget::~SearchSideBarWidget()
{
    delete d;
}

void SearchSideBarWidget::setActive(bool active)
{
}

void SearchSideBarWidget::loadViewState(KConfigGroup &group)
{
}

void SearchSideBarWidget::saveViewState(KConfigGroup &group)
{
}

void SearchSideBarWidget::applySettings()
{
}

void SearchSideBarWidget::changeAlbumFromHistory(Album *album)
{
    Q3ListViewItem *item = (Q3ListViewItem*) album->extraData(d->searchFolderView);
    if (!item)
        return;

    d->searchFolderView->setSelected(item, true);
    d->searchFolderView->ensureItemVisible(item);
}

QPixmap SearchSideBarWidget::getIcon()
{
    return SmallIcon("edit-find");
}

QString SearchSideBarWidget::getCaption()
{
    return i18n("Searches");
}

void SearchSideBarWidget::newKeywordSearch()
{
    d->searchTabHeader->newKeywordSearch();
}

void SearchSideBarWidget::newAdvancedSearch()
{
    d->searchTabHeader->newAdvancedSearch();
}

// -----------------------------------------------------------------------------

class FuzzySearchSideBarWidgetPriv
{
public:
    FuzzySearchSideBarWidgetPriv() :
        fuzzySearchView(0)
    {
    }

    FuzzySearchView*          fuzzySearchView;
    SearchModel *searchModel;
};

FuzzySearchSideBarWidget::FuzzySearchSideBarWidget(QWidget *parent, SearchModel *searchModel) :
    SideBarWidget(parent), d(new FuzzySearchSideBarWidgetPriv)
{

    d->searchModel = searchModel;

    d->fuzzySearchView  = new FuzzySearchView(this);
    d->fuzzySearchView->searchBar()->setModel(searchModel, AbstractAlbumModel::AlbumIdRole);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(d->fuzzySearchView);

    connect(d->fuzzySearchView, SIGNAL(signalUpdateFingerPrints()),
            this, SIGNAL(sginalRebuildFingerPrints()));
    connect(d->fuzzySearchView, SIGNAL(signalGenerateFingerPrintsFirstTime()),
            this, SIGNAL(signalGenerateFingerPrintsFirstTime()));

}

FuzzySearchSideBarWidget::~FuzzySearchSideBarWidget()
{
    delete d;
}

void FuzzySearchSideBarWidget::setActive(bool active)
{
}

void FuzzySearchSideBarWidget::loadViewState(KConfigGroup &group)
{
}

void FuzzySearchSideBarWidget::saveViewState(KConfigGroup &group)
{
}

void FuzzySearchSideBarWidget::applySettings()
{
}

void FuzzySearchSideBarWidget::changeAlbumFromHistory(Album *album)
{
    Q3ListViewItem *item = (Q3ListViewItem*) album->extraData(
                    d->fuzzySearchView->folderView());
    if (!item)
        return;

    d->fuzzySearchView->folderView()->setSelected(item, true);
    d->fuzzySearchView->folderView()->ensureItemVisible(item);
}

QPixmap FuzzySearchSideBarWidget::getIcon()
{
    return SmallIcon("tools-wizard");
}

QString FuzzySearchSideBarWidget::getCaption()
{
    return i18n("Fuzzy Searches");
}

void FuzzySearchSideBarWidget::newDuplicatesSearch(Album *album)
{
    d->fuzzySearchView->newDuplicatesSearch(album);
}

void FuzzySearchSideBarWidget::newSimilarSearch(const ImageInfo &imageInfo)
{

    if (imageInfo.isNull())
    {
        return;
    }

    d->fuzzySearchView->setImageInfo(imageInfo);

}

// -----------------------------------------------------------------------------

#ifdef HAVE_MARBLEWIDGET

class GPSSearchSideBarWidgetPriv
{
public:
    GPSSearchSideBarWidgetPriv() :
        gpsSearchView(0)
    {
    }

    GPSSearchView*            gpsSearchView;
    SearchModel *searchModel;
};

GPSSearchSideBarWidget::GPSSearchSideBarWidget(QWidget *parent, SearchModel *searchModel) :
    SideBarWidget(parent), d(new GPSSearchSideBarWidgetPriv)
{

    d->searchModel = searchModel;

    d->gpsSearchView    = new GPSSearchView(this);
    d->gpsSearchView->searchBar()->setModel(searchModel, AbstractAlbumModel::AlbumIdRole);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(d->gpsSearchView);

    connect(d->gpsSearchView, SIGNAL(signalMapSelectedItems(const KUrl::List)),
            this, SIGNAL(signalMapSelectedItems(const KUrl::List&)));

    connect(d->gpsSearchView, SIGNAL(signalMapSoloItems(const KUrl::List, const QString&)),
            this, SIGNAL(signalMapSoloItems(const KUrl::List, const QString&)));

}

GPSSearchSideBarWidget::~GPSSearchSideBarWidget()
{
    delete d;
}

void GPSSearchSideBarWidget::setActive(bool active)
{
}

void GPSSearchSideBarWidget::loadViewState(KConfigGroup &group)
{
}

void GPSSearchSideBarWidget::saveViewState(KConfigGroup &group)
{
}

void GPSSearchSideBarWidget::applySettings()
{
}

void GPSSearchSideBarWidget::changeAlbumFromHistory(Album *album)
{
}

QPixmap GPSSearchSideBarWidget::getIcon()
{
    return SmallIcon("applications-internet");
}

QString GPSSearchSideBarWidget::getCaption()
{
    return i18n("Map Searches");
}

void GPSSearchSideBarWidget::slotDigikamViewNoCurrentItem()
{
    d->gpsSearchView->slotDigikamViewNoCurrentItem();
}

// TODO what are the variable names?
void GPSSearchSideBarWidget::slotDigikamViewImageSelected(const ImageInfoList &selectedImage, bool hasPrevious, bool hasNext, const ImageInfoList &allImages)
{
    d->gpsSearchView->slotDigikamViewImageSelected(selectedImage, hasPrevious, hasNext, allImages);
}

#endif

}
