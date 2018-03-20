/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-08-21
 * Description : Overlays for the import interface
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importoverlays.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "importcategorizedview.h"
#include "importdelegate.h"
#include "camiteminfo.h"
#include "ratingwidget.h"

namespace Digikam
{

ImportOverlayWidget::ImportOverlayWidget(QWidget* const parent)
    : QAbstractButton(parent)
{
}

void ImportOverlayWidget::paintEvent(QPaintEvent*)
{
}

// -- Coordinates Overlay ------------------------------------------------------------------

ImportCoordinatesOverlay::ImportCoordinatesOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

ImportOverlayWidget* ImportCoordinatesOverlay::buttonWidget() const
{
    return static_cast<ImportOverlayWidget*>(m_widget);
}

QWidget* ImportCoordinatesOverlay::createWidget()
{
    QAbstractButton* const button = new ImportOverlayWidget(parentWidget());
    //button->setCursor(Qt::PointingHandCursor);
    return button;
}

void ImportCoordinatesOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);
}

void ImportCoordinatesOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void ImportCoordinatesOverlay::updatePosition()
{
    if (!m_index.isValid())
    {
        return;
    }

    QRect rect       = static_cast<ImportDelegate*>(delegate())->coordinatesIndicatorRect();
    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

bool ImportCoordinatesOverlay::checkIndex(const QModelIndex& index) const
{
    CamItemInfo info = ImportImageModel::retrieveCamItemInfo(index);
    QRect rect       = static_cast<ImportDelegate*>(delegate())->coordinatesIndicatorRect();

    if (!rect.isNull() && info.photoInfo.hasCoordinates)
    {
        m_widget->setToolTip(i18nc("@info:tooltip", "This item has geolocation information."));
        return true;
    }

    // If info.photoInfo.hasCoordinates = false, no need to show a tooltip, because there is no icon over thumbnail.

    return false;
}

void ImportCoordinatesOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);
    m_index = index;
    updatePosition();
}

// -- Lock Overlay ------------------------------------------------------------------

ImportLockOverlay::ImportLockOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

ImportOverlayWidget* ImportLockOverlay::buttonWidget() const
{
    return static_cast<ImportOverlayWidget*>(m_widget);
}

QWidget* ImportLockOverlay::createWidget()
{
    QAbstractButton* const button = new ImportOverlayWidget(parentWidget());
    //button->setCursor(Qt::PointingHandCursor);
    return button;
}

void ImportLockOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);
}

void ImportLockOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void ImportLockOverlay::updatePosition()
{
    if (!m_index.isValid())
    {
        return;
    }

    QRect rect       = static_cast<ImportDelegate*>(delegate())->lockIndicatorRect();
    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

bool ImportLockOverlay::checkIndex(const QModelIndex& index) const
{
    CamItemInfo info = ImportImageModel::retrieveCamItemInfo(index);

    if (info.writePermissions == 0)
    {
        m_widget->setToolTip(i18nc("@info:tooltip", "This item is locked."));
        return true;
    }

    // If info.writePermissions = 1, no need to show a tooltip, because there is no icon over thumbnail.

    return false;
}

void ImportLockOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);
    m_index = index;
    updatePosition();
}

// -- Download Overlay ------------------------------------------------------------------

ImportDownloadOverlay::ImportDownloadOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

ImportOverlayWidget* ImportDownloadOverlay::buttonWidget() const
{
    return static_cast<ImportOverlayWidget*>(m_widget);
}

QWidget* ImportDownloadOverlay::createWidget()
{
    QAbstractButton* const button = new ImportOverlayWidget(parentWidget());
    //button->setCursor(Qt::PointingHandCursor);
    return button;
}

void ImportDownloadOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);
}

void ImportDownloadOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void ImportDownloadOverlay::updatePosition()
{
    if (!m_index.isValid())
    {
        return;
    }

    QRect rect       = static_cast<ImportDelegate*>(delegate())->downloadIndicatorRect();
    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

bool ImportDownloadOverlay::checkIndex(const QModelIndex& index) const
{
    CamItemInfo info = ImportImageModel::retrieveCamItemInfo(index);

    if (info.downloaded == CamItemInfo::DownloadUnknown)
    {
        m_widget->setToolTip(i18nc("@info:tooltip", "This item has an unknown download status"));
        return true;
    }

    if (info.downloaded == CamItemInfo::DownloadedNo) // TODO: CamItemInfo::NewPicture
    {
        m_widget->setToolTip(i18nc("@info:tooltip", "This item has never been downloaded"));
        return true;
    }

    if (info.downloaded == CamItemInfo::DownloadedYes)
    {
        m_widget->setToolTip(i18nc("@info:tooltip", "This item has already been downloaded"));
        return true;
    }

    return false;
}

void ImportDownloadOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);
    m_index = index;
    updatePosition();
}

// -- Rating Overlay ------------------------------------------------------------------

ImportRatingOverlay::ImportRatingOverlay(QObject* const parent)
    : AbstractWidgetDelegateOverlay(parent)
{
}

RatingWidget* ImportRatingOverlay::ratingWidget() const
{
    return static_cast<RatingWidget*>(m_widget);
}

QWidget* ImportRatingOverlay::createWidget()
{
    RatingWidget* const w = new RatingWidget(parentWidget());
    w->setFading(true);
    w->setTracking(false);
    return w;
}

void ImportRatingOverlay::setActive(bool active)
{
    AbstractWidgetDelegateOverlay::setActive(active);

    if (active)
    {
        connect(ratingWidget(), SIGNAL(signalRatingChanged(int)),
                this, SLOT(slotRatingChanged(int)));

        if (view()->model())
        {
            connect(view()->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                    this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));
        }
    }
    else
    {
        // widget is deleted

        if (view() && view()->model())
        {
            disconnect(view()->model(), 0, this, 0);
        }
    }
}

void ImportRatingOverlay::visualChange()
{
    if (m_widget && m_widget->isVisible())
    {
        updatePosition();
    }
}

void ImportRatingOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(m_index);
}

void ImportRatingOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

void ImportRatingOverlay::hide()
{
    delegate()->setRatingEdited(QModelIndex());
    AbstractWidgetDelegateOverlay::hide();
}

void ImportRatingOverlay::updatePosition()
{
    if (!m_index.isValid() || !m_widget)
    {
        return;
    }

    QRect rect = delegate()->ratingRect();

    if (rect.width() > ratingWidget()->maximumVisibleWidth())
    {
        int offset = (rect.width() - ratingWidget()->maximumVisibleWidth()) / 2;
        rect.adjust(offset, 0, -offset, 0);
    }

    QRect visualRect = m_view->visualRect(m_index);
    rect.translate(visualRect.topLeft());

    m_widget->setFixedSize(rect.width() + 1, rect.height() + 1);
    m_widget->move(rect.topLeft());
}

void ImportRatingOverlay::updateRating()
{
    if (!m_index.isValid() || !m_widget)
    {
        return;
    }

    ImportImageModel* const model = m_index.data(ImportImageModel::ImportImageModelPointerRole).value<ImportImageModel*>();
    ratingWidget()->setRating(model->camItemInfoRef(m_index).rating);
}

void ImportRatingOverlay::slotRatingChanged(int rating)
{
    if (m_widget && m_widget->isVisible() && m_index.isValid())
    {
        emit ratingEdited(affectedIndexes(m_index), rating);
    }
}

void ImportRatingOverlay::slotEntered(const QModelIndex& index)
{
    AbstractWidgetDelegateOverlay::slotEntered(index);

    // see bug 228810, this is a small workaround
    if (m_widget && m_widget->isVisible() && m_index.isValid() && index == m_index)
    {
        ratingWidget()->setVisibleImmediately();
    }

    m_index = index;

    updatePosition();
    updateRating();

    delegate()->setRatingEdited(m_index);
    view()->update(m_index);
}

void ImportRatingOverlay::slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (m_widget && m_widget->isVisible() && QItemSelectionRange(topLeft, bottomRight).contains(m_index))
    {
        updateRating();
    }
}

// -- Rotate Overlay ----------------------------------------------------------------

ImportRotateOverlayButton::ImportRotateOverlayButton(ImportRotateOverlayDirection dir, QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView),
      m_direction(dir)
{
}

QSize ImportRotateOverlayButton::sizeHint() const
{
    return QSize(32, 32);
}

QIcon ImportRotateOverlayButton::icon()
{
    if (m_direction == ImportRotateOverlayLeft)
    {
        return QIcon::fromTheme(QLatin1String("object-rotate-left"));
    }
    else
    {
        return QIcon::fromTheme(QLatin1String("object-rotate-right"));
    }
}

void ImportRotateOverlayButton::updateToolTip()
{
    if (m_direction == ImportRotateOverlayLeft)
    {
        setToolTip(i18nc("@info:tooltip", "Rotate Left"));
    }
    else
    {
        setToolTip(i18nc("@info:tooltip", "Rotate Right"));
    }
}

// --------------------------------------------------------------------

ImportRotateOverlay::ImportRotateOverlay(ImportRotateOverlayDirection dir, QObject* const parent)
    : HoverButtonDelegateOverlay(parent),
      m_direction(dir)
{
}

void ImportRotateOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }
}

ItemViewHoverButton* ImportRotateOverlay::createButton()
{
    return new ImportRotateOverlayButton(m_direction, view());
}

void ImportRotateOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int size   = qBound(16, rect.width() / 8 - 2, 48);
    const int gap    = 5;
    const int x      = rect.right() - 2*gap - (isLeft() ? size*5 + 2 : size*4 + 2);
    const int y      = rect.top() + gap;
    button()->resize(size, size);
    button()->move(QPoint(x, y));
}

void ImportRotateOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        emit signalRotate(affectedIndexes(index));
    }
}

bool ImportRotateOverlay::checkIndex(const QModelIndex& index) const
{
    CamItemInfo info = ImportImageModel::retrieveCamItemInfo(index);
    return (info.mime.contains(QLatin1String("image/")));
}

void ImportRotateOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(button()->index());
}

void ImportRotateOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

} // namespace Digikam
