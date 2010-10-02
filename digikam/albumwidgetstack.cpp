/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumwidgetstack.moc"

// Qt includes

#include <QFileInfo>
#include <QSplitter>
#include <QTimer>
#include <QWidget>

// KDE includes

#include <kurl.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <khtmlview.h>
#include <kglobal.h>
#include <kapplication.h>

// Local includes

#include "albumsettings.h"
#include "digikamimageview.h"
#include "digikamview.h"
#include "imagealbummodel.h"
#include "imagealbumfiltermodel.h"
#include "imagepreviewview.h"
#include "imagethumbnailbar.h"
#include "loadingcacheinterface.h"
#include "welcomepageview.h"
#include "mediaplayerview.h"
#include "thumbbardock.h"
#include "navigationbar.h"

namespace Digikam
{

class AlbumWidgetStackPriv
{

public:

    AlbumWidgetStackPriv()
    {
        dockArea           = 0;
        splitter           = 0;
        thumbBar           = 0;
        thumbBarDock       = 0;
        imageIconView      = 0;
        imagePreviewView   = 0;
        welcomePageView    = 0;
        mediaPlayerView    = 0;
        thumbbarTimer      = 0;
        needUpdateBar      = false;
        navigationBar      = 0;
        navigationBarDock  = 0;
    }

    bool              needUpdateBar;

    QMainWindow*      dockArea;
    QSplitter*        splitter;
    QTimer*           thumbbarTimer;

    DigikamImageView* imageIconView;
    ImageThumbnailBar*thumbBar;
    ImagePreviewView* imagePreviewView;
    MediaPlayerView*  mediaPlayerView;
    ThumbBarDock*     thumbBarDock;
    WelcomePageView*  welcomePageView;
    NavigationBar*    navigationBar;
    QDockWidget*      navigationBarDock;
};

AlbumWidgetStack::AlbumWidgetStack(QWidget *parent)
                : QStackedWidget(parent), d(new AlbumWidgetStackPriv)
{
    /**
     * Views modes
     */
    d->imageIconView    = new DigikamImageView(this);
    d->imagePreviewView = new ImagePreviewView(this);
    d->welcomePageView  = new WelcomePageView(this);
    d->mediaPlayerView  = new MediaPlayerView(this);

    /**
     * Thumbnails dock bar
     */
    d->thumbBarDock     = new ThumbBarDock();
    d->thumbBar         = new ImageThumbnailBar(d->thumbBarDock);
    d->thumbBar->setModels(d->imageIconView->imageModel(), d->imageIconView->imageFilterModel());
    d->thumbBarDock->setWidget(d->thumbBar);
    d->thumbBarDock->setObjectName("mainwindow_thumbbar");

    /**
     * Navigation & Actions dock bar
     */
    d->navigationBar    = new NavigationBar(this);
    d->navigationBarDock = new QDockWidget();
    d->navigationBarDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    d->navigationBarDock->setWidget(d->navigationBar);

    // Connect navigation & actions signals
    connect(d->navigationBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));

    connect(d->navigationBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->navigationBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));

    connect(d->navigationBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->navigationBar, SIGNAL(signalPreviewRequestedCurrent()),
            this, SIGNAL(signalPreviewRequestedCurrent()));

    connect(d->navigationBar, SIGNAL(signalBack2Album()),
            this, SIGNAL(signalBack2Album()));

    connect(d->navigationBar, SIGNAL(signalDeleteItem()),
            this, SIGNAL(signalDeleteItem()));

    connect(d->navigationBar, SIGNAL(signalImageSelected(const ImageInfo&)),
            this, SIGNAL(signalImageSelected(const ImageInfo&)));

    connect(d->navigationBar, SIGNAL(signalRotateLeft()),
            d->imageIconView, SLOT(slotRotateLeft()));

    connect(d->navigationBar, SIGNAL(signalRotateRight()),
            d->imageIconView, SLOT(slotRotateRight()));

    connect(d->navigationBar, SIGNAL(signalEditItem()),
            this, SIGNAL(signalEditItem()));

    connect(d->navigationBar, SIGNAL(signalSlideShowAll()),
            this, SIGNAL(signalSlideShowAll()));

    connect(d->navigationBar, SIGNAL(signalSlideShowSelection()),
            this, SIGNAL(signalSlideShowSelection()));

    connect(d->navigationBar, SIGNAL(signalSlideShowRecursive()),
            this, SIGNAL(signalSlideShowRecursive()));

    connect(this, SIGNAL(signalImageSelected(const ImageInfoList&, bool, bool, const ImageInfoList&)),
            d->navigationBar, SLOT(slotImageSelected(const ImageInfoList&, bool, bool, const ImageInfoList&)));

    connect(this, SIGNAL(signalSelectionChanged(int)),
            d->navigationBar, SLOT(slotSelectionChanged(int)));

    // Connect filter signals
    ImageAlbumFilterModel *model = d->imageIconView->imageAlbumFilterModel();

    connect(d->navigationBar, SIGNAL(signalRatingFilter(int, ImageFilterSettings::RatingCondition)),
            model, SLOT(setRatingFilter(int, ImageFilterSettings::RatingCondition)));

    connect(d->navigationBar, SIGNAL(signalMimeTypeFilter(int)),
            model, SLOT(setMimeTypeFilter(int)));

    connect(d->navigationBar, SIGNAL(signalTextFilter(const SearchTextSettings&)),
            model, SLOT(setTextFilter(const SearchTextSettings&)));

    connect(model, SIGNAL(filterMatches(bool)),
            d->navigationBar, SIGNAL(signalFilterMatches(bool)));

    connect(model, SIGNAL(filterMatchesForText(bool)),
            d->navigationBar, SIGNAL(signalFilterMatchesForText(bool)));

    connect(model, SIGNAL(filterSettingsChanged(const ImageFilterSettings&)),
            d->navigationBar, SIGNAL(signaltFilterSettingsChanged(const ImageFilterSettings&)));

    // Connect zoom signals
    connect(d->navigationBar, SIGNAL(signalZoomSliderChanged(int)),
            this, SIGNAL(signalZoomSliderChanged(int)));

    connect(d->navigationBar, SIGNAL(signalZoomValueEdited(double)),
            this, SIGNAL(signalZoomValueEdited(double)));

    connect(d->navigationBar, SIGNAL(signalZoomIn()),
            this, SIGNAL(signalZoomIn()));

    connect(d->navigationBar, SIGNAL(signalZoomOut()),
            this, SIGNAL(signalZoomOut()));

    connect(d->navigationBar, SIGNAL(signalZoomTo100Percents()),
            this, SIGNAL(signalZoomTo100Percents()));

    connect(d->navigationBar, SIGNAL(signalFitToWindow()),
            this, SIGNAL(signalFitToWindow()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->navigationBar, SIGNAL(signalWindowHasMoved()));

    connect(this, SIGNAL(signalThumbSizeChanged(int)),
            d->navigationBar, SLOT(slotThumbSizeChanged(int)));

    connect(this, SIGNAL(signalZoomChanged(double, double, double)),
            d->navigationBar, SLOT(slotZoomChanged(double, double, double)));

    // Conect views modes signals
    connect(this, SIGNAL(signalToggledToPreviewMode(bool)),
            d->navigationBar, SLOT(slotToggledToPreviewMode(bool)));

    // -----------------------------------------------------------------

    insertWidget(PreviewAlbumMode, d->imageIconView);
    insertWidget(PreviewImageMode, d->imagePreviewView);
    insertWidget(WelcomePageMode,  d->welcomePageView->view());
    insertWidget(MediaPlayerMode,  d->mediaPlayerView);

    setPreviewMode(PreviewAlbumMode);
    setAttribute(Qt::WA_DeleteOnClose);

    d->thumbbarTimer = new QTimer(this);
    d->thumbbarTimer->setSingleShot(true);

    readSettings();

    // -----------------------------------------------------------------

    connect(d->imagePreviewView, SIGNAL(signalGotoAlbumAndItem(const ImageInfo&)),
            this, SIGNAL(signalGotoAlbumAndItem(const ImageInfo&)));

    connect(d->imagePreviewView, SIGNAL(signalGotoDateAndItem(const ImageInfo&)),
            this, SIGNAL(signalGotoDateAndItem(const ImageInfo&)));

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

    connect(d->imagePreviewView, SIGNAL(signalBack2Album()),
            this, SIGNAL(signalBack2Album()));

    connect(d->imagePreviewView, SIGNAL(signalSlideShow()),
            this, SIGNAL(signalSlideShow()));

    connect(d->imagePreviewView, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->imagePreviewView, SIGNAL(signalInsert2LightTable()),
            this, SIGNAL(signalInsert2LightTable()));

    connect(d->imagePreviewView, SIGNAL(signalInsert2QueueMgr()),
            this, SIGNAL(signalInsert2QueueMgr()));

    connect(d->imagePreviewView, SIGNAL(signalFindSimilar()),
            this, SIGNAL(signalFindSimilar()));

    connect(d->imagePreviewView, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    /*
    connect(d->imageIconView->imageFilterModel(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(slotItemsAddedOrRemoved()));

    connect(d->imageIconView->imageFilterModel(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(slotItemsAddedOrRemoved()));

    connect(d->imageIconView->imageFilterModel(), SIGNAL(layoutChanged()),
            this, SLOT(slotItemsAddedOrRemoved()));

    connect(d->imageIconView->imageFilterModel(), SIGNAL(modelReset()),
            this, SLOT(slotItemsAddedOrRemoved()));

    connect(d->thumbbarTimer, SIGNAL(timeout()),
            this, SLOT(updateThumbbar()));
    */

    connect(d->thumbBar, SIGNAL(imageActivated(const ImageInfo&)),
            this, SIGNAL(signalImageSelected(const ImageInfo&)));

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->mediaPlayerView, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->mediaPlayerView, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->mediaPlayerView, SIGNAL(signalBack2Album()),
            this, SIGNAL(signalBack2Album()));

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(const QString&)));
}

AlbumWidgetStack::~AlbumWidgetStack()
{
    delete d;
}

void AlbumWidgetStack::readSettings()
{
    AlbumSettings *settings = AlbumSettings::instance();
    bool showThumbbar       = settings->getShowThumbbar();
    d->thumbBarDock->setShouldBeVisible(showThumbbar);
}

void AlbumWidgetStack::setDockArea(QMainWindow* dockArea)
{
    d->dockArea = dockArea;

    // Attach the thumbbar dock to the given dock area and place it initially on top.
    d->thumbBarDock->setParent(d->dockArea);
    d->dockArea->addDockWidget(Qt::TopDockWidgetArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);

    // Attach the NavigationBar dock to the given dock area and place it initially on bottom.
    d->navigationBarDock->setParent(d->dockArea);
    d->dockArea->addDockWidget(Qt::BottomDockWidgetArea, d->navigationBarDock);
    d->navigationBarDock->setFloating(false);
}

ThumbBarDock* AlbumWidgetStack::thumbBarDock()
{
    return d->thumbBarDock;
}

void AlbumWidgetStack::slotEscapePreview()
{
    if (previewMode() == MediaPlayerMode)
        d->mediaPlayerView->escapePreview();
}

DigikamImageView* AlbumWidgetStack::imageIconView()
{
    return d->imageIconView;
}

ImagePreviewView* AlbumWidgetStack::imagePreviewView()
{
    return d->imagePreviewView;
}

void AlbumWidgetStack::setPreviewItem(const ImageInfo& info, const ImageInfo& previous, const ImageInfo& next)
{
    if (info.isNull())
    {
        if (previewMode() == MediaPlayerMode)
        {
            d->mediaPlayerView->setImageInfo();
        }
        else if (previewMode() == PreviewImageMode)
        {
            d->imagePreviewView->setImageInfo();
        }

        /*
        // Special case to cleanup thumbbar if Image Lister do not query item accordingly to
        // IconView Filters.
        if (d->imageIconView->imageModel()->isEmpty())
            d->thumbBar->clear();
        */
    }
    else
    {
        AlbumSettings *settings      = AlbumSettings::instance();
        QString currentFileExtension = QFileInfo(info.fileUrl().toLocalFile()).suffix();
        QString mediaplayerfilter    = settings->getMovieFileFilter().toLower() +
                                       settings->getMovieFileFilter().toUpper() +
                                       settings->getAudioFileFilter().toLower() +
                                       settings->getAudioFileFilter().toUpper();
        if (mediaplayerfilter.contains(currentFileExtension) )
        {
            setPreviewMode(MediaPlayerMode);
            d->mediaPlayerView->setImageInfo(info, previous, next);
        }
        else
        {
            // Stop media player if running...
            if (previewMode() == MediaPlayerMode)
                setPreviewItem();

            /*
            if (previewMode() != PreviewImageMode)
                updateThumbbar();
            */

            d->imagePreviewView->setImageInfo(info, previous, next);

            // NOTE: No need to toggle immediately in PreviewImageMode here,
            // because we will receive a signal for that when the image preview will be loaded.
            // This will prevent a flicker effect with the old image preview loaded in stack.
        }

        d->thumbBar->setCurrentInfo(info);
        emit signalPreviewChanged(info);
    }
}

int AlbumWidgetStack::previewMode()
{
    return indexOf(currentWidget());
}

void AlbumWidgetStack::setPreviewMode(int mode)
{
    if (mode != PreviewAlbumMode && mode != PreviewImageMode &&
        mode != WelcomePageMode  && mode != MediaPlayerMode)
        return;

    if (mode == PreviewImageMode)
    {
        d->thumbBarDock->restoreVisibility();
    }
    else
    {
        d->thumbBarDock->hide();
    }

    if (mode == PreviewAlbumMode || mode == WelcomePageMode)
    {
        setPreviewItem();
        setCurrentIndex(mode);
        emit signalToggledToPreviewMode(false);
    }
    else
    {
        setCurrentIndex(mode);
    }
    d->imageIconView->setFocus();
}

void AlbumWidgetStack::previewLoaded()
{
     emit signalToggledToPreviewMode(true);
}

void AlbumWidgetStack::slotZoomFactorChanged(double z)
{
    if (previewMode() == PreviewImageMode)
        emit signalZoomFactorChanged(z);
}

void AlbumWidgetStack::slotFileChanged(const QString &path)
{
    // If item are updated from Icon View, and if we are in Preview Mode,
    // We will check if the current item preview need to be reloaded.

    if (previewMode() == PreviewAlbumMode ||
        previewMode() == WelcomePageMode  ||
        previewMode() == MediaPlayerMode)    // What we can do with media player ?
        return;

    if (path == imagePreviewView()->getImageInfo().filePath())
        d->imagePreviewView->reload();
}

/*
void AlbumWidgetStack::slotItemsAddedOrRemoved()
{
    // do this before the check in the next line, to store this state in any case,
    // even if we do not trigger updateThumbbar immediately
    d->needUpdateBar = true;

    if (previewMode() != PreviewImageMode)
        return;

    d->thumbbarTimer->start(50);

    KUrl currentUrl = d->imageIconView->currentUrl();
    if (currentUrl != KUrl())
    {
        ThumbBarItem* item = d->thumbBar->findItemByUrl(currentUrl);
        d->thumbBar->setSelected(item);
    }
}

void AlbumWidgetStack::updateThumbbar()
{
    if (!d->needUpdateBar)
        return;
    d->needUpdateBar = false;

    d->thumbBarDock->reInitialize();
    d->thumbBar->clear();

    ImageInfoList list = d->imageIconView->imageInfos();

    d->thumbBar->blockSignals(true);
    for (ImageInfoList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
            new ImagePreviewBarItem(d->thumbBar, *it);
    d->thumbBar->blockSignals(false);

    d->thumbBar->slotUpdate();
}
*/

void AlbumWidgetStack::increaseZoom()
{
    d->imagePreviewView->slotIncreaseZoom();
}

void AlbumWidgetStack::decreaseZoom()
{
    d->imagePreviewView->slotDecreaseZoom();
}

void AlbumWidgetStack::zoomTo100Percents()
{
    d->imagePreviewView->setZoomFactor(1.0);
}

void AlbumWidgetStack::fitToWindow()
{
    d->imagePreviewView->fitToWindow();
}

void AlbumWidgetStack::toggleFitToWindowOr100()
{
    d->imagePreviewView->toggleFitToWindowOr100();
}

bool AlbumWidgetStack::maxZoom()
{
    return d->imagePreviewView->maxZoom();
}

bool AlbumWidgetStack::minZoom()
{
    return d->imagePreviewView->minZoom();
}

void AlbumWidgetStack::setZoomFactor(double z)
{
    d->imagePreviewView->setZoomFactor(z);
}

void AlbumWidgetStack::setZoomFactorSnapped(double z)
{
    d->imagePreviewView->setZoomFactorSnapped(z);
}

double AlbumWidgetStack::zoomFactor()
{
    return d->imagePreviewView->zoomFactor();
}

double AlbumWidgetStack::zoomMin()
{
    return d->imagePreviewView->zoomMin();
}

double AlbumWidgetStack::zoomMax()
{
    return d->imagePreviewView->zoomMax();
}

void AlbumWidgetStack::slotZoomChanged(double zoom)
{
    emit signalZoomChanged(zoom, zoomMin(), zoomMax());
}

void AlbumWidgetStack::applySettings()
{
    AlbumSettings *settings = AlbumSettings::instance();
    d->imagePreviewView->setLoadFullImageSize(settings->getPreviewLoadFullImageSize());
}

}  // namespace Digikam
