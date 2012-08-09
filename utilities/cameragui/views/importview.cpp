/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-26-07
 * Description : Main view for import tool
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "importview.moc"

// Qt includes

#include <QTimer>
#include <QShortcut>

// KDE includes

#include <kmessagebox.h>
#include <kdebug.h>

// Local includes

#include "cameraui.h"
#include "importiconview.h"
#include "importstackedview.h"
#include "thumbnailsize.h"
#include "fileactionmngr.h"
#include "importsettings.h"
#include "sidebar.h"
#include "dzoombar.h"
#include "camitemsortsettings.h"

namespace Digikam
{

class ImportView::Private
{
public:

    Private() :
        needDispatchSelection(false),
        thumbSize(ThumbnailSize::Medium),
        dockArea(0),
        splitter(0),
        selectionTimer(0),
        thumbSizeTimer(0),
        parent(0),
        iconView(0),
        StackedView(0),
        lastPreviewMode(ImportStackedView::PreviewCameraMode),
        leftSideBar(0)
        //TODO: rightSideBar(0)
        //FIXME: filterWidget(0)
    {
    }

    void                          addPageUpDownActions(ImportView* const q, QWidget* const w);

public:

    bool                          needDispatchSelection;

    //FIXME: int                           initialAlbumID;
    int                           thumbSize;

    QMainWindow*                  dockArea;

    SidebarSplitter*              splitter;

    QTimer*                       selectionTimer;
    QTimer*                       thumbSizeTimer;

    // left side bar
    CameraUI*                     parent;

    ImportIconView*               iconView;
    //TODO: MapWidgetView*                mapView;
    ImportStackedView*            StackedView;
    int                           lastPreviewMode;

    Sidebar*                      leftSideBar;
    //TODO: ImagePropertiesSideBarDB*     rightSideBar;

    //FIXME: FilterSideBarWidget*          filterWidget;

    QString                       optionAlbumViewPrefix;

    QList<SidebarWidget*>         leftSideBarWidgets;
};

void ImportView::Private::addPageUpDownActions(ImportView* const q, QWidget* const w)
{
    QShortcut *nextImageShortcut = new QShortcut(w);
    nextImageShortcut->setKey(Qt::Key_PageDown);
    nextImageShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(nextImageShortcut, SIGNAL(activated()), q, SLOT(slotNextItem()));

    QShortcut *prevImageShortcut = new QShortcut(w);
    prevImageShortcut->setKey(Qt::Key_PageUp);
    prevImageShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(prevImageShortcut, SIGNAL(activated()), q, SLOT(slotPrevItem()));
}

ImportView::ImportView(CameraUI* const ui, QWidget* const parent)
    : KHBox(parent), d(new Private)
{
    d->parent      = /*static_cast<CameraUI*>*/(ui);

    d->splitter    = new SidebarSplitter;
    d->splitter->setFrameStyle(QFrame::NoFrame);
    d->splitter->setFrameShadow(QFrame::Plain);
    d->splitter->setFrameShape(QFrame::NoFrame);
    d->splitter->setOpaqueResize(false);

    d->leftSideBar = new Sidebar(this, d->splitter, KMultiTabBar::Left);
    d->leftSideBar->setObjectName("Import Left Sidebar");
    d->splitter->setParent(this);

    // The dock area where the thumbnail bar is allowed to go.
    d->dockArea    = new QMainWindow(this, Qt::Widget);
    d->splitter->addWidget(d->dockArea);
    d->StackedView = new ImportStackedView(d->parent->getCameraController(), d->dockArea);
    d->dockArea->setCentralWidget(d->StackedView);
    d->StackedView->setDockArea(d->dockArea);

    d->iconView = d->StackedView->importIconView();
    //TODO: d->mapView  = d->StackedView->mapWidgetView();

    d->addPageUpDownActions(this, d->StackedView->importPreviewView());
    d->addPageUpDownActions(this, d->StackedView->thumbBar());
    //TODO: d->addPageUpDownActions(this, d->StackedView->mediaPlayerView());

    //TODO: d->rightSideBar = new ImagePropertiesSideBarDB(this, d->splitter, KMultiTabBar::Right, true);
    //TODO: d->rightSideBar->setObjectName("Import Right Sidebar");

    foreach(SidebarWidget* leftWidget, d->leftSideBarWidgets)
    {
        d->leftSideBar->appendTab(leftWidget, leftWidget->getIcon(),
                                  leftWidget->getCaption());
        connect(leftWidget, SIGNAL(requestActiveTab(SidebarWidget*)),
                this, SLOT(slotLeftSideBarActivate(SidebarWidget*)));
    }

    // To the right.

    //d->addPageUpDownActions(this, d->rightSideBar->imageDescEditTab());

    d->selectionTimer = new QTimer(this);
    d->selectionTimer->setSingleShot(true);
    d->selectionTimer->setInterval(75);
    d->thumbSizeTimer = new QTimer(this);
    d->thumbSizeTimer->setSingleShot(true);
    d->thumbSizeTimer->setInterval(300);

    slotSidebarTabTitleStyleChanged();
    setupConnections();
}

ImportView::~ImportView()
{
    delete d;
}

void ImportView::applySettings()
{
    foreach(SidebarWidget* sidebarWidget, d->leftSideBarWidgets)
    {
        sidebarWidget->applySettings();
    }

    refreshView();
}

void ImportView::refreshView()
{
    //d->rightSideBar->refreshTagsView();
}

void ImportView::setupConnections()
{
    // -- CameraUI connections ----------------------------------

    connect(d->parent, SIGNAL(signalEscapePressed()),
            this, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalEscapePressed()),
            d->StackedView, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->parent, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->parent, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));

    connect(d->parent, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));

    // -- IconView Connections -------------------------------------

    connect(d->iconView->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(layoutChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(selectionChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(previewRequested(CamItemInfo)),
            this, SLOT(slotTogglePreviewMode(CamItemInfo)));

    connect(d->iconView, SIGNAL(zoomOutStep()),
            this, SLOT(slotZoomOut()));

    connect(d->iconView, SIGNAL(zoomInStep()),
            this, SLOT(slotZoomIn()));

    // -- Sidebar Connections -------------------------------------

    connect(d->leftSideBar, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotLeftSidebarChangedTab(QWidget*)));

    //TODO: connect(d->rightSideBar, SIGNAL(signalFirstItem()),
            //this, SLOT(slotFirstItem()));

    //TODO: connect(d->rightSideBar, SIGNAL(signalNextItem()),
            //this, SLOT(slotNextItem()));

    //TODO: connect(d->rightSideBar, SIGNAL(signalPrevItem()),
            //this, SLOT(slotPrevItem()));

    //TODO: connect(d->rightSideBar, SIGNAL(signalLastItem()),
            //this, SLOT(slotLastItem()));

    //TODO: connect(this, SIGNAL(signalNoCurrentItem()),
            //d->rightSideBar, SLOT(slotNoCurrentItem()));

    // -- Preview image widget Connections ------------------------

    connect(d->StackedView, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->StackedView, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->StackedView, SIGNAL(signalEditItem()),
            this, SLOT(slotImageEdit()));

    connect(d->StackedView, SIGNAL(signalDeleteItem()),
            this, SLOT(slotImageDelete()));

    connect(d->StackedView, SIGNAL(signalViewModeChanged()),
            this, SLOT(slotViewModeChanged()));

    //FIXME: connect(d->StackedView, SIGNAL(signalBack2Album()),
            //this, SLOT(slotEscapePreview()));

    connect(d->StackedView, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    // -- FileActionMngr progress ---------------

    connect(FileActionMngr::instance(), SIGNAL(signalImageChangeFailed(QString, QStringList)),
            this, SLOT(slotImageChangeFailed(QString, QStringList)));

    // -- timers ---------------

    connect(d->selectionTimer, SIGNAL(timeout()),
            this, SLOT(slotDispatchImageSelected()));

    connect(d->thumbSizeTimer, SIGNAL(timeout()),
            this, SLOT(slotThumbSizeEffect()) );

    // -- Import Settings ----------------

    connect(ImportSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSidebarTabTitleStyleChanged()));
}

//void ImportView::connectIconViewFilter(FilterStatusBar* filterbar)
//{
//    ImageAlbumFilterModel* model = d->iconView->imageAlbumFilterModel();

//    connect(model, SIGNAL(filterMatches(bool)),
//            filterbar, SLOT(slotFilterMatches(bool)));

//    connect(model, SIGNAL(filterSettingsChanged(ImageFilterSettings)),
//            filterbar, SLOT(slotFilterSettingsChanged(ImageFilterSettings)));

//    connect(filterbar, SIGNAL(signalResetFilters()),
//            d->filterWidget, SLOT(slotResetFilters()));

//    connect(filterbar, SIGNAL(signalPopupFiltersView()),
//            this, SLOT(slotPopupFiltersView()));
//}

//void ImportView::slotPopupFiltersView()
//{
//    d->rightSideBar->setActiveTab(d->filterWidget);
//    d->filterWidget->setFocusToTextFilter();
//}

void ImportView::loadViewState()
{
    foreach(SidebarWidget* widget, d->leftSideBarWidgets)
    {
        widget->loadState();
    }

    //TODO: d->filterWidget->loadState();

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("MainWindow");

    // Restore the splitter
    d->splitter->restoreState(group);

    // Restore the thumbnail bar dock.
    QByteArray thumbbarState;
    thumbbarState = group.readEntry("ThumbbarState", thumbbarState);
    d->dockArea->restoreState(QByteArray::fromBase64(thumbbarState));

    //TODO: d->mapView->loadState();
    //TODO: d->rightSideBar->loadState();
}

void ImportView::saveViewState()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("MainWindow");

    foreach(SidebarWidget* widget, d->leftSideBarWidgets)
    {
        widget->saveState();
    }

    //TODO: d->filterWidget->saveState();

    // Save the splitter states.
    d->splitter->saveState(group);

    // Save the position and size of the thumbnail bar. The thumbnail bar dock
    // needs to be closed explicitly, because when it is floating and visible
    // (when the user is in image preview mode) when the layout is saved, it
    // also reappears when restoring the view, while it should always be hidden.
    d->StackedView->thumbBarDock()->close();
    group.writeEntry("ThumbbarState", d->dockArea->saveState().toBase64());

    //TODO: d->mapView->saveState();

    //TODO: d->rightSideBar->saveState();
}

QList<SidebarWidget*> ImportView::leftSidebarWidgets()
{
    return d->leftSideBarWidgets;
}

CamItemInfo ImportView::camItemInfo(const QString& folder, const QString& file) const
{
    return d->iconView->camItemInfo(folder, file);
}

KUrl::List ImportView::allUrls() const
{
    return d->iconView->urls();
}

KUrl::List ImportView::selectedUrls() const
{
    return d->iconView->selectedUrls();
}

QList<CamItemInfo> ImportView::selectedCamItemInfos() const
{
    return d->iconView->selectedCamItemInfos();
}

QList<CamItemInfo> ImportView::allItems() const
{
    return d->iconView->camItemInfos();
}

int ImportView::downloadedCamItemInfos() const
{
    QList<CamItemInfo> infos = d->iconView->camItemInfos();
    int numberOfDownloaded = 0;

    foreach(CamItemInfo info, infos)
    {
        if(info.downloaded == CamItemInfo::DownloadedYes)
        {
            ++numberOfDownloaded;
        }
    }

    return numberOfDownloaded;
}

//TODO: Needs testing.
bool ImportView::isSelected(const KUrl url)
{
    QList<KUrl> urlsList = selectedUrls();

    foreach(KUrl selected, urlsList)
    {
        if(url == selected)
        {
            return true;
        }
    }

    return false;
}

void ImportView::showSideBars()
{
    d->leftSideBar->restore();
    //d->rightSideBar->restore();
}

void ImportView::hideSideBars()
{
    d->leftSideBar->backup();
    //d->rightSideBar->backup();
}

void ImportView::slotFirstItem()
{
    d->iconView->toFirstIndex();
}

void ImportView::slotPrevItem()
{
    d->iconView->toPreviousIndex();
}

void ImportView::slotNextItem()
{
    d->iconView->toNextIndex();
}

void ImportView::slotLastItem()
{
    d->iconView->toLastIndex();
}

void ImportView::slotSelectItemByUrl(const KUrl& url)
{
    d->iconView->toIndex(url);
}

void ImportView::slotImageSelected()
{
    // delay to slotDispatchImageSelected
    d->needDispatchSelection = true;
    d->selectionTimer->start();
    emit signalSelectionChanged(d->iconView->numberOfSelectedIndexes());
}

void ImportView::slotDispatchImageSelected()
{
    if (d->needDispatchSelection)
    {
        // the list of CamItemInfos of currently selected items, currentItem first
        // since the iconView tracks the changes also while we are in map widget mode,
        // we can still pull the data from the iconView
        const CamItemInfoList list = d->iconView->selectedCamItemInfosCurrentFirst();

        const CamItemInfoList allImages = d->iconView->camItemInfos();

        if (list.isEmpty())
        {
            d->StackedView->setPreviewItem();
            emit signalImageSelected(list, false, false, allImages);
            emit signalNewSelection(false);
            emit signalNoCurrentItem();
        }
        else
        {
            //TODO: d->rightSideBar->itemChanged(list);

            CamItemInfo previousInfo;
            CamItemInfo nextInfo;

            if (d->StackedView->previewMode() != ImportStackedView::MapWidgetMode)
            {
                previousInfo = d->iconView->previousInfo(list.first());
                nextInfo = d->iconView->nextInfo(list.first());
            }

            if (   (d->StackedView->previewMode() != ImportStackedView::PreviewCameraMode)
                   && (d->StackedView->previewMode() != ImportStackedView::MapWidgetMode) )
            {
                d->StackedView->setPreviewItem(list.first(), previousInfo, nextInfo);
            }

            emit signalImageSelected(list, !previousInfo.isNull(), !nextInfo.isNull(), allImages);
            emit signalNewSelection(true);
        }

        d->needDispatchSelection = false;
    }
}

double ImportView::zoomMin()
{
    return d->StackedView->zoomMin();
}

double ImportView::zoomMax()
{
    return d->StackedView->zoomMax();
}

void ImportView::setZoomFactor(double zoom)
{
    d->StackedView->setZoomFactorSnapped(zoom);
}

void ImportView::slotZoomFactorChanged(double zoom)
{
    toggleZoomActions();
    emit signalZoomChanged(zoom);
}

void ImportView::setThumbSize(int size)
{
    if (d->StackedView->previewMode() == ImportStackedView::PreviewImageMode)
    {
        double z = DZoomBar::zoomFromSize(size, zoomMin(), zoomMax());
        setZoomFactor(z);
    }
    else if (d->StackedView->previewMode() == ImportStackedView::PreviewCameraMode)
    {
        if (size > ThumbnailSize::Huge)
        {
            d->thumbSize = ThumbnailSize::Huge;
        }
        else if (size < ThumbnailSize::Small)
        {
            d->thumbSize = ThumbnailSize::Small;
        }
        else
        {
            d->thumbSize = size;
        }

        emit signalThumbSizeChanged(d->thumbSize);

        d->thumbSizeTimer->start();
    }
}

ThumbnailSize ImportView::thumbnailSize()
{
    return d->thumbSize;
}

void ImportView::slotThumbSizeEffect()
{
    qDebug() << "SETTING THUMBNAIL SIZE IN <<<IMPORT VIEW>>>" << QString::number(d->thumbSize);
    d->iconView->setThumbnailSize(d->thumbSize);
    toggleZoomActions();

    ImportSettings::instance()->setDefaultIconSize(d->thumbSize);
}

void ImportView::toggleZoomActions()
{
    if (d->StackedView->previewMode() == ImportStackedView::PreviewImageMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->StackedView->maxZoom())
        {
            d->parent->enableZoomPlusAction(false);
        }

        if (d->StackedView->minZoom())
        {
            d->parent->enableZoomMinusAction(false);
        }
    }
    else if (d->StackedView->previewMode() == ImportStackedView::PreviewCameraMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->thumbSize >= ThumbnailSize::Huge)
        {
            d->parent->enableZoomPlusAction(false);
        }

        if (d->thumbSize <= ThumbnailSize::Small)
        {
            d->parent->enableZoomMinusAction(false);
        }
    }
    else
    {
        d->parent->enableZoomMinusAction(false);
        d->parent->enableZoomPlusAction(false);
    }
}

void ImportView::slotZoomIn()
{
    if (d->StackedView->previewMode() == ImportStackedView::PreviewCameraMode)
    {
        setThumbSize(d->thumbSize + ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->StackedView->previewMode() == ImportStackedView::PreviewImageMode)
    {
        d->StackedView->increaseZoom();
    }
}

void ImportView::slotZoomOut()
{
    if (d->StackedView->previewMode() == ImportStackedView::PreviewCameraMode)
    {
        setThumbSize(d->thumbSize - ThumbnailSize::Step);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->StackedView->previewMode() == ImportStackedView::PreviewImageMode)
    {
        d->StackedView->decreaseZoom();
    }
}

void ImportView::slotZoomTo100Percents()
{
    if (d->StackedView->previewMode() == ImportStackedView::PreviewImageMode)
    {
        d->StackedView->toggleFitToWindowOr100();
    }
}

void ImportView::slotFitToWindow()
{
    if (d->StackedView->previewMode() == ImportStackedView::PreviewCameraMode)
    {
        int nts = d->iconView->fitToWidthIcons();
        kDebug() << "new thumb size = " << nts;//TODO: Remove this line.
        setThumbSize(nts);
        toggleZoomActions();
        emit signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->StackedView->previewMode() == ImportStackedView::PreviewImageMode)
    {
        d->StackedView->fitToWindow();
    }
}

// ----------------------------------------------------------------

void ImportView::slotEscapePreview()
{
    if (d->StackedView->previewMode() == ImportStackedView::PreviewCameraMode)
        //TODO: || d->StackedView->previewMode() == ImportStackedView::WelcomePageMode)
    {
        return;
    }

    // pass a null camera item info, because we want to fall back to the old
    // view mode
    slotTogglePreviewMode(CamItemInfo());
}

void ImportView::slotMapWidgetView()
{
    d->StackedView->setPreviewMode(ImportStackedView::MapWidgetMode);
}

void ImportView::slotIconView()
{
    if (d->StackedView->previewMode() == ImportStackedView::PreviewImageMode)
    {
        emit signalThumbSizeChanged(d->iconView->thumbnailSize().size());
    }

    // and switch to icon view
    d->StackedView->setPreviewMode(ImportStackedView::PreviewCameraMode);

    // make sure the next/previous buttons are updated
    slotImageSelected();
}

void ImportView::slotImagePreview()
{
    const int currentPreviewMode = d->StackedView->previewMode();
    CamItemInfo currentInfo;

    if (currentPreviewMode == ImportStackedView::PreviewCameraMode)
    {
        currentInfo = d->iconView->currentInfo();
    }
    //TODO: Implement MapWidget
    //else if (currentPreviewMode == ImportStackedView::MapWidgetMode)
    //{
    //    currentInfo = d->mapView->currentInfo();
    //}

    slotTogglePreviewMode(currentInfo);
}

/**
 * @brief This method toggles between IconView/MapWidgetView and ImagePreview modes, depending on the context.
 */
void ImportView::slotTogglePreviewMode(const CamItemInfo& info)
{
    if (  (d->StackedView->previewMode() == ImportStackedView::PreviewCameraMode
           || d->StackedView->previewMode() == ImportStackedView::MapWidgetMode)
          && !info.isNull() )
    {
        d->lastPreviewMode = d->StackedView->previewMode();

        if (d->StackedView->previewMode() == ImportStackedView::PreviewCameraMode)
        {
            d->StackedView->setPreviewItem(info, d->iconView->previousInfo(info), d->iconView->nextInfo(info));
        }
        else
        {
            d->StackedView->setPreviewItem(info, CamItemInfo(), CamItemInfo());
        }
    }
    else
    {
        // go back to either CameraViewMode or MapWidgetMode
        d->StackedView->setPreviewMode( d->lastPreviewMode );
    }

    // make sure the next/previous buttons are updated
    slotImageSelected();
}

void ImportView::slotViewModeChanged()
{
    toggleZoomActions();

    switch (d->StackedView->previewMode())
    {
        case ImportStackedView::PreviewCameraMode:
            emit signalSwitchedToIconView();
            emit signalThumbSizeChanged(d->iconView->thumbnailSize().size());
            break;
        case ImportStackedView::PreviewImageMode:
            emit signalSwitchedToPreview();
            slotZoomFactorChanged(d->StackedView->zoomFactor());
            break;
        //TODO: case ImportStackedView::WelcomePageMode:
            //emit signalSwitchedToIconView();
            //break;
        case ImportStackedView::MediaPlayerMode:
            emit signalSwitchedToPreview();
            break;
        case ImportStackedView::MapWidgetMode:
            emit signalSwitchedToMapView();
            //TODO: connect map view's zoom buttons to main status bar zoom buttons
            break;
    }
}

void ImportView::slotImageRename()
{
    d->iconView->rename();
}

//FIXME: Remove these functions
//void ImportView::slotImageDelete()
//{
//    d->iconView->deleteSelected(false);
//}

//void ImportView::slotImageDeletePermanently()
//{
//    d->iconView->deleteSelected(true);
//}

//void ImportView::slotImageDeletePermanentlyDirectly()
//{
//    d->iconView->deleteSelectedDirectly(false);
//}

//void ImportView::slotImageTrashDirectly()
//{
//    d->iconView->deleteSelectedDirectly(true);
//}

void ImportView::slotSelectAll()
{
    d->iconView->selectAll();
}

void ImportView::slotSelectNone()
{
    d->iconView->clearSelection();
}

void ImportView::slotSelectInvert()
{
    d->iconView->invertSelection();
}

void ImportView::slotSortImages(int sortRole)
{
    ImportSettings* settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSortOrder(sortRole);
    d->iconView->importFilterModel()->setSortRole((CamItemSortSettings::SortRole) sortRole);
}

void ImportView::slotSortImagesOrder(int order)
{
    ImportSettings* settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSorting(order);
    d->iconView->importFilterModel()->setSortOrder((CamItemSortSettings::SortOrder) order);
}

void ImportView::slotGroupImages(int categoryMode)
{
    ImportSettings* settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageGroupMode(categoryMode);
    d->iconView->importFilterModel()->setCategorizationMode((CamItemSortSettings::CategorizationMode) categoryMode);
}

void ImportView::slotLeftSidebarChangedTab(QWidget* w)
{
    // TODO update, temporary cast
    SidebarWidget* widget = dynamic_cast<SidebarWidget*> (w);
    foreach(SidebarWidget* sideBarWidget, d->leftSideBarWidgets)
    {
        bool active = (widget && (widget == sideBarWidget));
        sideBarWidget->setActive(active);
    }
}

void ImportView::slotSidebarTabTitleStyleChanged()
{
    //d->leftSideBar->setStyle(ImportSettings::instance()->getSidebarTitleStyle());
    //TODO: d->rightSideBar->setStyle(ImportSettings::instance()->getSidebarTitleStyle());

    /// @todo Which settings actually have to be reloaded?
    //     d->rightSideBar->applySettings();
}

void ImportView::toggleShowBar(bool b)
{
    d->StackedView->thumbBarDock()->showThumbBar(b);
}

bool ImportView::isThumbBarVisible()
{
    return d->StackedView->thumbBarDock()->isVisible();
}

void ImportView::scrollTo(const QString& folder, const QString& file)
{
    CamItemInfo info = camItemInfo(folder, file);
    QModelIndex index = d->iconView->importFilterModel()->indexForCamItemInfo(info);
    d->iconView->scrollToRelaxed(index);
    d->iconView->setSelectedCamItemInfos(CamItemInfoList() << info);
}

void ImportView::slotImageChangeFailed(const QString& message, const QStringList& fileNames)
{
    if (fileNames.isEmpty())
    {
        return;
    }

    KMessageBox::errorList(0, message, fileNames);
}

void ImportView::slotLeftSideBarActivate(SidebarWidget* widget)
{
    d->leftSideBar->setActiveTab(widget);
}

void ImportView::slotLeftSideBarActivate(QWidget* widget)
{
    slotLeftSideBarActivate(static_cast<SidebarWidget*>(widget));
}

bool ImportView::hasCurrentItem() const
{
    // We should actually get this directly from the selection model,
    // but the iconView is fine for now.
    return !d->iconView->currentInfo().isNull();
}

//void ImportView::slotImageExifOrientation(int orientation)
//{
//    FileActionMngr::instance()->setExifOrientation(d->iconView->selectedCamItemInfos(), orientation);
//}

//void ImportView::imageTransform(KExiv2Iface::RotationMatrix::TransformationAction transform)
//{
//    FileActionMngr::instance()->transform(d->iconView->selectedCamItemInfos(), transform);
//}

} // namespace Digikam
