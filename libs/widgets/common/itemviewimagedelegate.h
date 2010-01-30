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

#ifndef ITEMVIEWIMAGEDELEGATE_H
#define ITEMVIEWIMAGEDELEGATE_H

// Qt includes

// Local includes

#include "ditemdelegate.h"
#include "thumbnailsize.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageCategoryDrawer;
class ImageCategorizedView;
class ImageDelegateOverlay;
class ImageFilterModel;
class ImageModel;
class ItemViewImageDelegatePrivate;

class DIGIKAM_EXPORT ItemViewImageDelegate : public DItemDelegate
{
    Q_OBJECT

public:

    ItemViewImageDelegate(DCategorizedView *parent);
    ~ItemViewImageDelegate();

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex & index) const;
    virtual QSize gridSize() const;

    /** You must set these options from the view */
    virtual void setThumbnailSize(const ThumbnailSize& thumbSize);
    virtual void setSpacing(int spacing);
    /** Style option with standard values to use for cached rendering.
     *  option.rect shall be the viewport rectangle.
     *  Call on resize, font change.*/
    virtual void setDefaultViewOptions(const QStyleOptionViewItem& option);

    /** These methods take four parameters: The position on viewport, the rect on viewport,
     *  the index, and optionally a parameter into which, if the return value is true,
     *  a rectangle can be written for which the return value will be true as well. */
    virtual bool acceptsToolTip(const QPoint& pos, const QRect& visualRect,
                                const QModelIndex& index, QRect *tooltipRect = 0) const;
    virtual bool acceptsActivation(const QPoint& pos, const QRect& visualRect,
                                   const QModelIndex& index, QRect *activationRect = 0) const;

    QRect rect() const;

    /** Can be used to temporarily disable drawing of the rating.
     *  Call with QModelIndex() afterwards. */
    void setRatingEdited(const QModelIndex &index);

    // to be called by ImageCategorizedView only
    void installOverlay(ImageDelegateOverlay *overlay);
    void removeOverlay(ImageDelegateOverlay *overlay);
    void removeAllOverlays();
    void mouseMoved(QMouseEvent *e, const QRect& visualRect, const QModelIndex& index);

protected Q_SLOTS:

    void slotThemeChanged();
    void slotSetupChanged();

protected:

    virtual void invalidatePaintingCache();
    virtual void updateSizeRectsAndPixmaps() = 0;

    QPixmap ratingPixmap(int rating, bool selected) const;

    ItemViewImageDelegatePrivate *const d_ptr;
    ItemViewImageDelegate(ItemViewImageDelegatePrivate &dd, DCategorizedView *parent);
    
private:

    Q_DECLARE_PRIVATE(ItemViewImageDelegate)
};

} // namespace Digikam

#endif /* ITEMVIEWIMAGEDELEGATE_H */
