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

// Qt includes

#include <QPainter>
#include <QBrush>
#include <QGradient>
#include <QLinearGradient>
#include <QApplication>

// Local includes

#include "imagefiltershistoryitemdelegate.h"

namespace Digikam
{

ImageFiltersHistoryItemDelegate::ImageFiltersHistoryItemDelegate(QObject* parent)
                               : QAbstractItemDelegate(parent)
{
}

ImageFiltersHistoryItemDelegate::~ImageFiltersHistoryItemDelegate()
{
}

QSize ImageFiltersHistoryItemDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
    if(!index.model()->parent(index).isValid())
    {
        return QSize(250, 30);
    }
    else
    {
        return QSize(250, 18);
    }
}

void ImageFiltersHistoryItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QLinearGradient linearGradient((int)(option.rect.width()/2), option.rect.y(), (int)(option.rect.width()/2), option.rect.height());
    linearGradient.setColorAt(0.0, Qt::white);
    linearGradient.setColorAt(0.2, Qt::green);
    linearGradient.setColorAt(1.0, Qt::black);

    if(!index.model()->parent(index).isValid())
    {
        //painter->
        //painter->fillRect(option.rect, QBrush(QColor(235, 235, 240)));
        painter->drawRect(option.rect);
    }

    QRect textRect = option.rect;
    textRect.setLeft(textRect.left() + 8);

    painter->drawText(textRect, Qt::AlignVCenter, index.data(Qt::DisplayRole).toString() );
    painter->restore();
}

} //namespace Digikam
