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
#include <QTimer>

// KDE includes

#include <KLocale>
#include <KIconLoader>
#include <KDebug>
#include <KColorScheme>

// Local includes

#include "imagepropertiesversionsdelegate.h"
#include "imageversionsmodel.h"
#include "thumbnailloadthread.h"
#include "workingwidget.h"

namespace Digikam
{

class ImagePropertiesVersionsDelegate::ImagePropertiesVersionsDelegatePriv
{
public:

    ImagePropertiesVersionsDelegatePriv()
    {
        workingWidget = 0;
        thumbsPainted = 0;
    }

    WorkingWidget* workingWidget;
    int            thumbsPainted;
};

ImagePropertiesVersionsDelegate::ImagePropertiesVersionsDelegate(QObject* parent)
                               : QStyledItemDelegate(parent), d(new ImagePropertiesVersionsDelegatePriv)
{
    d->workingWidget = new WorkingWidget();
    d->thumbsPainted = 0;

    //FIXME: this doesn't work, it needs some better way how to paint it transparentely, ie. get the view color
    // make the workingWidget's background transparent
    //d->workingWidget->setStyleSheet(QString("background-color: rgba(255, 255, 255, 0%);"));

    connect(d->workingWidget, SIGNAL(animationStep()),
            this, SLOT(slotAnimationStep()));
}

ImagePropertiesVersionsDelegate::~ImagePropertiesVersionsDelegate()
{
    delete d->workingWidget;
    delete d;
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

    if(dynamic_cast<const ImageVersionsModel*>(index.model())->paintTree())
    {
        const_cast<QStyleOptionViewItem&>(option).rect.setLeft(option.rect.left() + (index.data(Qt::UserRole).toInt() * 16));
    }

    QRect thumbRect(option.rect.left()+4, option.rect.top()+8, 64, 48);
    painter->setPen(Qt::black);
    painter->drawRect(thumbRect);

    QPixmap thumbnail;
    if(ThumbnailLoadThread::defaultIconViewThread()->find(index.data(Qt::DisplayRole).toString(), thumbnail))
    {
        if(!thumbnail.isNull())
        {
            thumbnail = thumbnail.scaled(64, 48, Qt::KeepAspectRatio);
            const_cast<ImagePropertiesVersionsDelegate*>(this)->d->thumbsPainted++;
        }
        else
        {
            //if the thumbnail pixmap is null, display an error icon instead
            thumbnail = BarIcon("task-reject");
        }
        if(d->thumbsPainted == index.model()->rowCount())
        {
            // the timer can be stopped after last thumbnail is drawn,
            // but it needs to be delayed a little, so that all thumbs
            // have enough time to get painted correctly
            delayedAnimationTimerStop();

        }
    }
    else
    {
        //when the thumbnail is not loaded yet, start the animation
        d->workingWidget->toggleTimer(true);

        connect(d->workingWidget, SIGNAL(animationStep()),
                dynamic_cast<const ImageVersionsModel*>(index.model()), SLOT(slotAnimationStep()));

        thumbnail = QPixmap::grabWidget(d->workingWidget);
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
        painter->drawText(textRect, Qt::AlignVCenter, i18n("This is the original image"));
    }
    else
    {
        painter->drawText(textRect, Qt::AlignVCenter, path.fileName());
    }
    painter->restore();
}

WorkingWidget* ImagePropertiesVersionsDelegate::getWidget() const
{
    return d->workingWidget;
}

void ImagePropertiesVersionsDelegate::slotAnimationStep()
{
    emit throbberUpdated();
}

void ImagePropertiesVersionsDelegate::resetThumbsCounter()
{
    d->thumbsPainted = 0;
}

void ImagePropertiesVersionsDelegate::delayedAnimationTimerStop() const
{
    QTimer::singleShot(1500, d->workingWidget, SLOT(toggleTimer()));
}

} // namespace Digikam
