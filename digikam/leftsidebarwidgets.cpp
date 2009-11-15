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

// Local includes
#include "albumfolderview.h"
#include "searchtextbar.h"
#include "albummodificationhelper.h"
#include "albummanager.h"

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

AlbumFolderViewSideBarWidget::AlbumFolderViewSideBarWidget(QWidget *parent) :
    SideBarWidget(parent), d(new AlbumFolderViewSideBarWidgetPriv)
{

    d->albumModificationHelper = new AlbumModificationHelper(this, this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->albumFolderView = new AlbumFolderViewNew(this, d->albumModificationHelper);
    d->searchTextBar   = new SearchTextBar(this, "DigikamViewFolderSearchBar");

    layout->addWidget(d->albumFolderView);
    layout->addWidget(d->searchTextBar);

    // setup connection
    connect(d->searchTextBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->albumFolderView->albumFilterModel(), SLOT(setSearchTextSettings(const SearchTextSettings&)));
    connect(d->albumFolderView->albumFilterModel(), SIGNAL(hasSearchResult(bool)),
            d->searchTextBar, SLOT(slotSearchResult(bool)));

    connect(d->albumFolderView, SIGNAL(signalFindDuplicatesInAlbum),
            this, SIGNAL(signalFindDuplicatesInAlbum));

}

AlbumFolderViewSideBarWidget::~AlbumFolderViewSideBarWidget()
{
    delete d;
}

void AlbumFolderViewSideBarWidget::setActive(bool active)
{
    // TODO reselect current album
}

void AlbumFolderViewSideBarWidget::loadViewState(KConfigGroup &group)
{
    d->albumFolderView->loadViewState(group);
}

void AlbumFolderViewSideBarWidget::saveViewState(KConfigGroup &group)
{
    d->albumFolderView->saveViewState(group);
}

void AlbumFolderViewSideBarWidget::changeAlbumFromHistory(Album *album)
{
    d->albumFolderView->slotSelectAlbum(album);
}

void AlbumFolderViewSideBarWidget::gotoAlbumAndItem(const ImageInfo &info)
{

    Album* album = dynamic_cast<Album*> (AlbumManager::instance()->findPAlbum(
                    info.albumId()));

    // Change the current album in list view.
    d->albumFolderView->slotSelectAlbum(album);

    // TODO update, request focus on the sidebar, new method neede for that
    // Change to (physical) Album view.
    // Note, that this also opens the side bar if it is closed; this is
    // considered as a feature, because it highlights that the view was changed.
    //d->leftSideBar->setActiveTab(d->folderBox);

}

QPixmap AlbumFolderViewSideBarWidget::getIcon()
{
    return SmallIcon("folder-image");
}

QString AlbumFolderViewSideBarWidget::getCaption()
{
    return i18n("Albums");
}

}
