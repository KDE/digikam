/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumwidgetstack.h"
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
#include "imagepreviewbar.h"
#include "loadingcacheinterface.h"
#include "welcomepageview.h"
#include "mediaplayerview.h"
#include "thumbbardock.h"

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
    }

    QMainWindow      *dockArea;

    QSplitter        *splitter;

    ImagePreviewBar  *thumbBar;
    ThumbBarDock     *thumbBarDock;

    DigikamImageView *imageIconView;

    ImagePreviewView *imagePreviewView;

    WelcomePageView  *welcomePageView;

    MediaPlayerView  *mediaPlayerView;

    QTimer           *thumbbarTimer;

    bool              needUpdateBar;
};

AlbumWidgetStack::AlbumWidgetStack(QWidget *parent)
                : QStackedWidget(parent), d(new AlbumWidgetStackPriv)
{
    d->imageIconView    = new DigikamImageView(this);
    d->imagePreviewView = new ImagePreviewView(this, this);
    d->thumbBarDock     = new ThumbBarDock();
    d->thumbBar         = new ImagePreviewBar(d->thumbBarDock, Qt::Horizontal,
                                              AlbumSettings::instance()->getExifRotate());
    d->thumbBarDock->setWidget(d->thumbBar);
    d->thumbBarDock->setObjectName("mainwindow_thumbbar");

    // To prevent flicker effect with content when user change icon view filter
    // if scrollbar appears or disapears.
    d->thumbBar->setHScrollBarMode(Q3ScrollView::AlwaysOn);

    d->welcomePageView = new WelcomePageView(this);
    d->mediaPlayerView = new MediaPlayerView(this);

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

    connect(d->imageIconView->imageFilterModel(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(slotItemsAddedOrRemoved()));

    connect(d->imageIconView->imageFilterModel(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(slotItemsAddedOrRemoved()));

    connect(d->imageIconView->imageFilterModel(), SIGNAL(layoutChanged()),
            this, SLOT(slotItemsAddedOrRemoved()));

    connect(d->imageIconView->imageFilterModel(), SIGNAL(modelReset()),
            this, SLOT(slotItemsAddedOrRemoved()));

    connect(d->thumbBar, SIGNAL(signalUrlSelected(const KUrl&)),
            this, SIGNAL(signalUrlSelected(const KUrl&)));

    connect(d->thumbbarTimer, SIGNAL(timeout()),
            this, SLOT(updateThumbbar()));

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(const QString &)));
}

AlbumWidgetStack::~AlbumWidgetStack()
{
    delete d;
}

void AlbumWidgetStack::readSettings()
{
    AlbumSettings *settings = AlbumSettings::instance();
    bool showThumbbar = settings->getShowThumbbar();
    d->thumbBarDock->setShouldBeVisible(showThumbbar);
}

void AlbumWidgetStack::setDockArea(QMainWindow *dockArea)
{
    // Attach the thumbbar dock to the given dock area and place it initially
    // on top.
    d->dockArea = dockArea;
    d->thumbBarDock->setParent(d->dockArea);
    d->dockArea->addDockWidget(Qt::TopDockWidgetArea, d->thumbBarDock);
}

ThumbBarDock *AlbumWidgetStack::thumbBarDock()
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

void AlbumWidgetStack::setPreviewItem(const ImageInfo & info, const ImageInfo& previous, const ImageInfo& next)
{
    if (info.isNull())
    {
        if (previewMode() == MediaPlayerMode)
        {
            d->mediaPlayerView->setMediaPlayerFromUrl(KUrl());
        }
        else if (previewMode() == PreviewImageMode)
        {
            d->imagePreviewView->setImageInfo();
        }

        // Special case to cleanup thumbbar if Image Lister do not query item accordingly to
        // IconView Filters.
        if (d->imageIconView->imageModel()->isEmpty())
            d->thumbBar->clear();
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
            d->mediaPlayerView->setMediaPlayerFromUrl(info.fileUrl());
        }
        else
        {
            // Stop media player if running...
            if (previewMode() == MediaPlayerMode)
                setPreviewItem();

            if (previewMode() != PreviewImageMode)
                updateThumbbar();

            d->imagePreviewView->setImageInfo(info, previous, next);

            // NOTE: No need to toggle immediately in PreviewImageMode here,
            // because we will receive a signal for that when the image preview will be loaded.
            // This will prevent a flicker effect with the old image preview loaded in stack.
        }

        ThumbBarItem* item = d->thumbBar->findItemByUrl(info.fileUrl());
        d->thumbBar->setSelected(item);
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

    KUrl url = KUrl::fromPath(path);
    ThumbBarItem* foundItem = d->thumbBar->findItemByUrl(url);
    if (foundItem)
        d->thumbBar->reloadThumb(foundItem);
}

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

void AlbumWidgetStack::applySettings()
{
    AlbumSettings *settings = AlbumSettings::instance();
    d->imagePreviewView->setLoadFullImageSize(settings->getPreviewLoadFullImageSize());
    d->thumbBar->applySettings();
}

}  // namespace Digikam
