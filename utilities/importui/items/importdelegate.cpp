/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-08
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importdelegate.h"
#include "importdelegatepriv.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QCache>
#include <QPainter>
#include <QRect>
#include <QApplication>

// Local includes

#include "importimagemodel.h"
#include "importfiltermodel.h"
#include "importsettings.h"
#include "importcategorizedview.h"
#include "albummanager.h"

namespace Digikam
{

void ImportDelegate::ImportDelegatePrivate::clearRects()
{
    ItemViewImportDelegatePrivate::clearRects();
    dateRect             = QRect(0, 0, 0, 0);
    pixmapRect           = QRect(0, 0, 0, 0);
    nameRect             = QRect(0, 0, 0, 0);
//  titleRect            = QRect(0, 0, 0, 0);
//  commentsRect         = QRect(0, 0, 0, 0);
    resolutionRect       = QRect(0, 0, 0, 0);
    sizeRect             = QRect(0, 0, 0, 0);
    downloadRect         = QRect(0, 0, 0, 0);
    lockRect             = QRect(0, 0, 0, 0);
    coordinatesRect      = QRect(0, 0, 0, 0);
    tagRect              = QRect(0, 0, 0, 0);
    ratingRect           = QRect(0, 0, 0, 0);
    imageInformationRect = QRect(0, 0, 0, 0);
    pickLabelRect        = QRect(0, 0, 0, 0);
    groupRect            = QRect(0, 0, 0, 0);
}

ImportDelegate::ImportDelegate(QObject* const parent)
    : ItemViewImportDelegate(*new ImportDelegatePrivate, parent)
{
}

ImportDelegate::ImportDelegate(ImportDelegate::ImportDelegatePrivate& dd, QObject* const parent)
    : ItemViewImportDelegate(dd, parent)
{
}

ImportDelegate::~ImportDelegate()
{
    Q_D(ImportDelegate);
    Q_UNUSED(d); // To please compiler about warnings.
}

void ImportDelegate::setView(ImportCategorizedView* view)
{
    Q_D(ImportDelegate);
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

void ImportDelegate::setModel(QAbstractItemModel* model)
{
    Q_D(ImportDelegate);

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

void ImportDelegate::setSpacing(int spacing)
{
    Q_D(ImportDelegate);

    if (d->categoryDrawer)
    {
        d->categoryDrawer->setLowerSpacing(spacing);
    }

    ItemViewImportDelegate::setSpacing(spacing);
}

ImportCategoryDrawer* ImportDelegate::categoryDrawer() const
{
    Q_D(const ImportDelegate);
    return d->categoryDrawer;
}

/*
QRect ImportDelegate::commentsRect() const
{
   Q_D(const ImportDelegate);
   return d->commentsRect;
}
*/

QRect ImportDelegate::tagsRect() const
{
    Q_D(const ImportDelegate);
    return d->tagRect;
}

QRect ImportDelegate::pixmapRect() const
{
    Q_D(const ImportDelegate);
    return d->pixmapRect;
}

QRect ImportDelegate::imageInformationRect() const
{
    Q_D(const ImportDelegate);
    return d->imageInformationRect;
}

QRect ImportDelegate::groupIndicatorRect() const
{
    Q_D(const ImportDelegate);
    return d->groupRect;
}

QRect ImportDelegate::downloadIndicatorRect() const
{
    Q_D(const ImportDelegate);
    return d->downloadRect;
}

QRect ImportDelegate::lockIndicatorRect() const
{
    Q_D(const ImportDelegate);
    return d->lockRect;
}

QRect ImportDelegate::coordinatesIndicatorRect() const
{
    Q_D(const ImportDelegate);
    return d->coordinatesRect;
}

QPixmap ImportDelegate::retrieveThumbnailPixmap(const QModelIndex& index, int thumbnailSize)
{
    // work around constness
    QAbstractItemModel* const model = const_cast<QAbstractItemModel*>(index.model());
    // set requested thumbnail size
    model->setData(index, thumbnailSize, ImportImageModel::ThumbnailRole);
    // get data from model
    QVariant thumbData              = index.data(ImportImageModel::ThumbnailRole);
    return (thumbData.value<QPixmap>());
}

QPixmap ImportDelegate::thumbnailPixmap(const QModelIndex& index) const
{
    Q_D(const ImportDelegate);
    return retrieveThumbnailPixmap(index, d->thumbSize.size());
}

void ImportDelegate::paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_D(const ImportDelegate);
    CamItemInfo info = ImportImageModel::retrieveCamItemInfo(index);

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
        const_cast<ImportDelegate*>(this)->updateActualPixmapRect(index, actualPixmapRect);
    }

    if (!d->ratingRect.isNull())
    {
        drawRating(p, index, d->ratingRect, info.rating, isSelected);
    }

    // Draw Color Label rectangle
    drawColorLabelRect(p, option, isSelected, info.colorLabel);

    p->setPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                         : qApp->palette().color(QPalette::Text));

/*
    // If there is ImageHistory present, paint a small icon over the thumbnail to indicate that this is derived image

 if (info.hasImageHistory())
    {
        p->drawPixmap(d->pixmapRect.right()-24, d->pixmapRect.bottom()-24, QIcon::fromTheme(QLatin1String("svn_switch")).pixmap(22));
    }
*/

    if (!d->nameRect.isNull())
    {
        drawName(p, d->nameRect, info.downloadName);
    }

    if (!d->dateRect.isNull())
    {
        drawCreationDate(p, d->dateRect, info.ctime);
    }

    if (!d->sizeRect.isNull())
    {
        drawFileSize(p, d->sizeRect, info.size);
    }

    if (!d->downloadRect.isNull())
    {
        drawDownloadIndicator(p, d->downloadRect, info.downloaded);
    }

    if (!d->lockRect.isNull())
    {
        drawLockIndicator(p, d->lockRect, info.writePermissions);
    }

    if (!d->resolutionRect.isNull())
    {
        QSize dimensions(info.width, info.height);
        drawImageSize(p, d->resolutionRect, dimensions);
    }

    //TODO: Implement grouping in import tool.
/*
    if (!d->groupRect.isNull())
    {
        drawGroupIndicator(p, d->groupRect, info.numberOfGroupedImages(),
                           index.data(ImportFilterModel::GroupIsOpenRole).toBool());
    }
*/

    if (!d->tagRect.isNull())
    {
        QStringList tagsList = AlbumManager::instance()->tagNames(info.tagIds);
        tagsList.sort();
        QString tags         = tagsList.join(QLatin1String(", "));
        drawTags(p, d->tagRect, tags, isSelected);
    }

    if (!d->pickLabelRect.isNull())
    {
        drawPickLabelIcon(p, d->pickLabelRect, info.pickLabel);
    }

    if (d->drawImageFormat)
    {
        QString frm = info.mime;
        drawImageFormat(p, actualPixmapRect, frm);
    }

    if (d->drawCoordinates && info.photoInfo.hasCoordinates)
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

QPixmap ImportDelegate::pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const
{
    QPixmap icon;

    if (!indexes.isEmpty())
    {
        icon = thumbnailPixmap(indexes.first());
    }

    return makeDragPixmap(option, indexes, icon);
}

bool ImportDelegate::acceptsToolTip(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                   QRect* toolTipRect) const
{
    return onActualPixmapRect(pos, visualRect, index, toolTipRect);
}

bool ImportDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                      QRect* activationRect) const
{
    return onActualPixmapRect(pos, visualRect, index, activationRect);
}

bool ImportDelegate::onActualPixmapRect(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
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

void ImportDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ImportDelegate);

    if (d->categoryDrawer)
    {
        d->categoryDrawer->setDefaultViewOptions(option);
    }

    ItemViewImportDelegate::setDefaultViewOptions(option);
}

void ImportDelegate::invalidatePaintingCache()
{
    Q_D(ImportDelegate);

    if (d->categoryDrawer)
    {
        d->categoryDrawer->invalidatePaintingCache();
    }

    ItemViewImportDelegate::invalidatePaintingCache();
}

void ImportDelegate::updateContentWidth()
{
    Q_D(ImportDelegate);
    d->contentWidth = d->thumbSize.size() + 2*d->radius;
}

void ImportDelegate::updateSizeRectsAndPixmaps()
{
    Q_D(ImportDelegate);

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
        //Normally we prepare the pixmaps over the background of the rating rect.
        //If the rating is drawn over the thumbnail, we can only draw over a transparent pixmap.
        prepareRatingPixmaps(!d->ratingOverThumbnail);
    }

    // ---- Drawing related caches ----

    clearCaches();
}

void ImportDelegate::clearCaches()
{
    Q_D(ImportDelegate);
    ItemViewImportDelegate::clearCaches();
    d->actualPixmapRectCache.clear();
}

void ImportDelegate::clearModelDataCaches()
{
    Q_D(ImportDelegate);
    d->actualPixmapRectCache.clear();
}

void ImportDelegate::modelChanged()
{
    Q_D(ImportDelegate);
    clearModelDataCaches();
    setModel(d->currentView ? d->currentView->model() : 0);
}

void ImportDelegate::modelContentsChanged()
{
    clearModelDataCaches();
}

QRect ImportDelegate::actualPixmapRect(const QModelIndex& index) const
{
    Q_D(const ImportDelegate);
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

void ImportDelegate::updateActualPixmapRect(const QModelIndex& index, const QRect& rect)
{
    Q_D(ImportDelegate);
    QRect* const old = d->actualPixmapRectCache.object(index.row());

    if (!old || *old != rect)
    {
        d->actualPixmapRectCache.insert(index.row(), new QRect(rect));
    }
}

int ImportDelegate::calculatethumbSizeToFit(int ws)
{
    Q_D(ImportDelegate);

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
        return (ts2);

    return (ts1);
}

// --- ImportThumbnailDelegate ---------------------------------------

void ImportThumbnailDelegatePrivate::init(ImportThumbnailDelegate* const q)
{
    QObject::connect(ImportSettings::instance(), SIGNAL(setupChanged()),
                     q, SLOT(slotSetupChanged()));
}

// ------------------------------------------------------------------------------------------------

ImportThumbnailDelegate::ImportThumbnailDelegate(ImportCategorizedView* const parent)
    : ImportDelegate(*new ImportThumbnailDelegatePrivate, parent)
{
    Q_D(ImportThumbnailDelegate);
    d->init(this);
}

ImportThumbnailDelegate::~ImportThumbnailDelegate()
{
}

void ImportThumbnailDelegate::setFlow(QListView::Flow flow)
{
    Q_D(ImportThumbnailDelegate);
    d->flow = flow;
}

void ImportThumbnailDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ImportThumbnailDelegate);
    // store before calling parent class
    d->viewSize = option.rect;
    ImportDelegate::setDefaultViewOptions(option);
}

int ImportThumbnailDelegate::maximumSize() const
{
    Q_D(const ImportThumbnailDelegate);

    return ThumbnailSize::maxThumbsSize() + (2*d->radius + 2*d->margin);
}

int ImportThumbnailDelegate::minimumSize() const
{
    Q_D(const ImportThumbnailDelegate);
    return ThumbnailSize::Small + 2*d->radius + 2*d->margin;
}

bool ImportThumbnailDelegate::acceptsActivation(const QPoint& pos, const QRect& visualRect,
                                                const QModelIndex& index, QRect* activationRect) const
{
    // reuse implementation from grandparent
    return ItemViewImportDelegate::acceptsActivation(pos, visualRect, index, activationRect);
}

void ImportThumbnailDelegate::updateContentWidth()
{
    Q_D(ImportThumbnailDelegate);
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

    ImportDelegate::updateContentWidth();
}

int ImportThumbnailDelegate::thumbnailPixmapSize(bool withHighlight, int size)
{
    if (withHighlight && size >= 10)
    {
        return size + 2;
    }

    return size;
}

void ImportThumbnailDelegate::updateRects()
{
    Q_D(ImportThumbnailDelegate);

    d->pixmapRect      = QRect(d->margin, d->margin, d->contentWidth, d->contentWidth);
    d->rect            = QRect(0, 0, d->contentWidth + 2*d->margin, d->contentWidth + 2*d->margin);
    d->drawImageFormat = ImportSettings::instance()->getIconShowImageFormat();
    d->drawCoordinates = ImportSettings::instance()->getIconShowCoordinates();

    const int iconSize = qBound(16, (d->contentWidth + 2*d->margin) / 8 - 2, 48);
    int pos            = iconSize + 2;
    d->downloadRect    = QRect(d->contentWidth - pos, d->pixmapRect.top(), iconSize, iconSize);
    pos += iconSize;
    d->lockRect        = QRect(d->contentWidth - pos, d->pixmapRect.top(), iconSize, iconSize);
    pos += iconSize;
    d->coordinatesRect = QRect(d->contentWidth - pos, d->pixmapRect.top(), iconSize, iconSize);

    if (ImportSettings::instance()->getIconShowRating())
    {
        int top       = d->rect.bottom() - d->margin - d->starPolygonSize.height() - 2;
        d->ratingRect = QRect(d->margin, top, d->contentWidth, d->starPolygonSize.height());
    }

    if (d->flow == QListView::LeftToRight)
    {
        d->gridSize = QSize(d->rect.width() + d->spacing, d->rect.height());
    }
    else
    {
        d->gridSize = QSize(d->rect.width(), d->rect.height() + d->spacing);
    }
}

// --- ImportNormalDelegate -----------------------------------------------------------------------

void ImportNormalDelegatePrivate::init(ImportNormalDelegate* const q, ImportCategorizedView* const parent)
{
    categoryDrawer = new ImportCategoryDrawer(parent);

    QObject::connect(ImportSettings::instance(), SIGNAL(setupChanged()),
                     q, SLOT(slotSetupChanged()));
}

ImportNormalDelegatePrivate::~ImportNormalDelegatePrivate()
{
    delete categoryDrawer;
}

// ------------------------------------------------------------------------------------------------

ImportNormalDelegate::ImportNormalDelegate(ImportCategorizedView* const parent)
    : ImportDelegate(*new ImportNormalDelegatePrivate, parent)
{
    Q_D(ImportNormalDelegate);
    d->init(this, parent);
}

ImportNormalDelegate::ImportNormalDelegate(ImportNormalDelegatePrivate& dd, ImportCategorizedView* const parent)
    : ImportDelegate(dd, parent)
{

    Q_D(ImportNormalDelegate);
    d->init(this, parent);
}

ImportNormalDelegate::~ImportNormalDelegate()
{
}

void ImportNormalDelegate::updateRects()
{
    Q_D(ImportNormalDelegate);

    int y                                      = d->margin;
    d->pixmapRect                              = QRect(d->margin, y, d->contentWidth, d->contentWidth);
    y                                          = d->pixmapRect.bottom();
    d->imageInformationRect                    = QRect(d->margin, y, d->contentWidth, 0);
    const ImportSettings* const importSettings = ImportSettings::instance();
    d->drawImageFormat                         = importSettings->getIconShowImageFormat();
    d->drawCoordinates                         = ImportSettings::instance()->getIconShowCoordinates();
    const int iconSize                         = qBound(16, (d->contentWidth + 2*d->margin) / 8 - 2, 48);

    d->pickLabelRect   = QRect(d->margin, y, iconSize, iconSize);
//  d->groupRect       = QRect(d->contentWidth - iconSize, y, iconSize, iconSize); // TODO

    int pos            = iconSize + 2;
    d->downloadRect    = QRect(d->contentWidth - pos, d->pixmapRect.top(), iconSize, iconSize);
    pos += iconSize;
    d->lockRect        = QRect(d->contentWidth - pos, d->pixmapRect.top(), iconSize, iconSize);
    pos += iconSize;
    d->coordinatesRect = QRect(d->contentWidth - pos, d->pixmapRect.top(), iconSize, iconSize);

    if (importSettings->getIconShowRating())
    {
        d->ratingRect = QRect(d->margin, y, d->contentWidth, d->starPolygonSize.height());
        y             = d->ratingRect.bottom();
    }

    if (importSettings->getIconShowName())
    {
        d->nameRect = QRect(d->margin, y, d->contentWidth-d->margin, d->oneRowRegRect.height());
        y           = d->nameRect.bottom();
    }

    if (importSettings->getIconShowDate())
    {
        d->dateRect = QRect(d->margin, y, d->contentWidth, d->oneRowXtraRect.height());
        y           = d->dateRect.bottom();
    }

    //TODO: Add resolution entry in importSettings.
/*
    if (importSettings->getIconShowResolution())
    {
        d->resolutionRect = QRect(d->margin, y, d->contentWidth, d->oneRowXtraRect.height());
        y                 = d->resolutionRect.bottom() ;
    }
*/

    if (importSettings->getIconShowSize())
    {
        d->sizeRect = QRect(d->margin, y, d->contentWidth, d->oneRowXtraRect.height());
        y           = d->sizeRect.bottom();
    }

    if (importSettings->getIconShowTags())
    {
        d->tagRect = QRect(d->margin, y, d->contentWidth, d->oneRowComRect.height());
        y          = d->tagRect.bottom();
    }

    d->imageInformationRect.setBottom(y);

    d->rect     = QRect(0, 0, d->contentWidth + 2*d->margin, y+d->margin+d->radius);
    d->gridSize = QSize(d->rect.width() + d->spacing, d->rect.height() + d->spacing);
}

} // namespace Digikam
