/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2013      by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfotodelegate.h"
#include "showfotodelegatepriv.h"

// Qt includes

#include <QPainter>
#include <QRect>
#include <QApplication>

// KDE includes

#include <ksharedconfig.h>

// Local includes

#include "showfotoimagemodel.h"
#include "showfotofiltermodel.h"
#include "showfotothumbnailbar.h"
#include "showfotoiteminfo.h"
#include "showfotothumbnailmodel.h"
#include "showfotosettings.h"

namespace ShowFoto
{

void ShowfotoDelegate::ShowfotoDelegatePrivate::clearRects()
{
    ItemViewShowfotoDelegatePrivate::clearRects();
    dateRect             = QRect(0, 0, 0, 0);
    pixmapRect           = QRect(0, 0, 0, 0);
    nameRect             = QRect(0, 0, 0, 0);
    resolutionRect       = QRect(0, 0, 0, 0);
    sizeRect             = QRect(0, 0, 0, 0);
    imageInformationRect = QRect(0, 0, 0, 0);
    coordinatesRect      = QRect(0, 0, 0, 0);
}

ShowfotoDelegate::ShowfotoDelegate(QObject* const parent)
    : ItemViewShowfotoDelegate(*new ShowfotoDelegatePrivate, parent)
{
}

ShowfotoDelegate::ShowfotoDelegate(ShowfotoDelegate::ShowfotoDelegatePrivate& dd, QObject* const parent)
    : ItemViewShowfotoDelegate(dd, parent)
{
}

ShowfotoDelegate::~ShowfotoDelegate()
{
    Q_D(ShowfotoDelegate);
    Q_UNUSED(d); // To please compiler about warnings.
}

void ShowfotoDelegate::setView(ShowfotoThumbnailBar* view)
{
    Q_D(ShowfotoDelegate);
    setViewOnAllOverlays(view);

    if (d->currentView)
    {
        disconnect(d->currentView, SIGNAL(modelChanged()),
                   this, SLOT(modelChanged()));
    }

    d->currentView = view;

    setModel(view ? view->model() : 0);

    if (d->currentView)
    {
        connect(d->currentView, SIGNAL(modelChanged()),
                this, SLOT(modelChanged()));
    }
}

void ShowfotoDelegate::setModel(QAbstractItemModel* model)
{
    Q_D(ShowfotoDelegate);

    // 1) We only need the model to invalidate model-index based caches on change
    // 2) We do not need to care for overlays. The view calls setActive() on them on model change

    if (model == d->currentModel)
    {
        return;
    }

    if (d->currentModel)
    {
        disconnect(d->currentModel, 0, this, 0);
    }

    d->currentModel = model;

    if (d->currentModel)
    {
        connect(d->currentModel, SIGNAL(layoutAboutToBeChanged()),
                this, SLOT(modelContentsChanged()));

        connect(d->currentModel, SIGNAL(modelAboutToBeReset()),
                this, SLOT(modelContentsChanged()));

        connect(d->currentModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                this, SLOT(modelContentsChanged()));

        connect(d->currentModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(modelContentsChanged()));

        connect(d->currentModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(modelContentsChanged()));
    }
}

void ShowfotoDelegate::setSpacing(int spacing)
{
    ItemViewShowfotoDelegate::setSpacing(spacing);
}

QRect ShowfotoDelegate::pixmapRect() const
{
    Q_D(const ShowfotoDelegate);
    return d->pixmapRect;
}

QRect ShowfotoDelegate::imageInformationRect() const
{
    Q_D(const ShowfotoDelegate);
    return d->imageInformationRect;
}

QRect ShowfotoDelegate::groupIndicatorRect() const
{
    Q_D(const ShowfotoDelegate);
    return d->groupRect;
}

QRect ShowfotoDelegate::coordinatesIndicatorRect() const
{
    Q_D(const ShowfotoDelegate);
    return d->coordinatesRect;
}

QPixmap ShowfotoDelegate::retrieveThumbnailPixmap(const QModelIndex& index, int thumbnailSize)
{
    // work around constness
    QAbstractItemModel* const model = const_cast<QAbstractItemModel*>(index.model());
    // set requested thumbnail size
    model->setData(index, thumbnailSize, ShowfotoImageModel::ThumbnailRole);
    // get data from model
    QVariant thumbData              = index.data(ShowfotoImageModel::ThumbnailRole);

    return thumbData.value<QPixmap>();
}

QPixmap ShowfotoDelegate::thumbnailPixmap(const QModelIndex& index) const
{
    Q_D(const ShowfotoDelegate);
    return retrieveThumbnailPixmap(index, d->thumbSize.size());
}

void ShowfotoDelegate::paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(const ShowfotoDelegate);
    ShowfotoItemInfo info     = ShowfotoImageModel::retrieveShowfotoItemInfo(index);
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("ImageViewer Settings");

    if (info.isNull())
    {
        return;
    }

    // state of painter must not be changed
    p->save();
    p->translate(option.rect.topLeft());

    QRect r;
    bool isSelected = (option.state & QStyle::State_Selected);

    // Thumbnail
    QPixmap pix;

    if (isSelected)
    {
        pix = d->selPixmap;
    }
    else
    {
        pix = d->regPixmap;
    }

    QRect actualPixmapRect = drawThumbnail(p, d->pixmapRect, pix, thumbnailPixmap(index));

    if (!actualPixmapRect.isNull())
    {
        const_cast<ShowfotoDelegate*>(this)->updateActualPixmapRect(index, actualPixmapRect);
    }

    p->setPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                         : qApp->palette().color(QPalette::Text));

    if (!d->nameRect.isNull())
    {
        drawName(p, d->nameRect, info.name);
    }

    if (!d->dateRect.isNull())
    {
        drawCreationDate(p, d->dateRect, info.ctime);
    }

    if (!d->sizeRect.isNull())
    {
        drawFileSize(p, d->sizeRect, info.size);
    }

    if (ShowfotoSettings::instance()->getShowFormatOverThumbnail())
    {
        QString frm = info.mime;
        drawImageFormat(p, actualPixmapRect, frm);
    }

    if (ShowfotoSettings::instance()->getShowCoordinates() && info.photoInfo.hasCoordinates)
    {
        drawGeolocationIndicator(p, d->coordinatesRect);
    }

    if (d->drawFocusFrame)
    {
        drawFocusRect(p, option, isSelected);
    }

    if (d->drawMouseOverFrame)
    {
        drawMouseOverRect(p, option);
    }

    p->restore();

    drawOverlays(p, option, index);
}

QPixmap ShowfotoDelegate::pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const
{
    QPixmap icon;

    if (!indexes.isEmpty())
    {
        icon = thumbnailPixmap(indexes.first());
    }

    return makeDragPixmap(option, indexes, icon);
}

bool ShowfotoDelegate::acceptsToolTip(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                      QRect* toolTipRect) const
{
    return onActualPixmapRect(pos, visualRect, index, toolTipRect);
}

bool ShowfotoDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                         QRect* activationRect) const
{
    return onActualPixmapRect(pos, visualRect, index, activationRect);
}

bool ShowfotoDelegate::onActualPixmapRect(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                          QRect* returnRect) const
{
    QRect actualRect = actualPixmapRect(index);

    if (actualRect.isNull())
    {
        return false;
    }

    actualRect.translate(visualRect.topLeft());

    if (returnRect)
    {
        *returnRect = actualRect;
    }

    return actualRect.contains(pos);
}

void ShowfotoDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{

    ItemViewShowfotoDelegate::setDefaultViewOptions(option);
}

void ShowfotoDelegate::invalidatePaintingCache()
{

    ItemViewShowfotoDelegate::invalidatePaintingCache();
}

void ShowfotoDelegate::updateContentWidth()
{
    Q_D(ShowfotoDelegate);
    d->contentWidth = d->thumbSize.size() + 2*d->radius;
}

void ShowfotoDelegate::updateSizeRectsAndPixmaps()
{
    Q_D(ShowfotoDelegate);

    // ---- Reset rects and prepare fonts ----

    d->clearRects();
    prepareFonts();

    // ---- Fixed sizes and metrics ----

    updateContentWidth();
    prepareMetrics(d->contentWidth);

    // ---- Calculate rects ----

    updateRects();

    // ---- Cached pixmaps ----

    prepareBackground();

    // ---- Drawing related caches ----

    clearCaches();
}

void ShowfotoDelegate::clearCaches()
{
    Q_D(ShowfotoDelegate);
    ItemViewShowfotoDelegate::clearCaches();
    d->actualPixmapRectCache.clear();
}

void ShowfotoDelegate::clearModelDataCaches()
{
    Q_D(ShowfotoDelegate);
    d->actualPixmapRectCache.clear();
}

void ShowfotoDelegate::modelChanged()
{
    Q_D(ShowfotoDelegate);
    clearModelDataCaches();
    setModel(d->currentView ? d->currentView->model() : 0);
}

void ShowfotoDelegate::modelContentsChanged()
{
    clearModelDataCaches();
}

QRect ShowfotoDelegate::actualPixmapRect(const QModelIndex& index) const
{
    Q_D(const ShowfotoDelegate);
    // We do not recompute if not found. Assumption is cache is always properly updated.
    QRect* const rect = d->actualPixmapRectCache.object(index.row());

    if (rect)
    {
        return *rect;
    }
    else
    {
        return d->pixmapRect;
    }
}

void ShowfotoDelegate::updateActualPixmapRect(const QModelIndex& index, const QRect& rect)
{
    Q_D(ShowfotoDelegate);
    QRect* const old = d->actualPixmapRectCache.object(index.row());

    if (!old || *old != rect)
    {
        d->actualPixmapRectCache.insert(index.row(), new QRect(rect));
    }
}

int ShowfotoDelegate::calculatethumbSizeToFit(int ws)
{
    Q_D(ShowfotoDelegate);

    int ts     = thumbnailSize().size();
    int gs     = gridSize().width();
    int sp     = spacing();
    ws         = ws - 2*sp;

    // Thumbnails size loop to check (upper/lower)
    int ts1, ts2;
    // New grid size used in loop
    int ngs;

    double rs1 = fmod((double)ws, (double)gs);

    for (ts1 = ts ; ts1 < ThumbnailSize::Huge ; ++ts1)
    {
        ngs        = ts1 + 2*(d->margin + d->radius) + sp;
        double nrs = fmod((double)ws, (double)ngs);

        if (nrs <= rs1)
        {
            rs1 = nrs;
        }
        else
        {
            break;
        }
    }

    double rs2 = fmod((double)ws, (double)gs);

    for (ts2 = ts ; ts2 > ThumbnailSize::Small ; --ts2)
    {
        ngs        = ts2 + 2*(d->margin + d->radius) + sp;
        double nrs = fmod((double)ws, (double)ngs);

        if (nrs >= rs2)
        {
            rs2 = nrs;
        }
        else
        {
            rs2 = nrs;
            break;
        }
    }

    if (rs1 > rs2)
        return (ts2);

    return (ts1);
}

// --- ShowfotoThumbnailDelegate ---------------------------------------

void ShowfotoThumbnailDelegatePrivate::init(ShowfotoThumbnailDelegate* const q)
{
    Q_UNUSED(q);
}

// ------------------------------------------------------------------------------------------------

ShowfotoThumbnailDelegate::ShowfotoThumbnailDelegate(ShowfotoThumbnailBar* const parent)
    : ShowfotoDelegate(*new ShowfotoThumbnailDelegatePrivate, parent)
{
    Q_D(ShowfotoThumbnailDelegate);
    d->init(this);
}

ShowfotoThumbnailDelegate::~ShowfotoThumbnailDelegate()
{
}

void ShowfotoThumbnailDelegate::setFlow(QListView::Flow flow)
{
    Q_D(ShowfotoThumbnailDelegate);
    d->flow = flow;
}

void ShowfotoThumbnailDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ShowfotoThumbnailDelegate);
    // store before calling parent class
    d->viewSize = option.rect;
    ShowfotoDelegate::setDefaultViewOptions(option);
}

int ShowfotoThumbnailDelegate::maximumSize() const
{
    Q_D(const ShowfotoThumbnailDelegate);

    return ThumbnailSize::Huge + (2*d->radius + 2*d->margin);
}

int ShowfotoThumbnailDelegate::minimumSize() const
{
    Q_D(const ShowfotoThumbnailDelegate);
    return ThumbnailSize::Small + 2*d->radius + 2*d->margin;
}

bool ShowfotoThumbnailDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect,
                                                const QModelIndex& index, QRect* activationRect) const
{
    // reuse implementation from grandparent
    return ItemViewShowfotoDelegate::acceptsActivation(pos, visualRect, index, activationRect);
}

void ShowfotoThumbnailDelegate::updateContentWidth()
{
    Q_D(ShowfotoThumbnailDelegate);
    int maxSize;

    if (d->flow == QListView::LeftToRight)
    {
        maxSize = d->viewSize.height();
    }
    else
    {
        maxSize = d->viewSize.width();
    }

    d->thumbSize = thumbnailPixmapSize(true, maxSize - 2*d->radius - 2*d->margin);

    ShowfotoDelegate::updateContentWidth();
}

int ShowfotoThumbnailDelegate::thumbnailPixmapSize(bool withHighlight, int size)
{
    if (withHighlight && size >= 10)
    {
        return size + 2;
    }

    return size;
}

void ShowfotoThumbnailDelegate::updateRects()
{
    Q_D(ShowfotoThumbnailDelegate);

    d->pixmapRect      = QRect(d->margin, d->margin, d->contentWidth, d->contentWidth);
    d->rect            = QRect(0, 0, d->contentWidth + 2*d->margin, d->contentWidth + 2*d->margin);
    d->coordinatesRect = QRect(d->contentWidth - 16+2, d->pixmapRect.top(), 16, 16);

    if (d->flow == QListView::LeftToRight)
    {
        d->gridSize = QSize(d->rect.width() + d->spacing, d->rect.height());
    }
    else
    {
        d->gridSize = QSize(d->rect.width(), d->rect.height() + d->spacing);
    }
}

// --- ShowfotoNormalDelegate -----------------------------------------------------------------------

void ShowfotoNormalDelegatePrivate::init(ShowfotoNormalDelegate* const q, ShowfotoThumbnailBar* const parent)
{
    //categoryDrawer = new ShowfotoCategoryDrawer(parent);
    Q_UNUSED(parent);
    Q_UNUSED(q);
}

// ------------------------------------------------------------------------------------------------

ShowfotoNormalDelegate::ShowfotoNormalDelegate(ShowfotoThumbnailBar* const parent)
    : ShowfotoDelegate(*new ShowfotoNormalDelegatePrivate, parent)
{
    Q_D(ShowfotoNormalDelegate);
    d->init(this, parent);
}

ShowfotoNormalDelegate::ShowfotoNormalDelegate(ShowfotoNormalDelegatePrivate& dd, ShowfotoThumbnailBar* const parent)
    : ShowfotoDelegate(dd, parent)
{

    Q_D(ShowfotoNormalDelegate);
    d->init(this, parent);
}

ShowfotoNormalDelegate::~ShowfotoNormalDelegate()
{
}

void ShowfotoNormalDelegate::updateRects()
{
    Q_D(ShowfotoNormalDelegate);

    int y                                      = d->margin;
    d->pixmapRect                              = QRect(d->margin, y, d->contentWidth, d->contentWidth);
    y                                          = d->pixmapRect.bottom();
    d->imageInformationRect                    = QRect(d->margin, y, d->contentWidth, 0);

    d->imageInformationRect.setBottom(y);

    d->rect     = QRect(0, 0, d->contentWidth + 2*d->margin, y+d->margin+d->radius);
    d->gridSize = QSize(d->rect.width() + d->spacing, d->rect.height() + d->spacing);
}

} // namespace ShowFoto
