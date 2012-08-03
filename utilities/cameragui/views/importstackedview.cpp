/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-07
 * Description : QStackedWidget to handle different types of views
 *               (icon view, image preview, media view)
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

// Local includes

#include "previewlayout.h"
#include "importsettings.h"

namespace Digikam
{

class ImportStackedView::Private
{

public:

    Private()
    {
        dockArea            = 0;
        splitter            = 0;
       //REM thumbBar            = 0;
        //REMthumbBarDock        = 0;
        importIconView      = 0;
        importPreviewView   = 0;
        //mediaPlayerView     = 0;
        //mapWidgetView       = 0;
        syncingSelection    = false;
    }

    bool                syncingSelection;

    QMainWindow*        dockArea;
    QSplitter*          splitter;

    ImportIconView*     importIconView;
    //REMImportThumbnailBar* thumbBar;
    ImportPreviewView*  importPreviewView;
    //REMThumbBarDock*       thumbBarDock;
    //FIXME: MediaPlayerView*   mediaPlayerView;
    //FIXME: MapWidgetView*     mapWidgetView;
};

ImportStackedView::ImportStackedView(CameraController* const controller, QWidget* const parent)
    : QStackedWidget(parent), d(new Private)
{
    d->importIconView    = new ImportIconView(this);
    //d->importPreviewView = new ImportPreviewView(this);
    //d->thumbBarDock      = new ThumbBarDock();
    //d->thumbBar          = new ImportThumbnailBar(d->thumbBarDock);
    //d->thumbBar->setModelsFiltered(d->importIconView->importImageModel(), d->importIconView->importFilterModel());
    //FIXME: d->thumbBar->installRatingOverlay();

    //d->thumbBarDock->setWidget(d->thumbBar);
    //d->thumbBarDock->setObjectName("import_thumbbar");

    if(controller)
    {
        d->importIconView->init(controller);
    }

    //FIXME: d->mediaPlayerView = new MediaPlayerView(this);
    //FIXME: d->mapWidgetView   = new MapWidgetView(d->importIconView->getSelectionModel(),
                                           //d->importIconView->imageFilterModel(), this);
    //FIXME: d->mapWidgetView->setObjectName("import_mapwidgetview");

    insertWidget(PreviewCameraMode, d->importIconView);
    //REMinsertWidget(PreviewImageMode, d->importPreviewView);
    //insertWidget(MediaPlayerMode,  d->mediaPlayerView);
    //insertWidget(MapWidgetMode,    d->mapWidgetView);

    setPreviewMode(PreviewCameraMode);
    setAttribute(Qt::WA_DeleteOnClose);

    readSettings();

    // -----------------------------------------------------------------

    //FIXME: connect(d->importPreviewView, SIGNAL(signalPopupTagsView()),
            //d->importIconView, SIGNAL(signalPopupTagsView()));

    //REMconnect(d->importPreviewView, SIGNAL(signalGotoFolderAndItem(CamItemInfo)),
            //REMthis, SIGNAL(signalGotoFolderAndItem(CamItemInfo)));

    //REMconnect(d->importPreviewView, SIGNAL(signalGotoDateAndItem(CamItemInfo)),
            //REMthis, SIGNAL(signalGotoDateAndItem(CamItemInfo)));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalGotoTagAndItem(int)),
            //this, SIGNAL(signalGotoTagAndItem(int)));

    //REMconnect(d->importPreviewView, SIGNAL(signalNextItem()),
            //REMthis, SIGNAL(signalNextItem()));

    //REMconnect(d->importPreviewView, SIGNAL(signalPrevItem()),
            //REMthis, SIGNAL(signalPrevItem()));

   //REM connect(d->importPreviewView, SIGNAL(signalEditItem()),
          //REM  this, SIGNAL(signalEditItem()));

    //REMconnect(d->importPreviewView, SIGNAL(signalDeleteItem()),
         //REM   this, SIGNAL(signalDeleteItem()));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalBack2FilesList()),
            //this, SIGNAL(signalBack2Album()));

    //REMconnect(d->importPreviewView->layout(), SIGNAL(zoomFactorChanged(double)),
            //REMthis, SLOT(slotZoomFactorChanged(double)));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalInsert2LightTable()),
            //this, SIGNAL(signalInsert2LightTable()));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalInsert2QueueMgr()),
            //this, SIGNAL(signalInsert2QueueMgr()));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalFindSimilar()),
            //this, SIGNAL(signalFindSimilar()));

    //FIXME: connect(d->importPreviewView, SIGNAL(signalAddToExistingQueue(int)),
            //this, SIGNAL(signalAddToExistingQueue(int)));

    //REMconnect(d->thumbBar, SIGNAL(selectionChanged()),
           //REM this, SLOT(slotThumbBarSelectionChanged()));

    connect(d->importIconView, SIGNAL(selectionChanged()),
            this, SLOT(slotIconViewSelectionChanged()));

    //REMconnect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            //REMd->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    //FIXME: connect(d->mediaPlayerView, SIGNAL(signalNextItem()),
            //this, SIGNAL(signalNextItem()));

    //FIXME: connect(d->mediaPlayerView, SIGNAL(signalPrevItem()),
            //this, SIGNAL(signalPrevItem()));

    //FIXME: connect(d->mediaPlayerView, SIGNAL(signalBack2Album()),
            //this, SIGNAL(signalBack2Album()));

   //REM connect(d->importPreviewView, SIGNAL(signalPreviewLoaded(bool)),
            //REMthis, SLOT(slotPreviewLoaded(bool)));
}
ImportStackedView::~ImportStackedView()
{
    delete d;
}

void ImportStackedView::readSettings()
{
/*
    ImportSettings* settings = ImportSettings::instance();
    REMbool showThumbbar        = settings->getShowThumbbar();
    REMd->thumbBarDock->setShouldBeVisible(showThumbbar);
*/
}

void ImportStackedView::setDockArea(QMainWindow* dockArea)
{
    // Attach the thumbbar dock to the given dock area and place it initially on top.
    d->dockArea = dockArea;
    //REMd->thumbBarDock->setParent(d->dockArea);
    //REMd->dockArea->addDockWidget(Qt::TopDockWidgetArea, d->thumbBarDock);
    //REMd->thumbBarDock->setFloating(false);
}

//REMThumbBarDock* ImportStackedView::thumbBarDock() const
//REM{
//REM    return d->thumbBarDock;
//REM}

//REMImportThumbnailBar* ImportStackedView::thumbBar() const
//REM{
//REM    return d->thumbBar;
//REM}

void ImportStackedView::slotEscapePreview()
{
//FIXME: Uncomment when MediaPlayerView is implemented
//    if (previewMode() == MediaPlayerMode)
//    {
//        d->mediaPlayerView->escapePreview();
//    }
}

ImportIconView* ImportStackedView::importIconView() const
{
    return d->importIconView;
}

ImportPreviewView* ImportStackedView::importPreviewView() const
{
    return d->importPreviewView;
}

//MapWidgetView* ImportStackedView::mapWidgetView() const
//{
//    return d->mapWidgetView;
//}

//FIXME: Uncomment when MediaPlayerView is implemented
//MediaPlayerView* ImportStackedView::mediaPlayerView() const
//{
//    return d->mediaPlayerView;
//}

bool ImportStackedView::isInSingleFileMode() const
{
    return currentIndex() == PreviewImageMode || currentIndex() == MediaPlayerMode;
}

bool ImportStackedView::isInMultipleFileMode() const
{
    return currentIndex() == PreviewCameraMode || currentIndex() == MapWidgetMode;
}

void ImportStackedView::setPreviewItem(const CamItemInfo& info, const CamItemInfo& /*previous*/, const CamItemInfo& /*next*/)
{
    if (info.isNull())
    {
        //FIXME: Uncomment when MediaPlayerView is implemented
        //if (previewMode() == MediaPlayerMode)
        //{
        //    d->mediaPlayerView->setCamItemInfo();
        //}
        /*else*/ if (previewMode() == PreviewImageMode)
        {
           //REM d->importPreviewView->setCamItemInfo();
        }
    }
    else
    {
        if (identifyCategoryforMime(info.mime) == "audio" || identifyCategoryforMime(info.mime) == "video")
        {
            // Stop image viewer
            if (previewMode() == PreviewImageMode)
            {
                //REMd->importPreviewView->setCamItemInfo();
            }

            //FIXME: Uncomment when MediaPlayerView is implemented
            //setPreviewMode(MediaPlayerMode);
            //d->mediaPlayerView->setCamItemInfo(info, previous, next);
        }
        else
        {
            // Stop media player if running...
            //FIXME: Uncomment when MediaPlayerView is implemented
            //if (previewMode() == MediaPlayerMode)
            //{
            //    d->mediaPlayerView->setCamItemInfo();
            //}

            //REMd->importPreviewView->setCamItemInfo(info, previous, next);

            // NOTE: No need to toggle immediately in PreviewImageMode here,
            // because we will receive a signal for that when the image preview will be loaded.
            // This will prevent a flicker effect with the old image preview loaded in stack.
        }

        // do not touch the selection, only adjust current info
        //REMQModelIndex currentIndex = d->thumbBar->importSortFilterModel()->indexForCamItemInfo(info);
        //REMd->thumbBar->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
    }
}

QString ImportStackedView::identifyCategoryforMime(const QString& mime) const
{
    return mime.split("/").at(0);
}

int ImportStackedView::previewMode()
{
    return indexOf(currentWidget());
}

void ImportStackedView::setPreviewMode(const int mode)
{
    if (mode != PreviewCameraMode && mode != PreviewImageMode &&
        mode != MediaPlayerMode && mode != MapWidgetMode)
    {
        return;
    }

    if (mode == PreviewImageMode || mode == MediaPlayerMode)
    {
        //REMd->thumbBarDock->restoreVisibility();
        //REMsyncSelection(d->importIconView, d->thumbBar);
    }
    else
    {
        //REMd->thumbBarDock->hide();
    }

    //TODO: Implement the MapPageMode
    //if (mode == PreviewCameraMode || mode == WelcomePageMode || mode == MapWidgetMode)
    //{
    //    setPreviewItem();
    //    setCurrentIndex(mode);
    //}
    //else
    //{
    //    setCurrentIndex(mode);
    //}

    //d->mapWidgetView->setActive(mode == MapWidgetMode);

    //if (mode == PreviewCameraMode)
    //{
    //    d->importIconView->setFocus();
    //}
    //else if (mode == MapWidgetMode)
    //{
    //    d->mapWidgetView->setFocus();
    //}

    emit signalViewModeChanged();
}

void ImportStackedView::syncSelection(ImportCategorizedView* const from, ImportCategorizedView* const to)
{
    ImportSortFilterModel* fromModel = from->importSortFilterModel();
    ImportSortFilterModel* toModel   = to->importSortFilterModel();
    // set current info
    QModelIndex currentIndex         = toModel->indexForCamItemInfo(from->currentInfo());
    to->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);

    // sync selection
    QItemSelection selection         = from->selectionModel()->selection();
    QItemSelection newSelection;

    foreach(const QItemSelectionRange& range, selection)
    {
        QModelIndex topLeft = toModel->indexForCamItemInfo(fromModel->camItemInfo(range.topLeft()));
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

    //REMsyncSelection(d->thumbBar, d->importIconView);
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

    //REMsyncSelection(d->importIconView, d->thumbBar);
}

void ImportStackedView::previewLoaded()
{
    emit signalViewModeChanged();
}

void ImportStackedView::slotZoomFactorChanged(double z)
{
    if (previewMode() == PreviewImageMode)
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

bool ImportStackedView::maxZoom()
{
    return d->importPreviewView->layout()->atMaxZoom();
}

bool ImportStackedView::minZoom()
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

double ImportStackedView::zoomFactor()
{
    return d->importPreviewView->layout()->zoomFactor();
}

double ImportStackedView::zoomMin()
{
    return d->importPreviewView->layout()->minZoomFactor();
}

double ImportStackedView::zoomMax()
{
    return d->importPreviewView->layout()->maxZoomFactor();
}

void ImportStackedView::slotPreviewLoaded(bool)
{
    setPreviewMode(ImportStackedView::PreviewImageMode);
    previewLoaded();
}

}  // namespace Digikam
