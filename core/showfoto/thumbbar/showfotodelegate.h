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

#ifndef SHOWFOTODELEGATE_H
#define SHOWFOTODELEGATE_H

// Qt includes

#include <QListView>

// Local includes

#include "itemviewshowfotodelegate.h"
#include "showfotothumbnailmodel.h"

namespace ShowFoto
{

class ShowfotoThumbnailBar;
class ShowfotoThumbnailDelegatePrivate;
class ShowfotoNormalDelegatePrivate;
class ShowfotoFilterModel;

class ShowfotoDelegate : public ItemViewShowfotoDelegate
{
    Q_OBJECT

public:
    explicit ShowfotoDelegate(QObject* const parent = 0);
    ~ShowfotoDelegate();

    void setView(ShowfotoThumbnailBar* view);

    QRect actualPixmapRect(const QModelIndex& index) const;
    QRect groupIndicatorRect() const;
    QRect coordinatesIndicatorRect() const;

    int calculatethumbSizeToFit(int ws);

    virtual void setSpacing(int spacing);
    virtual void setDefaultViewOptions(const QStyleOptionViewItem& option);
    virtual bool acceptsToolTip(const QPoint& pos, const QRect& visualRect,
                                const QModelIndex& index, QRect* tooltipRect = 0) const;
    virtual bool acceptsActivation(const QPoint& pos, const QRect& visualRect,
                                   const QModelIndex& index, QRect* activationRect = 0) const;

    virtual QRect pixmapRect() const;
    virtual QRect imageInformationRect() const;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QPixmap pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const;

    /**
     * Retrieve the thumbnail pixmap in given size for the ShowfotoImageModel::ThumbnailRole for
     * the given index from the given index, which must adhere to ShowfotoThumbnailModel semantics.
     */
    static QPixmap retrieveThumbnailPixmap(const QModelIndex& index, int thumbnailSize);

    /** Call this from a paint event, with all indexes expected to be painted immediately,
     *  so that thumbnails become available in order.
     */
    void prepareThumbnails(ShowfotoThumbnailModel* thumbModel, const QList<QModelIndex>& indexes);

public:

    // Declared as public because of use in ShowfotoNormalDelegate class.
    class ShowfotoDelegatePrivate;

protected:

    bool onActualPixmapRect(const QPoint& pos, const QRect& visualRect,
                            const QModelIndex& index, QRect* actualRect) const;
    void updateActualPixmapRect(const QModelIndex& index, const QRect& rect);

    void setModel(QAbstractItemModel* model);

    ShowfotoDelegate(ShowfotoDelegate::ShowfotoDelegatePrivate& dd, QObject* const parent);

    /** Reimplement this to set contentWidth. This is the maximum width of all
     *  content rectangles, typically excluding margins on both sides.
     */
    virtual void updateContentWidth();

    /** In a subclass, you need to implement this method to set up the rects
     *  for drawing. The paint() method operates depending on these rects.
     */
    virtual void updateRects() = 0;

    virtual void clearCaches();

    /** Reimplement to clear caches based on model indexes (hash on row number etc.)
     *  Change signals are listened to this is called whenever such properties become invalid.
     */
    virtual void clearModelDataCaches();

    virtual QPixmap thumbnailPixmap(const QModelIndex& index) const;

    virtual void invalidatePaintingCache();
    virtual void updateSizeRectsAndPixmaps();

protected Q_SLOTS:

    void modelChanged();
    void modelContentsChanged();

private:

    Q_DECLARE_PRIVATE(ShowfotoDelegate)
};

// -------------- ShowfotoThumbnailDelegate ---------------------

class ShowfotoThumbnailDelegate : public ShowfotoDelegate
{
    Q_OBJECT

public:

    explicit ShowfotoThumbnailDelegate(ShowfotoThumbnailBar* const parent);
    ~ShowfotoThumbnailDelegate();

    void setFlow(QListView::Flow flow);

    /** Returns the minimum or maximum viewport size in the limiting dimension,
     *  width or height, depending on current flow.
     */
    int maximumSize() const;
    int minimumSize() const;

    virtual void setDefaultViewOptions(const QStyleOptionViewItem& option);
    virtual bool acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index,
                                   QRect* activationRect) const;

protected:

    virtual void updateContentWidth();
    virtual void updateRects();
    int thumbnailPixmapSize(bool withHighlight, int size);

private:

    Q_DECLARE_PRIVATE(ShowfotoThumbnailDelegate)
};

// ------------------- ShowfotoNormalDelegate ------------------

class ShowfotoNormalDelegate : public ShowfotoDelegate
{
    Q_OBJECT

public:

    explicit ShowfotoNormalDelegate(ShowfotoThumbnailBar* const parent);
    ~ShowfotoNormalDelegate();

protected:

    ShowfotoNormalDelegate(ShowfotoNormalDelegatePrivate& dd, ShowfotoThumbnailBar* const parent);

    virtual void updateRects();

private:

    Q_DECLARE_PRIVATE(ShowfotoNormalDelegate)
};

}// namespace ShowFoto

#endif // SHOWFOTODELEGATE_H
