/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : Qt item view for images - common delegate code
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DITEMDELEGATE_H
#define DITEMDELEGATE_H

// Qt includes

#include <QAbstractItemDelegate>

// Local includes

#include "digikam_export.h"
#include "thumbnailsize.h"

namespace Digikam
{

class ItemViewCategorized;

class DIGIKAM_EXPORT DItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:

    explicit DItemDelegate(QObject* const parent = 0);
    ~DItemDelegate();

    /// Returns the gridsize to be set by the view. It's sizeHint plus spacing.
    virtual QSize gridSize() const = 0;
    virtual QPixmap pixmapForDrag(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes) const = 0;

    /** You must set these options from the view */
    virtual void setThumbnailSize(const ThumbnailSize& thumbSize) = 0;
    virtual void setSpacing(int spacing) = 0;
    /** Style option with standard values to use for cached rendering.
     *  option.rect shall be the viewport rectangle.
     *  Call on resize, font change.*/
    virtual void setDefaultViewOptions(const QStyleOptionViewItem& option) = 0;

    /** These methods take four parameters: The position on viewport, the rect on viewport,
     *  the index, and optionally a parameter into which, if the return value is true,
     *  a rectangle can be written for which the return value will be true as well. */
    virtual bool acceptsToolTip(const QPoint& pos, const QRect& visualRect,
                                const QModelIndex& index, QRect* tooltipRect = 0) const = 0;
    virtual bool acceptsActivation(const QPoint& pos, const QRect& visualRect,
                                   const QModelIndex& index, QRect* activationRect = 0) const = 0;

    // to be called by ItemViewCategorized only
    virtual void mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index) = 0;

    static QString squeezedText(const QFontMetrics& fm, int width, const QString& text);
    static QString dateToString(const QDateTime& datetime);
    static QPixmap makeDragPixmap(const QStyleOptionViewItem& option, const QList<QModelIndex>& indexes,
                                  const QPixmap& suggestedPixmap = QPixmap());

Q_SIGNALS:

    void gridSizeChanged(const QSize& newSize);
    void visualChange();

protected:

    virtual void clearCaches();

    QString squeezedTextCached(QPainter* const p, int width, const QString& text) const;
    QPixmap thumbnailBorderPixmap(const QSize& pixSize, bool isGrouped = false) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGEDELEGATE_H
