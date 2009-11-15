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
                AlbumModel *model) :
    SideBarWidget(parent), d(new AlbumFolderViewSideBarWidgetPriv)
{

    d->albumModificationHelper = new AlbumModificationHelper(this, this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->albumFolderView = new AlbumFolderViewNew(this, model, d->albumModificationHelper);
    d->searchTextBar   = new SearchTextBar(this, "DigikamViewFolderSearchBar");
    d->searchTextBar->setModel(model);

    layout->addWidget(d->albumFolderView);
    layout->addWidget(d->searchTextBar);

    // setup connection
    connect(d->searchTextBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->albumFolderView->albumFilterModel(), SLOT(setSearchTextSettings(const SearchTextSettings&)));
    connect(d->albumFolderView->albumFilterModel(), SIGNAL(hasSearchResult(bool)),
            d->searchTextBar, SLOT(slotSearchResult(bool)));

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
    d->albumFolderView->loadViewState(group);
}

void AlbumFolderViewSideBarWidget::saveViewState(KConfigGroup &group)
{
    d->albumFolderView->saveViewState(group);
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
    TagFolderView *tagFolderView;
};

TagViewSideBarWidget::TagViewSideBarWidget(QWidget *parent,
                TagModel *model) :
    SideBarWidget(parent), d(new TagViewSideBarWidgetPriv)
{

    d->tagModel = model;

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->tagFolderView = new TagFolderView(this);
    d->tagSearchBar  = new SearchTextBar(this, "DigikamViewTagSearchBar");

    layout->addWidget(d->tagFolderView);
    layout->addWidget(d->tagSearchBar);

    connect(d->tagSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->tagFolderView, SLOT(slotTextTagFilterChanged(const SearchTextSettings&)));
    connect(d->tagFolderView, SIGNAL(signalFindDuplicatesInTag(Album*)),
            this, SIGNAL(signalFindDuplicatesInAlbum(Album*)));
    connect(d->tagFolderView, SIGNAL(signalTextTagFilterMatch(bool)),
            d->tagSearchBar, SLOT(slotSearchResult(bool)));

    // TODO update, legacy signal passing
    connect(d->tagFolderView, SIGNAL(signalProgressBarMode(int, const QString&)),
            this, SIGNAL(signalProgressBarMode(int, const QString&)));
    connect(d->tagFolderView, SIGNAL(signalProgressValue(int)),
            this, SIGNAL(signalProgressValue(int)));

}

TagViewSideBarWidget::~TagViewSideBarWidget()
{
    delete d;
}

void TagViewSideBarWidget::setActive(bool active)
{
    d->tagFolderView->setActive(active);
}

void TagViewSideBarWidget::loadViewState(KConfigGroup &group)
{
}

void TagViewSideBarWidget::saveViewState(KConfigGroup &group)
{
}

void TagViewSideBarWidget::applySettings()
{
}

void TagViewSideBarWidget::changeAlbumFromHistory(Album *album)
{
    Q3ListViewItem *item = (Q3ListViewItem*) album->extraData(d->tagFolderView);
    if (!item)
        return;

    d->tagFolderView->setSelected(item, true);
    d->tagFolderView->ensureItemVisible(item);
}

QPixmap TagViewSideBarWidget::getIcon()
{
    return SmallIcon("tag");
}

QString TagViewSideBarWidget::getCaption()
{
    return i18n("Tags");
}

void TagViewSideBarWidget::refresh()
{
    d->tagFolderView->refresh();
}

void TagViewSideBarWidget::selectItem(int tagID)
{
    // Set the current tag in the tag folder view.
    d->tagFolderView->selectItem(tagID);
}

void TagViewSideBarWidget::slotNewTag()
{
    d->tagFolderView->tagNew();
}

void TagViewSideBarWidget::slotDeleteTag()
{
    d->tagFolderView->tagDelete();
}

void TagViewSideBarWidget::slotEditTag()
{
    d->tagFolderView->tagEdit();
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
};

TimelineSideBarWidget::TimelineSideBarWidget(QWidget *parent) :
    SideBarWidget(parent), d(new TimelineSideBarWidgetPriv)
{

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->timeLineView = new TimeLineView(this);

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
};

SearchSideBarWidget::SearchSideBarWidget(QWidget *parent) :
    SideBarWidget(parent), d(new SearchSideBarWidgetPriv)
{

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->searchTabHeader  = new SearchTabHeader(this);
    d->searchFolderView = new SearchFolderView(this);
    d->searchSearchBar  = new SearchTextBar(this, "DigikamViewSearchSearchBar");

    layout->addWidget(d->searchTabHeader);
    layout->addWidget(d->searchFolderView);
    layout->addWidget(d->searchSearchBar);

    connect(d->searchSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->searchFolderView, SLOT(slotTextSearchFilterChanged(const SearchTextSettings&)));

    connect(d->searchFolderView, SIGNAL(newSearch()),
            d->searchTabHeader, SLOT(newAdvancedSearch()));

    connect(d->searchFolderView, SIGNAL(signalTextSearchFilterMatch(bool)),
            d->searchSearchBar, SLOT(slotSearchResult(bool)));

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
};

FuzzySearchSideBarWidget::FuzzySearchSideBarWidget(QWidget *parent) :
    SideBarWidget(parent), d(new FuzzySearchSideBarWidgetPriv)
{

    d->fuzzySearchView  = new FuzzySearchView(this);

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
};

GPSSearchSideBarWidget::GPSSearchSideBarWidget(QWidget *parent) :
    SideBarWidget(parent), d(new GPSSearchSideBarWidgetPriv)
{

    d->gpsSearchView    = new GPSSearchView(this);

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
