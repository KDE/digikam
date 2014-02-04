/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : Item delegate for image versions list view
 *
 * Copyright (C) 2010-2011 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "versionsdelegate.moc"

// Qt includes

#include <QApplication>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyle>
#include <QStyleOptionViewItemV4>

// KDE includes

#include <kdebug.h>
#include <kcategorydrawer.h>
#include <kcolorscheme.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpixmapsequence.h>
#include <kdeversion.h>

// Local includes

#include "imagedelegate.h"
#include "imagehistorygraphmodel.h"
#include "imageversionsmodel.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class VersionsDelegate::Private
{
public:

    Private()
        : categoryExtraSpacing(6),
          filterItemExtraSpacing(4),
          animationState(0),
          animation(0),
          categoryDrawer(0),
          thumbnailSize(64),
          thumbsWaitingFor(0),
          inSizeHint(false)
    {
    }

    const int           categoryExtraSpacing;
    const int           filterItemExtraSpacing;

    int                 animationState;
    QPropertyAnimation* animation;
    KPixmapSequence     workingPixmap;
#if KDE_IS_VERSION(4,5,0)
    KCategoryDrawerV3*  categoryDrawer;
#else
    KCategoryDrawerV2*  categoryDrawer;
#endif
    int                 thumbnailSize;

    int                 thumbsWaitingFor;
    bool                inSizeHint;

public:

    inline const QWidget* widget(const QStyleOptionViewItem& option)
    {
        if (const QStyleOptionViewItemV3* v3 = qstyleoption_cast<const QStyleOptionViewItemV3*>(&option))
            return v3->widget;
        return 0;
    }

    inline const QStyle* style(const QStyleOptionViewItem& option)
    {
        const QWidget* w = widget(option);
        return w ? w->style() : QApplication::style();
    }
};

VersionsDelegate::VersionsDelegate(QObject* const parent)
    : QStyledItemDelegate(parent), d(new Private)
{
    d->workingPixmap = KPixmapSequence("process-working", KIconLoader::SizeSmallMedium);

    d->animation     = new QPropertyAnimation(this, "animationState", this);
    d->animation->setStartValue(0);
    d->animation->setEndValue(d->workingPixmap.frameCount() - 1);
    d->animation->setDuration(100 * d->workingPixmap.frameCount());
    d->animation->setLoopCount(-1);

#if KDE_IS_VERSION(4,5,0)
    d->categoryDrawer = new KCategoryDrawerV3(0);
#else
    d->categoryDrawer = new KCategoryDrawerV2;
#endif
}

VersionsDelegate::~VersionsDelegate()
{
    delete d->categoryDrawer;
    delete d;
}

int VersionsDelegate::animationState() const
{
    return d->animationState;
}

void VersionsDelegate::setAnimationState(int animationState)
{
    if (d->animationState == animationState)
        return;
    d->animationState = animationState;
    emit animationStateChanged();
}

void VersionsDelegate::setThumbnailSize(int size) const
{
    d->thumbnailSize = size;
}

int VersionsDelegate::thumbnailSize() const
{
    return d->thumbnailSize;
}

void VersionsDelegate::beginPainting()
{
    d->thumbsWaitingFor = 0;
}

void VersionsDelegate::finishPainting()
{
    //kDebug() << "painting finished" << d->thumbsWaitingFor;
    if (d->thumbsWaitingFor)
    {
        d->animation->start();
    }
    else
    {
        d->animation->stop();
    }
}

QSize VersionsDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(ImageHistoryGraphModel::IsImageItemRole).toBool())
    {
        d->inSizeHint = true;
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        d->inSizeHint = false;
        return size;
    }
    else if (index.data(ImageHistoryGraphModel::IsFilterActionItemRole).toBool())
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size += QSize(0, d->filterItemExtraSpacing);
        return size;
    }
    else if (index.data(ImageHistoryGraphModel::IsCategoryItemRole).toBool())
    {
        int height = d->categoryDrawer->categoryHeight(index, option) + d->categoryExtraSpacing;
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        return size.expandedTo(QSize(0, height));
    }
    else if (index.data(ImageHistoryGraphModel::IsSeparatorItemRole).toBool())
    {
        //int pm = d->style(option)->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, d->widget(option));
        int pm = d->style(option)->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, 0, d->widget(option));
        //int spacing = d->style(option)->pixelMetric(QStyle::PM_LayoutVerticalSpacing, &option);
        return QSize(1, pm);
    }
    else
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

void VersionsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(ImageHistoryGraphModel::IsCategoryItemRole).toBool())
    {
        QStyleOption opt = option;
        opt.rect.adjust(d->categoryExtraSpacing / 2, d->categoryExtraSpacing / 2, - d->categoryExtraSpacing / 2, 0);
        // purpose of sortRole is unclear, give Qt::DisplayRole
        d->categoryDrawer->drawCategory(index, Qt::DisplayRole, opt, painter);
    }
    else if (index.data(ImageHistoryGraphModel::IsSeparatorItemRole).toBool())
    {
        d->style(option)->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &option, painter, d->widget(option));
    }
    else
    {
        return QStyledItemDelegate::paint(painter, option, index);

        /*if (index.data(ImageHistoryGraphModel::IsSubjectImageRole).toBool())
        {
            // draw 1px border
            QPen oldPen = painter->pen();
            QPen pen(option.palette.windowText(), 0);
            painter->setPen(pen);
            painter->drawRect(option.rect);
            painter->setPen(oldPen);
        }*/
    }
}

void VersionsDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    // Don't show the separator-like focus indicator
    option->state &= ~QStyle::State_HasFocus;

    if (!index.data(ImageHistoryGraphModel::IsImageItemRole).toBool())
        return;

    if (index.data(ImageHistoryGraphModel::IsSubjectImageRole).toBool())
    {
        option->font.setWeight(QFont::Bold);
    }

    if (QStyleOptionViewItemV4* v4 = qstyleoption_cast<QStyleOptionViewItemV4*>(option))
    {
        v4->features |= QStyleOptionViewItemV2::HasDecoration;
        if (d->inSizeHint)
        {
            v4->decorationSize = QSize(d->thumbnailSize, d->thumbnailSize);
        }
        else
        {
            QPixmap pix = ImageDelegate::retrieveThumbnailPixmap(index, d->thumbnailSize);
            if (pix.isNull())
            {
                pix = d->workingPixmap.frameAt(d->animationState);
                d->thumbsWaitingFor++;
            }
            v4->icon           = QIcon(pix);
            v4->decorationSize = pix.size();
        }
    }
}

/*
void VersionsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);
    //kDebug() << QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, 0);
    if (dynamic_cast<const ImageVersionsModel*>(index.model())->paintTree())
    {
        const_cast<QStyleOptionViewItem&>(option).rect.setLeft(option.rect.left() + (index.data(Qt::UserRole).toInt() * 16));
    }

    QRect thumbRect(option.rect.left()+4, option.rect.top()+8, 64, 48);
    painter->setPen(Qt::black);
    painter->drawRect(thumbRect);

    QPixmap thumbnail;

    if (ThumbnailLoadThread::defaultIconViewThread()->find(index.data(Qt::DisplayRole).toString(), thumbnail))
    {
        if (!thumbnail.isNull())
        {
            thumbnail = thumbnail.scaled(64, 48, Qt::KeepAspectRatio);
            const_cast<VersionsDelegate*>(this)->d->thumbsPainted++;
        }
        else
        {
            //if the thumbnail pixmap is null, display an error icon instead
            thumbnail = BarIcon("task-reject");
        }

        if (d->thumbsPainted == index.model()->rowCount())
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

    if (index.row() == 0 && index.model()->rowCount() > 1)
    {
        painter->drawText(textRect, Qt::AlignVCenter, i18n("%1 (Original)", path.fileName()));
    }
    else if (index.row() == 0 && index.model()->rowCount() == 1)
    {
        painter->drawText(textRect, Qt::AlignVCenter, i18n("This is the original image"));
    }
    else
    {
        painter->drawText(textRect, Qt::AlignVCenter, path.fileName());
    }

    painter->restore();
}
*/

} // namespace Digikam
