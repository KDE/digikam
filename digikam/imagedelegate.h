/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEDELEGATE_H
#define IMAGEDELEGATE_H

// Qt includes

#include <QAbstractItemDelegate>

// KDE includes

// Local includes

#include "thumbnailsize.h"

namespace Digikam
{

class ImageCategoryDrawer;
class ImageCategorizedView;
class ImageDelegateOverlay;
class ImageFilterModel;
class ImageModel;
class ImageDelegatePriv;

class ImageDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:

    ImageDelegate(QObject *parent = 0);
    ~ImageDelegate();

    virtual void paint(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex & index) const;
    virtual QSize gridSize() const;
    virtual QPixmap pixmapForDrag(const QStyleOptionViewItem &option, const QList<QModelIndex> &indexes) const;

    /** You must set these options from the view */
    void setThumbnailSize(const ThumbnailSize &thumbSize);
    void setSpacing(int spacing);
    /** Style option with standard values to use for cached rendering.
     *  option.rect shall be the viewport rectangle.
     *  Call on resize, font change.*/
    void setDefaultViewOptions(const QStyleOptionViewItem &option);

    ImageCategoryDrawer *categoryDrawer() const;

    /** These methods take four parameters: The position on viewport, the rect on viewport,
     *  the index, and optionally a parameter into which, if the return value is true,
     *  a rectangle can be written for which the return value will be true as well. */
    virtual bool acceptsToolTip(const QPoint &pos, const QRect &visualRect,
                                const QModelIndex &index, QRect *tooltipRect = 0) const;
    virtual bool acceptsActivation(const QPoint &pos, const QRect &visualRect,
                                   const QModelIndex &index, QRect *activationRect = 0) const;

    QRect rect() const;
    QRect ratingRect() const;
    QRect commentsRect() const;
    QRect tagsRect() const;
    QRect actualPixmapRect(qlonglong imageid) const;

    // to be called by ImageCategorizedView only
    void installOverlay(ImageDelegateOverlay *overlay);
    void removeOverlay(ImageDelegateOverlay *overlay);
    void mouseMoved(QMouseEvent *e, const QRect &visualRect, const QModelIndex &index);

Q_SIGNALS:

    void gridSizeChanged(const QSize &newSize);
    void waitingForThumbnail(const QModelIndex &index) const;

    void visualChange();

protected Q_SLOTS:

    void slotThemeChanged();
    void slotSetupChanged();

protected:

    bool onActualPixmapRect(const QPoint &pos, const QRect &visualRect,
                            const QModelIndex & index, QRect *actualRect) const;
    void updateActualPixmapRect(qlonglong imageid, const QRect &rect);

    void invalidatePaintingCache();
    void updateSizeRectsAndPixmaps();

    QPixmap ratingPixmap(int rating, bool selected) const;
    QString dateToString(const QDateTime& datetime) const;
    QString squeezedText(QPainter* p, int width, const QString& text) const;
    QPixmap thumbnailBorderPixmap(const QSize &pixSize) const;

private:

    ImageDelegatePriv* const d;
};

} // namespace Digikam

#endif /* IMAGEDELEGATE_H */
