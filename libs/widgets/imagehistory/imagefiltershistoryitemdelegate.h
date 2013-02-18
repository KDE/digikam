/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-02
 * Description : delegate for custom painting of used filters view
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef IMAGEFILTERSHISTORYITEMDELEGATE_H
#define IMAGEFILTERSHISTORYITEMDELEGATE_H

// Qt includes

#include <QStyledItemDelegate>

namespace Digikam
{

class ImageFiltersHistoryItemDelegate : public QStyledItemDelegate
{
public:

    explicit ImageFiltersHistoryItemDelegate(QObject* const parent = 0);
    ~ImageFiltersHistoryItemDelegate();

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

} //namespace Digikam

#endif // IMAGEFILTERSHISTORYITEMDELEGATE_H
