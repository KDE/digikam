/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-07
 * Description : QStackedWidget to handle different types of views
 *               (icon view, image preview, media view)
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importstackedview.moc"

// Qt includes

#include <QSplitter>

// KDE includes

#include <kdebug.h>

// Local includes

#include "previewlayout.h"
#include "importsettings.h"

namespace Digikam
{

class MediaPlayerView;

class ImportStackedView::Private
{

public:

    Private()
    {
        dockArea            = 0;
        splitter            = 0;
        thumbBar            = 0;
        thumbBarDock        = 0;
        importIconView      = 0;
        importPreviewView   = 0;
        mediaPlayerView     = 0;
        syncingSelection    = false;
#ifdef HAVE_KGEOMAP
        mapWidgetView       = 0;
#endif // HAVE_KGEOMAP
    }

    bool                syncingSelection;

    QMainWindow*        dockArea;
    QSplitter*          splitter;

    ImportIconView*     importIconView;
    ImportThumbnailBar* thumbBar;
    ImportPreviewView*  importPreviewView;
    ThumbBarDock*       thumbBarDock;
    MediaPlayerView*    mediaPlayerView; // Reuse of albumgui mediaplayer view.

#ifdef HAVE_KGEOMAP
    MapWidgetView*      mapWidgetView;
#endif // HAVE_KGEOMAP
};

ImportStackedView::ImportStackedView(QWidget* const parent)
    : QStackedWidget(parent), d(new Private)
{
    d->importIconView    = new ImportIconView(this);
    d->importPreviewView = new ImportPreviewView(this);
    d->thumbBarDock      = new ThumbBarDock();
    d->thumbBar          = new ImportThumbnailBar(d->thumbBarDock);
    d->thumbBar->setModelsFiltered(d->importIconView->importImageModel(),
                                   d->importIconView->importFilterModel());

    d->thumbBar->installOverlays();
    d->thumbBarDock->setWidget(d->thumbBar);
    d->thumbBarDock->setObjectName("import_thumbbar");

    d->mediaPlayerView   = new MediaPlayerView(this);

    insertWidget(PreviewCameraMode, d->importIconView);
    insertWidget(PreviewImageMode,  d->importPreviewView);
    insertWidget(MediaPlayerMode,   d->mediaPlayerView);

#ifdef HAVE_KGEOMAP
    // TODO refactor MapWidgetView not to require the models on startup?
    d->mapWidgetView     = new MapWidgetView(d->importIconView->getSelectionModel(),
                                             d->importIconView->importFilterModel(), this,
                                             MapWidgetView::ApplicationImportUI);
    d->mapWidgetView->setObjectName("import_mapwidgetview");
    insertWidget(MapWidgetMode,     d->mapWidgetView);
#endif // HAVE_KGEOMAP

    setAttribute(Qt::WA_DeleteOnClose);

    readSettings();

    // -----------------------------------------------------------------

    //FIXME: connect(d->importPreviewView, SIGNAL(signalPopupTagsView()),
            //d->importIconView, SIGNAL(signalPopupTagsView()));

    //connect(d->importPreviewView, SIGNAL(signalGotoFolderAndItem(CamItemInfo)),
            //this, SIGNAL(signalGotoFolderAndItem(CamItemInfo)));

    //connect(d->importPreviewView, SIGNAL(signalGotoDateAndItem(CamItemInfo)),
            //this, SIGNAL(signalGotoDateAndItem(CamItemInfo)));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalGotoTagAndItem(int)),
            //this, SIGNAL(signalGotoTagAndItem(int)));

    connect(d->importPreviewView, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->importPreviewView, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    //connect(d->importPreviewView, SIGNAL(signalEditItem()),
            //this, SIGNAL(signalEditItem()));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalDeleteItem()),
            //this, SIGNAL(signalDeleteItem()));

    connect(d->importPreviewView, SIGNAL(signalEscapePreview()),
            this, SIGNAL(signalEscapePreview()));

    // A workaround to assign pickLabel, colorLabel, and rating in the preview view.
    connect(d->importPreviewView, SIGNAL(signalAssignPickLabel(int)),
            d->importIconView, SLOT(assignPickLabelToSelected(int)));

    connect(d->importPreviewView, SIGNAL(signalAssignColorLabel(int)),
            d->importIconView, SLOT(assignColorLabelToSelected(int)));

    connect(d->importPreviewView, SIGNAL(signalAssignRating(int)),
            d->importIconView, SLOT(assignRatingToSelected(int)));

    connect(d->importPreviewView->layout(), SIGNAL(zoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalInsert2LightTable()),
            //this, SIGNAL(signalInsert2LightTable()));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalInsert2QueueMgr()),
            //this, SIGNAL(signalInsert2QueueMgr()));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalFindSimilar()),
            //this, SIGNAL(signalFindSimilar()));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalAddToExistingQueue(int)),
            //this, SIGNAL(signalAddToExistingQueue(int)));

    connect(d->thumbBar, SIGNAL(selectionChanged()),
            this, SLOT(slotThumbBarSelectionChanged()));

    connect(d->importIconView, SIGNAL(selectionChanged()),
            this, SLOT(slotIconViewSelectionChanged()));

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->mediaPlayerView, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->mediaPlayerView, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->mediaPlayerView, SIGNAL(signalEscapePreview()),
            this, SIGNAL(signalEscapePreview()));

    connect(d->importPreviewView, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotPreviewLoaded(bool)));
}

ImportStackedView::~ImportStackedView()
{
    delete d;
}

void ImportStackedView::readSettings()
{
    ImportSettings* const settings = ImportSettings::instance();
    bool showThumbbar              = settings->getShowThumbbar();
    d->thumbBarDock->setShouldBeVisible(showThumbbar);
}

void ImportStackedView::setDockArea(QMainWindow* dockArea)
{
    // Attach the thumbbar dock to the given dock area and place it initially on top.
    d->dockArea = dockArea;
    d->thumbBarDock->setParent(d->dockArea);
    d->dockArea->addDockWidget(Qt::TopDockWidgetArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);
}

ThumbBarDock* ImportStackedView::thumbBarDock() const
{
    return d->thumbBarDock;
}

ImportThumbnailBar* ImportStackedView::thumbBar() const
{
    return d->thumbBar;
}

void ImportStackedView::slotEscapePreview()
{
    if (viewMode() == MediaPlayerMode)
    {
        d->mediaPlayerView->escapePreview();
    }
}

ImportIconView* ImportStackedView::importIconView() const
{
    return d->importIconView;
}

ImportPreviewView* ImportStackedView::importPreviewView() const
{
    return d->importPreviewView;
}

#ifdef HAVE_KGEOMAP
MapWidgetView* ImportStackedView::mapWidgetView() const
{
    return d->mapWidgetView;
}
#endif // HAVE_KGEOMAP

MediaPlayerView* ImportStackedView::mediaPlayerView() const
{
    return d->mediaPlayerView;
}

bool ImportStackedView::isInSingleFileMode() const
{
    return currentIndex() == PreviewImageMode || currentIndex() == MediaPlayerMode;
}

bool ImportStackedView::isInMultipleFileMode() const
{
    return currentIndex() == PreviewCameraMode || currentIndex() == MapWidgetMode;
}

void ImportStackedView::setPreviewItem(const CamItemInfo& info, const CamItemInfo& previous, const CamItemInfo& next)
{
    if (info.isNull())
    {
        if (viewMode() == MediaPlayerMode)
        {
            d->mediaPlayerView->setCurrentItem();
        }
        else if (viewMode() == PreviewImageMode)
        {
            d->importPreviewView->setCamItemInfo();
        }
    }
    else
    {
        if (identifyCategoryforMime(info.mime) == "audio" || identifyCategoryforMime(info.mime) == "video")
        {
            // Stop image viewer
            if (viewMode() == PreviewImageMode)
            {
                d->importPreviewView->setCamItemInfo();
            }

            setViewMode(MediaPlayerMode);
            d->mediaPlayerView->setCurrentItem(info.url(), !previous.isNull(), !next.isNull());
        }
        else
        {
            // Stop media player if running...
            if (viewMode() == MediaPlayerMode)
            {
                d->mediaPlayerView->setCurrentItem();
            }

            d->importPreviewView->setCamItemInfo(info, previous, next);

            // NOTE: No need to toggle immediately in PreviewImageMode here,
            // because we will receive a signal for that when the image preview will be loaded.
            // This will prevent a flicker effect with the old image preview loaded in stack.
        }

        // do not touch the selection, only adjust current info
        QModelIndex currentIndex = d->thumbBar->importSortFilterModel()->indexForCamItemInfo(info);
        d->thumbBar->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
    }
}

QString ImportStackedView::identifyCategoryforMime(const QString& mime) const
{
    return mime.split('/').at(0);
}

ImportStackedView::StackedViewMode ImportStackedView::viewMode() const
{
    return (StackedViewMode)(indexOf(currentWidget()));
}

void ImportStackedView::setViewMode(const StackedViewMode mode)
{
    if (mode != PreviewCameraMode && mode != PreviewImageMode &&
        mode != MediaPlayerMode   && mode != MapWidgetMode)
    {
        return;
    }

    if (mode == PreviewImageMode || mode == MediaPlayerMode)
    {
        d->thumbBarDock->restoreVisibility();
        syncSelection(d->importIconView, d->thumbBar);
    }
    else
    {
        d->thumbBarDock->hide();
    }

    if (mode == PreviewCameraMode || mode == MapWidgetMode)
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

    if (mode == PreviewCameraMode)
    {
        d->importIconView->setFocus();
    }
#ifdef HAVE_KGEOMAP
    else if (mode == MapWidgetMode)
    {
        d->mapWidgetView->setFocus();
    }
#endif // HAVE_KGEOMAP

    emit signalViewModeChanged();
}

void ImportStackedView::syncSelection(ImportCategorizedView* const from, ImportCategorizedView* const to)
{
    ImportSortFilterModel* const fromModel = from->importSortFilterModel();
    ImportSortFilterModel* const toModel   = to->importSortFilterModel();

    if(!fromModel || !toModel)
    {
        kWarning() << "one or both of the models are null?! from:" << from << "to:" << to;
        return;
    }

    // set current info
    QModelIndex currentIndex         = toModel->indexForCamItemInfo(from->currentInfo());
    to->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);

    // sync selection
    QItemSelection selection         = from->selectionModel()->selection();
    QItemSelection newSelection;

    foreach(const QItemSelectionRange& range, selection)
    {
        QModelIndex topLeft     = toModel->indexForCamItemInfo(fromModel->camItemInfo(range.topLeft()));
        QModelIndex bottomRight = toModel->indexForCamItemInfo(fromModel->camItemInfo(range.bottomRight()));
        newSelection.select(topLeft, bottomRight);
    }

    d->syncingSelection = true;
    to->selectionModel()->select(newSelection, QItemSelectionModel::ClearAndSelect);
    d->syncingSelection = false;
}

void ImportStackedView::slotThumbBarSelectionChanged()
{
    if (currentIndex() != PreviewImageMode && currentIndex() != MediaPlayerMode)
    {
        return;
    }

    if (d->syncingSelection)
    {
        return;
    }

    syncSelection(d->thumbBar, d->importIconView);
}

void ImportStackedView::slotIconViewSelectionChanged()
{
    if (currentIndex() != PreviewCameraMode)
    {
        return;
    }

    if (d->syncingSelection)
    {
        return;
    }

    syncSelection(d->importIconView, d->thumbBar);
}

void ImportStackedView::previewLoaded()
{
    emit signalViewModeChanged();
}

void ImportStackedView::slotZoomFactorChanged(double z)
{
    if (viewMode() == PreviewImageMode)
    {
        emit signalZoomFactorChanged(z);
    }
}

void ImportStackedView::increaseZoom()
{
    d->importPreviewView->layout()->increaseZoom();
}

void ImportStackedView::decreaseZoom()
{
    d->importPreviewView->layout()->decreaseZoom();
}

void ImportStackedView::zoomTo100Percents()
{
    d->importPreviewView->layout()->setZoomFactor(1.0);
}

void ImportStackedView::fitToWindow()
{
    d->importPreviewView->layout()->fitToWindow();
}

void ImportStackedView::toggleFitToWindowOr100()
{
    d->importPreviewView->layout()->toggleFitToWindowOr100();
}

bool ImportStackedView::maxZoom() const
{
    return d->importPreviewView->layout()->atMaxZoom();
}

bool ImportStackedView::minZoom() const
{
    return d->importPreviewView->layout()->atMinZoom();
}

void ImportStackedView::setZoomFactor(double z)
{
    // Giving a null anchor means to use the current view center
    d->importPreviewView->layout()->setZoomFactor(z, QPoint());
}

void ImportStackedView::setZoomFactorSnapped(double z)
{
    d->importPreviewView->layout()->setZoomFactor(z, QPoint(), SinglePhotoPreviewLayout::SnapZoomFactor);
}

double ImportStackedView::zoomFactor() const
{
    return d->importPreviewView->layout()->zoomFactor();
}

double ImportStackedView::zoomMin() const
{
    return d->importPreviewView->layout()->minZoomFactor();
}

double ImportStackedView::zoomMax() const
{
    return d->importPreviewView->layout()->maxZoomFactor();
}

void ImportStackedView::slotPreviewLoaded(bool)
{
    setViewMode(ImportStackedView::PreviewImageMode);
    previewLoaded();
}

}  // namespace Digikam
