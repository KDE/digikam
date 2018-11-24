/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt model-view for items - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "itemdelegate.h"
#include "itemdelegate_p.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QCache>
#include <QPainter>
#include <QIcon>
#include <QApplication>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "itemcategorydrawer.h"
#include "itemcategorizedview.h"
#include "itemdelegateoverlay.h"
#include "itemmodel.h"
#include "itemfiltermodel.h"
#include "itemthumbnailmodel.h"
#include "thumbnailloadthread.h"
#include "applicationsettings.h"

namespace Digikam
{

void ItemDelegate::ItemDelegatePrivate::clearRects()
{
    ItemViewDelegatePrivate::clearRects();
    dateRect             = QRect(0, 0, 0, 0);
    modDateRect          = QRect(0, 0, 0, 0);
    pixmapRect           = QRect(0, 0, 0, 0);
    nameRect             = QRect(0, 0, 0, 0);
    titleRect            = QRect(0, 0, 0, 0);
    commentsRect         = QRect(0, 0, 0, 0);
    resolutionRect       = QRect(0, 0, 0, 0);
    coordinatesRect      = QRect(0, 0, 0, 0);
    arRect               = QRect(0, 0, 0, 0);
    sizeRect             = QRect(0, 0, 0, 0);
    tagRect              = QRect(0, 0, 0, 0);
    imageInformationRect = QRect(0, 0, 0, 0);
    pickLabelRect        = QRect(0, 0, 0, 0);
    groupRect            = QRect(0, 0, 0, 0);
}

ItemDelegate::ItemDelegate(QObject* const parent)
    : ItemViewDelegate(*new ItemDelegatePrivate, parent)
{
}

ItemDelegate::ItemDelegate(ItemDelegate::ItemDelegatePrivate& dd, QObject* const parent)
    : ItemViewDelegate(dd, parent)
{
}

ItemDelegate::~ItemDelegate()
{
    Q_D(ItemDelegate);
    // crashes for a lot of people, see bug 230515. Cause unknown.
    //delete d->categoryDrawer;
    Q_UNUSED(d); // To please compiler about warnings.
}

void ItemDelegate::setView(ItemCategorizedView* view)
{
    Q_D(ItemDelegate);
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

void ItemDelegate::setModel(QAbstractItemModel* model)
{
    Q_D(ItemDelegate);

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

void ItemDelegate::setSpacing(int spacing)
{
    Q_D(ItemDelegate);

    if (d->categoryDrawer)
    {
        d->categoryDrawer->setLowerSpacing(spacing);
    }

    ItemViewDelegate::setSpacing(spacing);
}

ItemCategoryDrawer* ItemDelegate::categoryDrawer() const
{
    Q_D(const ItemDelegate);
    return d->categoryDrawer;
}

QRect ItemDelegate::commentsRect() const
{
    Q_D(const ItemDelegate);
    return d->commentsRect;
}

QRect ItemDelegate::tagsRect() const
{
    Q_D(const ItemDelegate);
    return d->tagRect;
}

QRect ItemDelegate::pixmapRect() const
{
    Q_D(const ItemDelegate);
    return d->pixmapRect;
}

QRect ItemDelegate::imageInformationRect() const
{
    Q_D(const ItemDelegate);
    return d->imageInformationRect;
}

QRect ItemDelegate::groupIndicatorRect() const
{
    Q_D(const ItemDelegate);
    return d->groupRect;
}

QRect ItemDelegate::coordinatesIndicatorRect() const
{
    Q_D(const ItemDelegate);
    return d->coordinatesRect;
}

void ItemDelegate::prepareThumbnails(ItemThumbnailModel* thumbModel, const QList<QModelIndex>& indexes)
{
    thumbModel->prepareThumbnails(indexes, thumbnailSize());
}

QPixmap ItemDelegate::retrieveThumbnailPixmap(const QModelIndex& index, int thumbnailSize)
{
    // work around constness
    QAbstractItemModel* const model = const_cast<QAbstractItemModel*>(index.model());
    // set requested thumbnail size
    model->setData(index, thumbnailSize, ItemModel::ThumbnailRole);
    // get data from model
    QVariant thumbData              = index.data(ItemModel::ThumbnailRole);
    // reset to default thumbnail size
    model->setData(index, QVariant(), ItemModel::ThumbnailRole);

    return thumbData.value<QPixmap>();
}

QPixmap ItemDelegate::thumbnailPixmap(const QModelIndex& index) const
{
    Q_D(const ItemDelegate);
    return retrieveThumbnailPixmap(index, d->thumbSize.size());
}

void ItemDelegate::paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(const ItemDelegate);
    ItemInfo info = ItemModel::retrieveItemInfo(index);

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

    bool groupedAndClosed  = (info.hasGroupedImages() &&
                              !index.data(ItemFilterModel::GroupIsOpenRole).toBool() &&
                              ApplicationSettings::instance()->getDrawFramesToGrouped());

    QRect actualPixmapRect = drawThumbnail(p, d->pixmapRect,
                                           pix, thumbnailPixmap(index),
                                           groupedAndClosed);

    if (!actualPixmapRect.isNull())
    {
        const_cast<ItemDelegate*>(this)->updateActualPixmapRect(index, actualPixmapRect);
    }

    if (!d->ratingRect.isNull())
    {
        drawRating(p, index, d->ratingRect, info.rating(), isSelected);
    }

    // Draw Color Label rectangle
    drawColorLabelRect(p, option, isSelected, info.colorLabel());

    p->setPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                         : qApp->palette().color(QPalette::Text));

/*
    // If there is ImageHistory present, paint a small icon over the thumbnail to indicate that this is derived image
    if (info.hasImageHistory())
    {
        p->drawPixmap(d->pixmapRect.right()-24, d->pixmapRect.bottom()-24, QIcon::fromTheme(QLatin1String("svn_switch")).pixmap(22, 22));
    }
*/

    if (!d->nameRect.isNull())
    {
        drawName(p, d->nameRect, info.name());
    }

    if (!d->titleRect.isNull())
    {
        drawTitle(p, d->titleRect, info.title());
    }

    if (!d->commentsRect.isNull())
    {
        drawComments(p, d->commentsRect, info.comment());
    }

    if (!d->dateRect.isNull())
    {
        drawCreationDate(p, d->dateRect, info.dateTime());
    }

    if (!d->modDateRect.isNull() && info.modDateTime().isValid() &&
                                    info.modDateTime() != info.dateTime())
    {
        drawModificationDate(p, d->modDateRect, info.modDateTime());
    }

    if (!d->resolutionRect.isNull())
    {
        drawImageSize(p, d->resolutionRect, info.dimensions());
    }

    if (!d->arRect.isNull())
    {
        drawAspectRatio(p, d->arRect, info.dimensions());
    }

    if (!d->sizeRect.isNull())
    {
        drawFileSize(p, d->sizeRect, info.fileSize());
    }

    if (!d->groupRect.isNull())
    {
        drawGroupIndicator(p, d->groupRect, info.numberOfGroupedImages(),
                           index.data(ItemFilterModel::GroupIsOpenRole).toBool());
    }

    if (!d->tagRect.isNull())
    {
        QStringList tagsList = AlbumManager::instance()->tagNames(info.tagIds());
        tagsList.sort();
        QString tags         = tagsList.join(QLatin1String(", "));
        drawTags(p, d->tagRect, tags, isSelected);
    }

    if (!d->pickLabelRect.isNull())
    {
        drawPickLabelIcon(p, d->pickLabelRect, info.pickLabel());
    }

    bool left  = index.data(ItemModel::LTLeftPanelRole).toBool();
    bool right = index.data(ItemModel::LTRightPanelRole).toBool();
    drawPanelSideIcon(p, left, right);

    if (d->drawImageFormat)
    {
        QString frm  = info.format();

        if (frm.contains(QLatin1Char('-')))
            frm = frm.section(QLatin1Char('-'), -1);   // For RAW format annotated as "RAW-xxx" => "xxx"

        drawImageFormat(p, actualPixmapRect, frm, d->drawImageFormatTop);
    }

    if (info.id() == info.currentReferenceImage())
    {
        drawSpecialInfo(p, actualPixmapRect, i18n("Reference Image"));
    }

    if (d->drawCoordinates && info.hasCoordinates())
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

QPixmap ItemDelegate::pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const
{
    QPixmap icon;

    if (!indexes.isEmpty())
    {
        icon = thumbnailPixmap(indexes.first());
    }

    return makeDragPixmap(option, indexes, icon);
}

bool ItemDelegate::acceptsToolTip(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                   QRect* toolTipRect) const
{
    return onActualPixmapRect(pos, visualRect, index, toolTipRect);
}

bool ItemDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                      QRect* activationRect) const
{
    return onActualPixmapRect(pos, visualRect, index, activationRect);
}

bool ItemDelegate::onActualPixmapRect(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
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

void ItemDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ItemDelegate);

    if (d->categoryDrawer)
    {
        d->categoryDrawer->setDefaultViewOptions(option);
    }

    ItemViewDelegate::setDefaultViewOptions(option);
}

void ItemDelegate::invalidatePaintingCache()
{
    Q_D(ItemDelegate);

    if (d->categoryDrawer)
    {
        d->categoryDrawer->invalidatePaintingCache();
    }

    ItemViewDelegate::invalidatePaintingCache();
}

void ItemDelegate::updateContentWidth()
{
    Q_D(ItemDelegate);
    d->contentWidth = d->thumbSize.size() + 2*d->radius;
}

void ItemDelegate::updateSizeRectsAndPixmaps()
{
    Q_D(ItemDelegate);

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

    if (!d->ratingRect.isNull())
    {
        // Normally we prepare the pixmaps over the background of the rating rect.
        // If the rating is drawn over the thumbnail, we can only draw over a transparent pixmap.
        prepareRatingPixmaps(!d->ratingOverThumbnail);
    }

    // ---- Drawing related caches ----

    clearCaches();
}

void ItemDelegate::clearCaches()
{
    Q_D(ItemDelegate);
    ItemViewDelegate::clearCaches();
    d->actualPixmapRectCache.clear();
}

void ItemDelegate::clearModelDataCaches()
{
    Q_D(ItemDelegate);
    d->actualPixmapRectCache.clear();
}

void ItemDelegate::modelChanged()
{
    Q_D(ItemDelegate);
    clearModelDataCaches();
    setModel(d->currentView ? d->currentView->model() : 0);
}

void ItemDelegate::modelContentsChanged()
{
    clearModelDataCaches();
}

QRect ItemDelegate::actualPixmapRect(const QModelIndex& index) const
{
    Q_D(const ItemDelegate);
    // We do not recompute if not found. Assumption is cache is always properly updated.
    QRect* rect = d->actualPixmapRectCache.object(index.row());

    if (rect)
    {
        return *rect;
    }
    else
    {
        return d->pixmapRect;
    }
}

void ItemDelegate::updateActualPixmapRect(const QModelIndex& index, const QRect& rect)
{
    Q_D(ItemDelegate);
    QRect* const old = d->actualPixmapRectCache.object(index.row());

    if (!old || *old != rect)
    {
        d->actualPixmapRectCache.insert(index.row(), new QRect(rect));
    }
}

int ItemDelegate::calculatethumbSizeToFit(int ws)
{
    Q_D(ItemDelegate);

    int ts     = thumbnailSize().size();
    int gs     = gridSize().width();
    int sp     = spacing();
    ws         = ws - 2*sp;

    // Thumbnails size loop to check (upper/lower)
    int ts1, ts2;
    // New grid size used in loop
    int ngs;

    double rs1 = fmod((double)ws, (double)gs);

    for (ts1 = ts ; ts1 < ThumbnailSize::maxThumbsSize() ; ++ts1)
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
    {
        return (ts2);
    }

    return (ts1);
}

} // namespace Digikam
