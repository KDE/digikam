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
#include "imagepropertiesversionsdelegate.moc"

// Qt includes

#include <QPainter>
#include <QApplication>

// KDE includes

#include <KLocale>
#include <KIconLoader>
#include <KDebug>

// Local includes

#include "imagepropertiesversionsdelegate.h"
#include "imageversionsmodel.h"
#include "thumbnailloadthread.h"
#include "workingwidget.h"

namespace Digikam
{

ImagePropertiesVersionsDelegate::ImagePropertiesVersionsDelegate(QObject* parent)
                               : QStyledItemDelegate(parent)
{
    m_workingWidget = new WorkingWidget();
    //TODO: get system color for views as a background
    m_workingWidget->setStyleSheet("background-color: rgb(255, 255, 255);");

    connect(m_workingWidget, SIGNAL(animationStep()),
            this, SLOT(slotAnimationStep()));
}

ImagePropertiesVersionsDelegate::~ImagePropertiesVersionsDelegate()
{
    delete m_workingWidget;
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

    QRect thumbRect(option.rect.left()+4, option.rect.top()+8, 64, 48);
    painter->setPen(Qt::black);
    painter->drawRect(thumbRect);

    QPixmap thumbnail;
    if(ThumbnailLoadThread::defaultIconViewThread()->find(index.data(Qt::DisplayRole).toString(), thumbnail))
    {
        if(!thumbnail.isNull())
        {
            thumbnail = thumbnail.scaled(64, 48, Qt::KeepAspectRatio);
        }
        else
        {
            //if the thumbnail pixmap is null, display an error icon instead
            thumbnail = BarIcon("task-reject");
        }
        if(index.row() == index.model()->rowCount()-1)
        {
            //the animation gets disconnected after the last thumbnail is drawn
            //this way it won't uselessly emit signals that model has updated data (which has not)
            disconnect(m_workingWidget, SIGNAL(animationStep()), 0, 0);
        }
    }
    else
    {
        //when the thumbnail is not loaded yet, start the animation
        connect(m_workingWidget, SIGNAL(animationStep()),
                dynamic_cast<const ImageVersionsModel*>(index.model()), SLOT(slotAnimationStep()));
        thumbnail = QPixmap::grabWidget(m_workingWidget);
    }
    
    painter->drawPixmap(thumbRect.left()+(32-(int)(thumbnail.width()/2)), thumbRect.top()+(24-(int)(thumbnail.height()/2)), thumbnail);

    QRect textRect = option.rect;
    textRect.setLeft(textRect.left() + 72);
    KUrl path(index.data(Qt::DisplayRole).toString());

    if(index.row() == 0 && index.model()->rowCount() > 1)
    {
        painter->drawText(textRect, Qt::AlignVCenter, i18n("%1 (Original)").arg(path.fileName()));
    }
    else if(index.row() == 0 && index.model()->rowCount() == 1)
    {
        painter->drawText(textRect, Qt::AlignVCenter, i18n("This is original image"));
    }
    else
    {
        painter->drawText(textRect, Qt::AlignVCenter, path.fileName());
    }
    painter->restore();
}

WorkingWidget* ImagePropertiesVersionsDelegate::getWidget()
{
    return m_workingWidget;
}

void ImagePropertiesVersionsDelegate::slotAnimationStep()
{
    emit throbberUpdated();
}

} // namespace Digikam