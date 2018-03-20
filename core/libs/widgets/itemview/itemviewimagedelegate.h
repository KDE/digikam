/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "ditemdelegate.h"
#include "imagedelegateoverlay.h"
#include "thumbnailsize.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemViewImageDelegatePrivate;

class DIGIKAM_EXPORT ItemViewImageDelegate : public DItemDelegate, public ImageDelegateOverlayContainer
{
    Q_OBJECT

public:

    explicit ItemViewImageDelegate(QObject* const parent = 0);
    ~ItemViewImageDelegate();

    ThumbnailSize thumbnailSize() const;
    int spacing()                 const;
    QRect rect()                  const;

    /** Can be used to temporarily disable drawing of the rating.
     *  Call with QModelIndex() afterwards.
     */
    void setRatingEdited(const QModelIndex& index);

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize gridSize()                                                             const;

    // reimplemented from DItemDelegate
    virtual void setThumbnailSize(const ThumbnailSize& thumbSize);
    virtual void setSpacing(int spacing);
    virtual void setDefaultViewOptions(const QStyleOptionViewItem& option);
    virtual bool acceptsToolTip(const QPoint& pos, const QRect& visualRect, const QModelIndex& index, QRect* tooltipRect = 0)       const;
    virtual bool acceptsActivation(const QPoint& pos, const QRect& visualRect, const QModelIndex& index, QRect* activationRect = 0) const;

    /** Returns the area where the pixmap is drawn,
     *  or null if not supported.
     */
    virtual QRect pixmapRect() const;

    /** Returns the area where the image information is drawn,
     *  or null if empty / not supported.
     *  The image information is textual or graphical information,
     *  but not the pixmap. The ratingRect() will e.g. typically
     *  be contained in this area.
     */
    virtual QRect imageInformationRect() const;

    /** Returns the rectangle where the rating is drawn,
     *  or a null rectangle if not supported.
     */
    virtual QRect ratingRect() const;

    virtual void mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index);

protected Q_SLOTS:

    void slotThemeChanged();
    void slotSetupChanged();

    virtual void overlayDestroyed(QObject* o);

Q_SIGNALS:

    void requestNotification(const QModelIndex& index, const QString& message);
    void hideNotification();

protected:

    /// Use the tool methods for painting in subclasses
    QRect drawThumbnail(QPainter* p, const QRect& thumbRect, const QPixmap& background, const QPixmap& thumbnail, bool isGrouped) const;
    void drawRating(QPainter* p, const QModelIndex& index, const QRect& ratingRect, int rating, bool isSelected)                  const;
    void drawSpecialInfo(QPainter* p,const QRect& r, const QString& text)                                                         const;
    void drawName(QPainter* p,const QRect& nameRect, const QString& name)                                                         const;
    void drawTitle(QPainter *p, const QRect& titleRect, const QString& title)                                                     const;
    void drawComments(QPainter* p, const QRect& commentsRect, const QString& comments)                                            const;
    void drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date)                                              const;
    void drawModificationDate(QPainter* p, const QRect& dateRect, const QDateTime& date)                                          const;
    void drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims)                                                     const;
    void drawAspectRatio(QPainter* p, const QRect& dimsRect, const QSize& dims)                                                   const;
    void drawFileSize(QPainter* p, const QRect& r, qlonglong bytes)                                                               const;
    void drawTags(QPainter* p, const QRect& r, const QString& tagsString, bool isSelected)                                        const;
    void drawImageFormat(QPainter* p, const QRect& r, const QString& f, bool drawTop)                                             const;
    void drawColorLabelRect(QPainter* p, const QStyleOptionViewItem& option, bool isSelected, int colorId)                        const;
    void drawPickLabelIcon(QPainter* p, const QRect& r, int pickLabel)                                                            const;
    void drawGroupIndicator(QPainter* p, const QRect& r, int numberOfGroupedImages, bool open)                                    const;
    void drawGeolocationIndicator(QPainter* p, const QRect& r)                                                                    const;
    void drawPanelSideIcon(QPainter* p, bool left, bool right)                                                                    const;
    void drawFocusRect(QPainter* p, const QStyleOptionViewItem& option, bool isSelected)                                          const;
    void drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option)                                                       const;
    void prepareFonts();
    void prepareMetrics(int maxWidth);
    void prepareBackground();
    void prepareRatingPixmaps(bool composeOverBackground = true);

    /** Returns the relevant pixmap from the cached rating pixmaps.
     */
    QPixmap ratingPixmap(int rating, bool selected) const;

    virtual QAbstractItemDelegate* asDelegate();

    // reimplement these in subclasses
    virtual void invalidatePaintingCache();
    virtual void updateSizeRectsAndPixmaps() = 0;

protected:

    ItemViewImageDelegatePrivate* const d_ptr;
    ItemViewImageDelegate(ItemViewImageDelegatePrivate& dd, QObject* const parent);

private:

    Q_DECLARE_PRIVATE(ItemViewImageDelegate)
};

} // namespace Digikam

#endif // ITEMVIEWIMAGEDELEGATE_H
