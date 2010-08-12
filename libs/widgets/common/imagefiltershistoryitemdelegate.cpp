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

// KDE includes

#include <KIcon>
#include <KLocale>
#include <KDebug>

// Local includes

#include "imagefiltershistoryitemdelegate.h"
#include "dimgfiltermanager.h"

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
        return QSize(230, 32);
    }
    else
    {
        return QSize(230, 18);
    }
}

void ImageFiltersHistoryItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    bool entryDisabled = index.data(Qt::UserRole).toBool();

    if(!index.model()->parent(index).isValid())
    {
        painter->drawRect(option.rect);

        QString iconName = DImgFilterManager::instance()->getFilterIcon(index.data(Qt::DecorationRole).toString());
        if(entryDisabled)
        {
            painter->drawPixmap(option.rect.left()+4, option.rect.top()+5, KIcon(iconName).pixmap(QSize(22,22), QIcon::Disabled));
        }
        else
        {
            painter->drawPixmap(option.rect.left()+4, option.rect.top()+5, KIcon(iconName).pixmap(QSize(22,22)));
        }
    }

    QRect textRect = option.rect;
    textRect.setLeft(textRect.left() + 32);

    if(entryDisabled)
    {
        QFont disabledFont(QApplication::font());
        disabledFont.setItalic(true);
        
        QPalette disabledColor;
        
        painter->setFont(disabledFont);
        painter->setPen(disabledColor.color(QPalette::Disabled, QPalette::WindowText));
    }

    if(!index.data(Qt::DisplayRole).toString().isEmpty())
    {
        painter->drawText(textRect, Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
    }
    else
    {
        //infoIcon.setToolTip(i18n("This filter's name is unknown, so you see only its identifier"));    //FIXME: better string?
        painter->drawPixmap(option.rect.right() - 22, option.rect.top() + 8, KIcon("dialog-information").pixmap(QSize(16,16)));
        painter->drawText(textRect, Qt::AlignVCenter, index.data(Qt::DecorationRole).toString());
    }

    if(entryDisabled)
    {
        //painter->fillRect(option.rect, QColor(200,200,200,160));
    }
    
    painter->restore();
}

} //namespace Digikam
