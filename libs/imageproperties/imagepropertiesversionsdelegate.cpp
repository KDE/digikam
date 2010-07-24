/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : Item delegate for image versions list view
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
#include <QApplication>

// KDE includes

#include <KLocale>

// Local includes

#include "imagepropertiesversionsdelegate.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

ImagePropertiesVersionsDelegate::ImagePropertiesVersionsDelegate(QObject* parent)
                               : QStyledItemDelegate(parent)
{
}

ImagePropertiesVersionsDelegate::~ImagePropertiesVersionsDelegate()
{
}

QSize ImagePropertiesVersionsDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(230, 64);
}

void ImagePropertiesVersionsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QPixmap thumbnail;
    if(ThumbnailLoadThread::defaultIconViewThread()->find(index.data(Qt::DisplayRole).toString(), thumbnail))
    {
        painter->drawPixmap(option.rect.left()+4, option.rect.top()+3, thumbnail.scaledToHeight(48));
    }

    QRect textRect = option.rect;
    textRect.setLeft(textRect.left() + 72);
    KUrl path(index.data(Qt::DisplayRole).toString());

    if(index.row() == 0 && index.model()->rowCount() > 1)
    {
        painter->drawText(textRect, Qt::AlignVCenter, i18n("%1 (Original)").arg(path.fileName()));
    }
    else
    {
        painter->drawText(textRect, Qt::AlignVCenter, path.fileName());
    }
    painter->restore();
}

} // namespace Digikam
