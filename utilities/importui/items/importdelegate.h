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

#ifndef IMPORTDELEGATE_H
#define IMPORTDELEGATE_H

#include <QtGlobal>
#include <QListView>

// Local includes

#include "itemviewimportdelegate.h"
#include "importthumbnailmodel.h"
#include "importcategorydrawer.h"

namespace Digikam
{

class ImportCategorizedView;
class ImportThumbnailDelegatePrivate;
class ImportNormalDelegatePrivate;

class ImportDelegate : public ItemViewImportDelegate
{
    Q_OBJECT

public:

    explicit ImportDelegate(QObject* const parent = 0);
    ~ImportDelegate();

    void setView(ImportCategorizedView* view);

    ImportCategoryDrawer* categoryDrawer() const;

    //QRect commentsRect() const;
    QRect tagsRect() const;
    QRect actualPixmapRect(const QModelIndex& index) const;
    QRect groupIndicatorRect() const;
    QRect downloadIndicatorRect() const;
    QRect lockIndicatorRect() const;
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
     * Retrieve the thumbnail pixmap in given size for the ImportImageModel::ThumbnailRole for
     * the given index from the given index, which must adhere to ImportThumbnailModel semantics.
     */
    static QPixmap retrieveThumbnailPixmap(const QModelIndex& index, int thumbnailSize);

public:

    // Declared as public because of use in ImportNormalDelegate class.
    class ImportDelegatePrivate;

protected:

    bool onActualPixmapRect(const QPoint& pos, const QRect& visualRect,
                            const QModelIndex& index, QRect* actualRect) const;
    void updateActualPixmapRect(const QModelIndex& index, const QRect& rect);

    void setModel(QAbstractItemModel* model);

    ImportDelegate(ImportDelegate::ImportDelegatePrivate& dd, QObject* const parent);

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

    Q_DECLARE_PRIVATE(ImportDelegate)
};

// ------ ImportThumbnailDelegate ----------------------------------------

class ImportThumbnailDelegate : public ImportDelegate
{
    Q_OBJECT

public:

    explicit ImportThumbnailDelegate(ImportCategorizedView* const parent);
    ~ImportThumbnailDelegate();

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

    Q_DECLARE_PRIVATE(ImportThumbnailDelegate)
};

// ------ ImportNormalDelegate ----------------------------------------

class ImportNormalDelegate : public ImportDelegate
{
    Q_OBJECT

public:

    explicit ImportNormalDelegate(ImportCategorizedView* const parent);
    ~ImportNormalDelegate();

protected:

    ImportNormalDelegate(ImportNormalDelegatePrivate& dd, ImportCategorizedView* const parent);

    virtual void updateRects();

private:

    Q_DECLARE_PRIVATE(ImportNormalDelegate)
};

} // namespace Digikam

#endif // IMPORTDELEGATE_H
