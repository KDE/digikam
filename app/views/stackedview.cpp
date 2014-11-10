/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
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

#include "stackedview.moc"

// Qt includes

#include <QFileInfo>
#include <QSplitter>
#include <QTimer>
#include <QWidget>

// KDE includes

#include <kurl.h>
#include <kapplication.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <khtmlview.h>
#include <kglobal.h>
//#include <klinkitemselectionmodel.h>

// Local includes

#include "applicationsettings.h"
#include "digikamimageview.h"
#include "digikamview.h"
#include "imagealbummodel.h"
#include "imagealbumfiltermodel.h"
#include "imagepreviewview.h"
#include "imagethumbnailbar.h"
#include "loadingcacheinterface.h"
#include "previewlayout.h"
#include "welcomepageview.h"
#include "mediaplayerview.h"
#include "thumbbardock.h"
#include "tableview.h"

#ifdef HAVE_KGEOMAP
#include "mapwidgetview.h"
#endif // HAVE_KGEOMAP

namespace Digikam
{

class StackedView::Private
{

public:

    Private()
    {
        dockArea           = 0;
        splitter           = 0;
        thumbBar           = 0;
        thumbBarDock       = 0;
        imageIconView      = 0;
        imagePreviewView   = 0;
        welcomePageView    = 0;
        mediaPlayerView    = 0;
        needUpdateBar      = false;
        syncingSelection   = false;
        tableView          = 0;
#ifdef HAVE_KGEOMAP
        mapWidgetView      = 0;
#endif // HAVE_KGEOMAP
    }

    bool               needUpdateBar;
    bool               syncingSelection;

    QMainWindow*       dockArea;
    QSplitter*         splitter;

    DigikamImageView*  imageIconView;
    ImageThumbnailBar* thumbBar;
    ImagePreviewView*  imagePreviewView;
    MediaPlayerView*   mediaPlayerView;
    ThumbBarDock*      thumbBarDock;
    WelcomePageView*   welcomePageView;
    TableView*         tableView;
#ifdef HAVE_KGEOMAP
    MapWidgetView*     mapWidgetView;
#endif // HAVE_KGEOMAP
};

StackedView::StackedView(QWidget* const parent)
    : QStackedWidget(parent), d(new Private)
{
    d->imageIconView    = new DigikamImageView(this);
    d->imagePreviewView = new ImagePreviewView(this);
    d->thumbBarDock     = new ThumbBarDock();
    d->thumbBar         = new ImageThumbnailBar(d->thumbBarDock);
    d->thumbBar->setModelsFiltered(d->imageIconView->imageModel(), d->imageIconView->imageFilterModel());
    d->thumbBar->installOverlays();
    d->thumbBarDock->setWidget(d->thumbBar);
    d->thumbBarDock->setObjectName("mainwindow_thumbbar");

    d->welcomePageView = new WelcomePageView(this);
    d->mediaPlayerView = new MediaPlayerView(this);
    d->tableView       = new TableView(d->imageIconView->getSelectionModel(),
                                       d->imageIconView->imageFilterModel(),
                                       this);
    d->tableView->setObjectName("mainwindow_tableview");

#ifdef HAVE_KGEOMAP
    d->mapWidgetView   = new MapWidgetView(d->imageIconView->getSelectionModel(),
                                           d->imageIconView->imageFilterModel(), this,
                                           MapWidgetView::ApplicationDigikam
                                          );
    d->mapWidgetView->setObjectName("mainwindow_mapwidgetview");
#endif // HAVE_KGEOMAP

    insertWidget(IconViewMode,     d->imageIconView);
    insertWidget(PreviewImageMode, d->imagePreviewView);
    insertWidget(WelcomePageMode,  d->welcomePageView->view());
    insertWidget(MediaPlayerMode,  d->mediaPlayerView);
    insertWidget(TableViewMode,    d->tableView);

#ifdef HAVE_KGEOMAP
    insertWidget(MapWidgetMode,    d->mapWidgetView);
#endif // HAVE_KGEOMAP

    setViewMode(IconViewMode);
    setAttribute(Qt::WA_DeleteOnClose);

    readSettings();

    // -----------------------------------------------------------------

    connect(d->imagePreviewView, SIGNAL(signalPopupTagsView()),
            d->imageIconView, SIGNAL(signalPopupTagsView()));

    connect(d->imagePreviewView, SIGNAL(signalGotoAlbumAndItem(ImageInfo)),
            this, SIGNAL(signalGotoAlbumAndItem(ImageInfo)));

    connect(d->imagePreviewView, SIGNAL(signalGotoDateAndItem(ImageInfo)),
            this, SIGNAL(signalGotoDateAndItem(ImageInfo)));

    connect(d->imagePreviewView, SIGNAL(signalGotoTagAndItem(int)),
            this, SIGNAL(signalGotoTagAndItem(int)));

    connect(d->imagePreviewView, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->imagePreviewView, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->imagePreviewView, SIGNAL(signalEditItem()),
            this, SIGNAL(signalEditItem()));

    connect(d->imagePreviewView, SIGNAL(signalDeleteItem()),
            this, SIGNAL(signalDeleteItem()));

    connect(d->imagePreviewView, SIGNAL(signalEscapePreview()),
            this, SIGNAL(signalEscapePreview()));

    connect(d->imagePreviewView, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));

    connect(d->imagePreviewView->layout(), SIGNAL(zoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->imagePreviewView, SIGNAL(signalInsert2LightTable()),
            this, SIGNAL(signalInsert2LightTable()));

    connect(d->imagePreviewView, SIGNAL(signalInsert2QueueMgr()),
            this, SIGNAL(signalInsert2QueueMgr()));

    connect(d->imagePreviewView, SIGNAL(signalFindSimilar()),
            this, SIGNAL(signalFindSimilar()));

    connect(d->imagePreviewView, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    connect(d->thumbBar, SIGNAL(selectionChanged()),
            this, SLOT(slotThumbBarSelectionChanged()));

    connect(d->imageIconView, SIGNAL(selectionChanged()),
            this, SLOT(slotIconViewSelectionChanged()));

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->mediaPlayerView, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->mediaPlayerView, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->mediaPlayerView, SIGNAL(signalEscapePreview()),
            this, SIGNAL(signalEscapePreview()));

    connect(d->imagePreviewView, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotPreviewLoaded(bool)));
}

StackedView::~StackedView()
{
    delete d;
}

void StackedView::readSettings()
{
    ApplicationSettings* settings = ApplicationSettings::instance();
    bool showThumbbar       = settings->getShowThumbbar();
    d->thumbBarDock->setShouldBeVisible(showThumbbar);
}

void StackedView::setDockArea(QMainWindow* dockArea)
{
    // Attach the thumbbar dock to the given dock area and place it initially on top.
    d->dockArea = dockArea;
    d->thumbBarDock->setParent(d->dockArea);
    d->dockArea->addDockWidget(Qt::TopDockWidgetArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);
}

ThumbBarDock* StackedView::thumbBarDock() const
{
    return d->thumbBarDock;
}

ImageThumbnailBar* StackedView::thumbBar() const
{
    return d->thumbBar;
}

void StackedView::slotEscapePreview()
{
    if (viewMode() == MediaPlayerMode)
    {
        d->mediaPlayerView->escapePreview();
    }
}

DigikamImageView* StackedView::imageIconView() const
{
    return d->imageIconView;
}

ImagePreviewView* StackedView::imagePreviewView() const
{
    return d->imagePreviewView;
}

#ifdef HAVE_KGEOMAP
MapWidgetView* StackedView::mapWidgetView() const
{
    return d->mapWidgetView;
}
#endif // HAVE_KGEOMAP

TableView* StackedView::tableView() const
{
    return d->tableView;
}

MediaPlayerView* StackedView::mediaPlayerView() const
{
    return d->mediaPlayerView;
}

bool StackedView::isInSingleFileMode() const
{
    return currentIndex() == PreviewImageMode || currentIndex() == MediaPlayerMode;
}

bool StackedView::isInMultipleFileMode() const
{
    return currentIndex() == IconViewMode
        || currentIndex() == MapWidgetMode
        || currentIndex() == TableViewMode;
}

bool StackedView::isInAbstractMode() const
{
    return currentIndex() == WelcomePageMode;
}

void StackedView::setPreviewItem(const ImageInfo& info, const ImageInfo& previous, const ImageInfo& next)
{
    if (info.isNull())
    {
        if (viewMode() == MediaPlayerMode)
        {
            d->mediaPlayerView->setCurrentItem();
        }
        else if (viewMode() == PreviewImageMode)
        {
            d->imagePreviewView->setImageInfo();
        }
    }
    else
    {
        if (info.category() == DatabaseItem::Audio || info.category() == DatabaseItem::Video)
        {
            // Stop image viewer
            if (viewMode() == PreviewImageMode)
            {
                d->imagePreviewView->setImageInfo();
            }

            setViewMode(MediaPlayerMode);
            d->mediaPlayerView->setCurrentItem(info.fileUrl(), !previous.isNull(), !next.isNull());
        }
        else
        {
            // Stop media player if running...
            if (viewMode() == MediaPlayerMode)
            {
                d->mediaPlayerView->setCurrentItem();
            }

            d->imagePreviewView->setImageInfo(info, previous, next);

            // NOTE: No need to toggle immediately in PreviewImageMode here,
            // because we will receive a signal for that when the image preview will be loaded.
            // This will prevent a flicker effect with the old image preview loaded in stack.
        }

        // do not touch the selection, only adjust current info
        QModelIndex currentIndex = d->thumbBar->imageSortFilterModel()->indexForImageInfo(info);
        d->thumbBar->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
    }
}

StackedView::StackedViewMode StackedView::viewMode() const
{
    return StackedViewMode(indexOf(currentWidget()));
}

void StackedView::setViewMode(const StackedViewMode mode)
{
    if ((mode<StackedViewModeFirst)||(mode>StackedViewModeLast))
    {
        return;
    }

    if (mode == PreviewImageMode || mode == MediaPlayerMode)
    {
        d->thumbBarDock->restoreVisibility();
        syncSelection(d->imageIconView, d->thumbBar);
    }
    else
    {
        d->thumbBarDock->hide();
    }

    if (mode == IconViewMode || mode == WelcomePageMode || mode == MapWidgetMode || mode == TableViewMode)
    {
        setPreviewItem();
        setCurrentIndex(mode);
    }
    else
    {
        setCurrentIndex(mode);
    }

#ifdef HAVE_KGEOMAP
    d->mapWidgetView->setActive(mode == MapWidgetMode);
#endif // HAVE_KGEOMAP

    d->tableView->slotSetActive(mode == TableViewMode);

    if (mode == IconViewMode)
    {
        d->imageIconView->setFocus();
    }
#ifdef HAVE_KGEOMAP
    else if (mode == MapWidgetMode)
    {
        d->mapWidgetView->setFocus();
    }
#endif // HAVE_KGEOMAP
    else if (mode == TableViewMode)
    {
        d->tableView->setFocus();
    }

    emit signalViewModeChanged();
}

void StackedView::syncSelection(ImageCategorizedView* from, ImageCategorizedView* to)
{
    ImageSortFilterModel* const fromModel = from->imageSortFilterModel();
    ImageSortFilterModel* const toModel   = to->imageSortFilterModel();
    // set current info
    QModelIndex currentIndex              = toModel->indexForImageInfo(from->currentInfo());
    to->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);

    // sync selection
    QItemSelection selection              = from->selectionModel()->selection();
    QItemSelection newSelection;

    foreach(const QItemSelectionRange& range, selection)
    {
        QModelIndex topLeft     = toModel->indexForImageInfo(fromModel->imageInfo(range.topLeft()));
        QModelIndex bottomRight = toModel->indexForImageInfo(fromModel->imageInfo(range.bottomRight()));
        newSelection.select(topLeft, bottomRight);
    }

    d->syncingSelection = true;
    to->selectionModel()->select(newSelection, QItemSelectionModel::ClearAndSelect);
    d->syncingSelection = false;
}

void StackedView::slotThumbBarSelectionChanged()
{
    if (currentIndex() != PreviewImageMode && currentIndex() != MediaPlayerMode)
    {
        return;
    }

    if (d->syncingSelection)
    {
        return;
    }

    syncSelection(d->thumbBar, d->imageIconView);
}

void StackedView::slotIconViewSelectionChanged()
{
    if (currentIndex() != IconViewMode)
    {
        return;
    }

    if (d->syncingSelection)
    {
        return;
    }

    syncSelection(d->imageIconView, d->thumbBar);
}

void StackedView::previewLoaded()
{
    emit signalViewModeChanged();
}

void StackedView::slotZoomFactorChanged(double z)
{
    if (viewMode() == PreviewImageMode)
    {
        emit signalZoomFactorChanged(z);
    }
}

void StackedView::increaseZoom()
{
    d->imagePreviewView->layout()->increaseZoom();
}

void StackedView::decreaseZoom()
{
    d->imagePreviewView->layout()->decreaseZoom();
}

void StackedView::zoomTo100Percents()
{
    d->imagePreviewView->layout()->setZoomFactor(1.0, QPoint());
}

void StackedView::fitToWindow()
{
    d->imagePreviewView->layout()->fitToWindow();
}

void StackedView::toggleFitToWindowOr100()
{
    d->imagePreviewView->layout()->toggleFitToWindowOr100();
}

bool StackedView::maxZoom()
{
    return d->imagePreviewView->layout()->atMaxZoom();
}

bool StackedView::minZoom()
{
    return d->imagePreviewView->layout()->atMinZoom();
}

void StackedView::setZoomFactor(double z)
{
    // Giving a null anchor means to use the current view center
    d->imagePreviewView->layout()->setZoomFactor(z, QPoint());
}

void StackedView::setZoomFactorSnapped(double z)
{
    d->imagePreviewView->layout()->setZoomFactor(z, QPoint(), SinglePhotoPreviewLayout::SnapZoomFactor);
}

double StackedView::zoomFactor()
{
    return d->imagePreviewView->layout()->zoomFactor();
}

double StackedView::zoomMin()
{
    return d->imagePreviewView->layout()->minZoomFactor();
}

double StackedView::zoomMax()
{
    return d->imagePreviewView->layout()->maxZoomFactor();
}

void StackedView::slotPreviewLoaded(bool)
{
    setViewMode(StackedView::PreviewImageMode);
    previewLoaded();
}

}  // namespace Digikam
