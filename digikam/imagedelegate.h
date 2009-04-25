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

    /** You must set these options from the view */
    void setThumbnailSize(const ThumbnailSize &thumbSize);
    void setDefaultViewOptions(const QStyleOptionViewItem &option);

    ImageCategoryDrawer *categoryDrawer() const;

    virtual bool acceptsToolTip(const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex & index) const;

protected Q_SLOTS:

    void slotThemeChanged();
    void slotSetupChanged();

protected:

    QRect actualPixmapRect(qlonglong imageid) const;
    void updateActualPixmapRect(qlonglong imageid, const QRect &rect);

    void updateRectsAndPixmaps();

    QPixmap ratingPixmap(int rating, bool selected) const;
    QString dateToString(const QDateTime& datetime) const;
    QString squeezedText(QPainter* p, int width, const QString& text) const;
    QPixmap thumbnailBorderPixmap(const QSize &pixSize) const;
    void clearThumbnailBorderCache();

private:

    ImageDelegatePriv* const d;
};

}

#endif



