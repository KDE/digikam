/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2009 Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "imagepreviewview.h"
#include "imagepreviewview.moc"

// Qt includes

#include <QCursor>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QToolButton>

// KDE includes

#include <kaction.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kdatetable.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>

// LibKIPI includes

#include <libkipi/plugin.h>
#include <libkipi/pluginloader.h>

// Local includes

#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumwidgetstack.h"
#include "contextmenuhelper.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "dimg.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "imageinfo.h"
#include "loadingdescription.h"
#include "metadatahub.h"
#include "paniconwidget.h"
#include "previewloadthread.h"
#include "ratingpopupmenu.h"
#include "tagspopupmenu.h"
#include "themeengine.h"

namespace Digikam
{

class ImagePreviewViewPriv
{
public:

    ImagePreviewViewPriv()
    {
        panIconPopup         = 0;
        panIconWidget        = 0;
        cornerButton         = 0;
        previewThread        = 0;
        previewPreloadThread = 0;
        stack                = 0;
        hasPrev              = false;
        hasNext              = false;
        loadFullImageSize    = false;
        currentFitWindowZoom = 0;
        previewSize          = 1024;
    }

    bool               hasPrev;
    bool               hasNext;
    bool               loadFullImageSize;

    int                previewSize;

    double             currentFitWindowZoom;

    QString            path;
    QString            nextPath;
    QString            previousPath;

    QToolButton       *cornerButton;

    KPopupFrame       *panIconPopup;

    PanIconWidget     *panIconWidget;

    DImg               preview;

    ImageInfo          imageInfo;

    PreviewLoadThread *previewThread;
    PreviewLoadThread *previewPreloadThread;

    AlbumWidgetStack  *stack;
};

ImagePreviewView::ImagePreviewView(QWidget *parent, AlbumWidgetStack *stack)
                : PreviewWidget(parent), d(new ImagePreviewViewPriv)
{
    d->stack = stack;

    // get preview size from screen size, but limit from VGA to WQXGA
    d->previewSize = qMax(KApplication::desktop()->height(),
                          KApplication::desktop()->width());
    if (d->previewSize < 640)
        d->previewSize = 640;
    if (d->previewSize > 2560)
        d->previewSize = 2560;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->cornerButton = new QToolButton(this);
    d->cornerButton->setIcon(SmallIcon("transform-move"));
    d->cornerButton->hide();
    d->cornerButton->setToolTip( i18n("Pan the image to a region"));
    setCornerWidget(d->cornerButton);

    // ------------------------------------------------------------

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

    connect(this, SIGNAL(signalShowNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(signalShowPrevImage()),
            this, SIGNAL(signalPrevItem()));

    connect(this, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(this, SIGNAL(signalLeftButtonClicked()),
            this, SIGNAL(signalBack2Album()));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // ------------------------------------------------------------

    slotReset();
}

ImagePreviewView::~ImagePreviewView()
{
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void ImagePreviewView::setLoadFullImageSize(bool b)
{
    d->loadFullImageSize = b;
    reload();
}

void ImagePreviewView::setImage(const DImg& image)
{
    d->preview = image;

    updateZoomAndSize(true);

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

DImg& ImagePreviewView::getImage() const
{
    return d->preview;
}

void ImagePreviewView::reload()
{
    // cache is cleaned from AlbumIconView::refreshItems
    setImagePath(d->path);
}

void ImagePreviewView::setPreviousNextPaths(const QString& previous, const QString &next)
{
    d->nextPath     = next;
    d->previousPath = previous;
}

void ImagePreviewView::setImagePath(const QString& path)
{
    setCursor( Qt::WaitCursor );

    d->path         = path;
    d->nextPath     = QString();
    d->previousPath = QString();

    if (d->path.isEmpty())
    {
        slotReset();
        unsetCursor();
        return;
    }

    if (!d->previewThread)
    {
        d->previewThread = new PreviewLoadThread();
        connect(d->previewThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg &)),
                this, SLOT(slotGotImagePreview(const LoadingDescription &, const DImg&)));
    }
    if (!d->previewPreloadThread)
    {
        d->previewPreloadThread = new PreviewLoadThread();
        connect(d->previewPreloadThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg &)),
                this, SLOT(slotNextPreload()));
    }

    if (d->loadFullImageSize)
        d->previewThread->loadHighQuality(LoadingDescription(path, 0, AlbumSettings::instance()->getExifRotate()));
    else
        d->previewThread->load(LoadingDescription(path, d->previewSize, AlbumSettings::instance()->getExifRotate()));
}

void ImagePreviewView::slotGotImagePreview(const LoadingDescription &description, const DImg& preview)
{
    if (description.filePath != d->path || description.isThumbnail())
        return;

    if (preview.isNull())
    {
        d->stack->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap,
                   i18n("Cannot display preview for\n\"%1\"",
                   info.fileName()));
        p.end();
        // three copies - but the image is small
        setImage(DImg(pix.toImage()));
        d->stack->previewLoaded();
        emit signalPreviewLoaded(false);
    }
    else
    {
        DImg img(preview);
        if (AlbumSettings::instance()->getExifRotate())
            d->previewThread->exifRotate(img, description.filePath);
        d->stack->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
        setImage(img);
        d->stack->previewLoaded();
        emit signalPreviewLoaded(true);
    }

    unsetCursor();
    slotNextPreload();
}

void ImagePreviewView::slotNextPreload()
{
    QString loadPath;
    if (!d->nextPath.isNull())
    {
        loadPath    = d->nextPath;
        d->nextPath = QString();
    }
    else if (!d->previousPath.isNull())
    {
        loadPath        = d->previousPath;
        d->previousPath = QString();
    }
    else
        return;

    if (d->loadFullImageSize)
        d->previewThread->loadHighQuality(LoadingDescription(loadPath, 0,
                                          AlbumSettings::instance()->getExifRotate()));
    else
        d->previewPreloadThread->load(LoadingDescription(loadPath, d->previewSize,
                                      AlbumSettings::instance()->getExifRotate()));
}

void ImagePreviewView::setImageInfo(const ImageInfo & info, const ImageInfo &previous, const ImageInfo &next)
{
    d->imageInfo = info;
    d->hasPrev   = previous.isNull() ? false : true;
    d->hasNext   = next.isNull()     ? false : true;

    if (!d->imageInfo.isNull())
        setImagePath(info.filePath());
    else
        setImagePath();

    setPreviousNextPaths(previous.filePath(), next.filePath());
}

ImageInfo ImagePreviewView::getImageInfo() const
{
    return d->imageInfo;
}

void ImagePreviewView::slotContextMenu()
{
    if (d->imageInfo.isNull())
        return;

    QList<qlonglong> idList;
    idList << d->imageInfo.id();
    KUrl::List selectedItems;
    selectedItems << d->imageInfo.fileUrl();

    // Temporary actions --------------------------------------

    QAction *back2AlbumAction, *prevAction, *nextAction = 0;

    back2AlbumAction = new QAction(SmallIcon("folder-image"), i18n("Back to Album"), this);
    prevAction       = new QAction(SmallIcon("go-previous"),  i18nc("go to previous image", "Back"), this);
    nextAction       = new QAction(SmallIcon("go-next"),      i18nc("go to next image", "Forward"), this);
    prevAction->setEnabled(d->hasPrev);
    nextAction->setEnabled(d->hasNext);

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(prevAction, true);
    cmhelper.addAction(nextAction, true);
    cmhelper.addAction(back2AlbumAction);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_edit");
    cmhelper.addAction("image_find_similar");
    cmhelper.addActionLightTable();
    cmhelper.addQueueManagerMenu();
    cmhelper.addGotoMenu(idList);
    cmhelper.addServicesMenu(selectedItems);
    cmhelper.addKipiActions();
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addActionItemDelete(this, SLOT(slotDeleteItem()));
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(idList);
    cmhelper.addRemoveTagsMenu(idList);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addRatingMenu();

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SLOT(slotGotoTag(int)));

    connect(&cmhelper, SIGNAL(signalGotoAlbum(ImageInfo&)),
            this, SIGNAL(signalGotoAlbumAndItem(ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalGotoDate(ImageInfo&)),
            this, SIGNAL(signalGotoDateAndItem(ImageInfo&)));

    // handle temporary actions

    QAction* choice = cmhelper.exec(QCursor::pos());
    if (choice)
    {
        if (choice == prevAction)            emit signalPrevItem();
        else if (choice == nextAction)       emit signalNextItem();
        else if (choice == back2AlbumAction) emit signalBack2Album();
    }

    // cleanup -------------------------

    popmenu.deleteLater();
    delete back2AlbumAction;
    delete prevAction;
    delete nextAction;
}

void ImagePreviewView::slotAssignTag(int tagID)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, true);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImagePreviewView::slotRemoveTag(int tagID)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, false);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImagePreviewView::slotAssignRating(int rating)
{
    rating = qMin(5, qMax(0, rating));
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setRating(rating);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImagePreviewView::slotThemeChanged()
{
    QPalette plt(palette());
    plt.setColor(backgroundRole(), ThemeEngine::instance()->baseColor());
    setPalette(plt);
}

void ImagePreviewView::slotCornerButtonPressed()
{
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
    }

    d->panIconPopup    = new KPopupFrame(this);
    PanIconWidget *pan = new PanIconWidget(d->panIconPopup);
    pan->setImage(180, 120, getImage());
    d->panIconPopup->setMainWidget(pan);

    QRect r((int)(contentsX()    / zoomFactor()), (int)(contentsY()     / zoomFactor()),
            (int)(visibleWidth() / zoomFactor()), (int)(visibleHeight() / zoomFactor()));
    pan->setRegionSelection(r);
    pan->setMouseFocus();

    connect(pan, SIGNAL(signalSelectionMoved(const QRect&, bool)),
            this, SLOT(slotPanIconSelectionMoved(const QRect&, bool)));

    connect(pan, SIGNAL(signalHidden()),
            this, SLOT(slotPanIconHiden()));

    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x()+ viewport()->size().width());
    g.setY(g.y()+ viewport()->size().height());
    d->panIconPopup->popup(QPoint(g.x() - d->panIconPopup->width(),
                                  g.y() - d->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void ImagePreviewView::slotPanIconHiden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void ImagePreviewView::slotDeleteItem()
{
    emit signalDeleteItem();
}

void ImagePreviewView::slotPanIconSelectionMoved(const QRect& r, bool b)
{
    setContentsPos((int)(r.x()*zoomFactor()), (int)(r.y()*zoomFactor()));

    if (b)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
        slotPanIconHiden();
    }
}

void ImagePreviewView::zoomFactorChanged(double zoom)
{
    updateScrollBars();

    if (horizontalScrollBar()->isVisible() || verticalScrollBar()->isVisible())
        d->cornerButton->show();
    else
        d->cornerButton->hide();

    PreviewWidget::zoomFactorChanged(zoom);
}

void ImagePreviewView::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    Q3ScrollView::resizeEvent(e);

    if (d->imageInfo.isNull())
        d->cornerButton->hide();

    updateZoomAndSize(false);
}

void ImagePreviewView::updateZoomAndSize(bool alwaysFitToWindow)
{
    // Set zoom for fit-in-window as minimum, but don't scale up images
    // that are smaller than the available space, only scale down.
    double zoom = calcAutoZoomFactor(ZoomInOnly);
    setZoomMin(zoom);
    setZoomMax(zoom*12.0);

    // Is currently the zoom factor set to fit to window? Then set it again to fit the new size.
    if (zoomFactor() < zoom || alwaysFitToWindow || zoomFactor() == d->currentFitWindowZoom)
    {
        setZoomFactor(zoom);
    }

    // store which zoom factor means it is fit to window
    d->currentFitWindowZoom = zoom;

    updateContentsSize();
}

int ImagePreviewView::previewWidth()
{
    return d->preview.width();
}

int ImagePreviewView::previewHeight()
{
    return d->preview.height();
}

bool ImagePreviewView::previewIsNull()
{
    return d->preview.isNull();
}

void ImagePreviewView::resetPreview()
{
    d->preview   = DImg();
    d->path      = QString();
    d->imageInfo = ImageInfo();

    updateZoomAndSize(true);
    emit signalPreviewLoaded(false);
}

void ImagePreviewView::paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh)
{
    DImg img     = d->preview.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());
    QPixmap pix2 = img.convertToPixmap();
    QPainter p(pix);
    p.drawPixmap(0, 0, pix2);
    p.end();
}

void ImagePreviewView::slotGotoTag(int tagID)
{
    emit signalGotoTagAndItem(tagID);
}

}  // namespace Digikam
