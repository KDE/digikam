/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "stackedview.h"

// Qt includes

#include <QSplitter>
#include <QWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
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
#include "thumbbardock.h"
#include "tableview.h"
#include "trashview.h"
#include "dimg.h"

#ifdef HAVE_MEDIAPLAYER
#include "mediaplayerview.h"
#endif //HAVE_MEDIAPLAYER

#ifdef HAVE_MARBLE
#include "mapwidgetview.h"
#endif // HAVE_MARBLE

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
        needUpdateBar      = false;
        syncingSelection   = false;
        tableView          = 0;
        trashView          = 0;

#ifdef HAVE_MEDIAPLAYER
        mediaPlayerView    = 0;
#endif //HAVE_MEDIAPLAYER

#ifdef HAVE_MARBLE
        mapWidgetView      = 0;
#endif // HAVE_MARBLE
    }

    bool               needUpdateBar;
    bool               syncingSelection;

    QMainWindow*       dockArea;
    QSplitter*         splitter;

    DigikamImageView*  imageIconView;
    ImageThumbnailBar* thumbBar;
    ImagePreviewView*  imagePreviewView;
    ThumbBarDock*      thumbBarDock;
    WelcomePageView*   welcomePageView;
    TableView*         tableView;
    TrashView*         trashView;

#ifdef HAVE_MEDIAPLAYER
    MediaPlayerView*   mediaPlayerView;
#endif //HAVE_MEDIAPLAYER

#ifdef HAVE_MARBLE
    MapWidgetView*     mapWidgetView;
#endif // HAVE_MARBLE
};

StackedView::StackedView(QWidget* const parent)
    : QStackedWidget(parent),
      d(new Private)
{
    d->imageIconView    = new DigikamImageView(this);
    d->imagePreviewView = new ImagePreviewView(this);
    d->thumbBarDock     = new ThumbBarDock();
    d->thumbBar         = new ImageThumbnailBar(d->thumbBarDock);
    d->thumbBar->setModelsFiltered(d->imageIconView->imageModel(), d->imageIconView->imageFilterModel());
    d->thumbBar->installOverlays();
    d->thumbBarDock->setWidget(d->thumbBar);
    d->thumbBarDock->setObjectName(QLatin1String("mainwindow_thumbbar"));
    d->welcomePageView = new WelcomePageView(this);
    d->tableView       = new TableView(d->imageIconView->getSelectionModel(),
                                       d->imageIconView->imageFilterModel(),
                                       this);
    d->tableView->setObjectName(QLatin1String("mainwindow_tableview"));
    d->trashView = new TrashView(this);

#ifdef HAVE_MARBLE
    d->mapWidgetView   = new MapWidgetView(d->imageIconView->getSelectionModel(),
                                           d->imageIconView->imageFilterModel(), this,
                                           MapWidgetView::ApplicationDigikam
                                          );
    d->mapWidgetView->setObjectName(QLatin1String("mainwindow_mapwidgetview"));
#endif // HAVE_MARBLE

#ifdef HAVE_MEDIAPLAYER
    d->mediaPlayerView = new MediaPlayerView(this);
#endif //HAVE_MEDIAPLAYER

    insertWidget(IconViewMode,     d->imageIconView);
    insertWidget(PreviewImageMode, d->imagePreviewView);
    insertWidget(WelcomePageMode,  d->welcomePageView);
    insertWidget(TableViewMode,    d->tableView);
    insertWidget(TrashViewMode,    d->trashView);

#ifdef HAVE_MARBLE
    insertWidget(MapWidgetMode,    d->mapWidgetView);
#endif // HAVE_MARBLE

#ifdef HAVE_MEDIAPLAYER
    insertWidget(MediaPlayerMode,  d->mediaPlayerView);
#endif //HAVE_MEDIAPLAYER

    setViewMode(IconViewMode);
    setAttribute(Qt::WA_DeleteOnClose);

    readSettings();

    // -----------------------------------------------------------------

    connect(d->imagePreviewView, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));

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

    connect(d->imagePreviewView, SIGNAL(signalSlideShowCurrent()),
            this, SIGNAL(signalSlideShowCurrent()));

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

    connect(d->imagePreviewView, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotPreviewLoaded(bool)));

#ifdef HAVE_MEDIAPLAYER
    connect(d->mediaPlayerView, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->mediaPlayerView, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->mediaPlayerView, SIGNAL(signalEscapePreview()),
            this, SIGNAL(signalEscapePreview()));
#endif //HAVE_MEDIAPLAYER
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
#ifdef HAVE_MEDIAPLAYER
    if (viewMode() == MediaPlayerMode)
    {
        d->mediaPlayerView->escapePreview();
    }
#endif //HAVE_MEDIAPLAYER
}

DigikamImageView* StackedView::imageIconView() const
{
    return d->imageIconView;
}

ImagePreviewView* StackedView::imagePreviewView() const
{
    return d->imagePreviewView;
}

#ifdef HAVE_MARBLE
MapWidgetView* StackedView::mapWidgetView() const
{
    return d->mapWidgetView;
}
#endif // HAVE_MARBLE

TableView* StackedView::tableView() const
{
    return d->tableView;
}

TrashView* StackedView::trashView() const
{
    return d->trashView;
}

#ifdef HAVE_MEDIAPLAYER
MediaPlayerView* StackedView::mediaPlayerView() const
{
    return d->mediaPlayerView;
}
#endif //HAVE_MEDIAPLAYER

bool StackedView::isInSingleFileMode() const
{
    return currentIndex() == PreviewImageMode || currentIndex() == MediaPlayerMode;
}

bool StackedView::isInMultipleFileMode() const
{
    return (currentIndex() == IconViewMode  ||
            currentIndex() == MapWidgetMode ||
            currentIndex() == TableViewMode);
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
#ifdef HAVE_MEDIAPLAYER
            d->mediaPlayerView->setCurrentItem();
#endif //HAVE_MEDIAPLAYER
        }
        else if (viewMode() == PreviewImageMode)
        {
            d->imagePreviewView->setImageInfo();
        }
    }
    else
    {
        if (info.category() == DatabaseItem::Audio      ||
            info.category() == DatabaseItem::Video      ||
            DImg::isAnimatedImage(info.fileUrl().toLocalFile())        // Special case for animated image as GIF or NMG
           )
        {
            // Stop image viewer

            if (viewMode() == PreviewImageMode)
            {
                d->imagePreviewView->setImageInfo();
            }

#ifdef HAVE_MEDIAPLAYER
            setViewMode(MediaPlayerMode);
            d->mediaPlayerView->setCurrentItem(info.fileUrl(), !previous.isNull(), !next.isNull());
#endif //HAVE_MEDIAPLAYER
        }
        else // Static image or Raw image.
        {
            // Stop media player if running...

            if (viewMode() == MediaPlayerMode)
            {
#ifdef HAVE_MEDIAPLAYER
                d->mediaPlayerView->setCurrentItem();
#endif //HAVE_MEDIAPLAYER
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
    qCDebug(DIGIKAM_GENERAL_LOG) << "Stacked View Mode : " << mode;

    if ((mode < StackedViewModeFirst) || (mode > StackedViewModeLast))
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

#ifdef HAVE_MARBLE
    d->mapWidgetView->setActive(mode == MapWidgetMode);
#endif // HAVE_MARBLE

    d->tableView->slotSetActive(mode == TableViewMode);

    if (mode == IconViewMode)
    {
        d->imageIconView->setFocus();
    }

#ifdef HAVE_MARBLE
    else if (mode == MapWidgetMode)
    {
        d->mapWidgetView->setFocus();
    }
#endif // HAVE_MARBLE

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
    QModelIndex currentIndex              = toModel->indexForImageInfo(from->currentInfo());

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

    if (currentIndex.isValid())
    {
        // set current info
        to->setCurrentIndex(currentIndex);
    }

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
