/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-29
 * Description : a tool to export images to Debian Screenshots
 *
 * Copyright (C) 2010 by Pau Garcia i Quiles <pgquiles at elpauer dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dspackagedelegate.h"

// Qt includes

#include <QPainter>

namespace GenericDigikamDebianScreenshotsPlugin
{

DSPackageDelegate::DSPackageDelegate(QObject* const parent)
    : QStyledItemDelegate(parent)
{
}

void DSPackageDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect( option.rect, option.palette.color(QPalette::Highlight) );
    }

    QString pkgName        = index.data(Qt::DisplayRole).toString();
    QString pkgDescription = index.model()->index(index.row(), 1, QModelIndex()).data(Qt::DisplayRole).toString();
    QRect r                = option.rect.adjusted(2, 2, -2, -2);
    painter->save();
    painter->drawText(r.left(), r.top(), r.width(), r.height()/2, Qt::AlignVCenter|Qt::AlignLeft|Qt::TextWordWrap, pkgName);
    painter->drawText(r.left() + 20, r.top() + r.height()/2, r.width(), r.height()/2, Qt::AlignVCenter|Qt::AlignLeft|Qt::TextWordWrap, pkgDescription);
    painter->restore();
}

QSize DSPackageDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return 2.2*QStyledItemDelegate::sizeHint(option, index);
}

} // namespace GenericDigikamDebianScreenshotsPlugin
