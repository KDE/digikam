/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "ratingsearchutilities.h"

// Qt includes

#include <QAbstractListModel>
#include <QItemDelegate>
#include <QLineEdit>
#include <QLinearGradient>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "ratingwidget.h"

namespace Digikam
{

RatingStarDrawer::RatingStarDrawer()
{
    m_starPolygon     = RatingWidget::starPolygon();
    m_starPolygonSize = QSize(15, 15);
}

QRect RatingStarDrawer::drawStarPolygons(QPainter* painter, int numberOfStars) const
{
    QRect    drawnRect(0, 0, 0, 0);
    QPolygon polygon(m_starPolygon);

    if (numberOfStars)
    {
        drawnRect.adjust(0, 0, 0, m_starPolygonSize.height());
    }

    for (int i = 0; i < numberOfStars; ++i)
    {
        painter->drawPolygon(polygon, Qt::WindingFill);
        polygon.translate(m_starPolygonSize.width(), 0);
        drawnRect.adjust(0, 0, m_starPolygonSize.width(), 0);
    }

    return drawnRect;
}

// -------------------------------------------------------------------------

RatingComboBoxDelegate::RatingComboBoxDelegate(QObject* const parent)
    : QItemDelegate(parent)
{
}

QSize RatingComboBoxDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QVariant value = index.data(Qt::DisplayRole);

    if (value.type() == QVariant::Int)
    {
        return QSize(RatingMax * (m_starPolygonSize.width() + 1), m_starPolygonSize.height());
    }
    else
    {
        return QItemDelegate::sizeHint(option, index);
    }
}

void RatingComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
{
    QVariant value  = index.data(Qt::DisplayRole);
    bool selectable = index.flags() & Qt::ItemIsSelectable;

    if (value.type() == QVariant::Int)
    {
        painter->save();
        drawBackground(painter, option, index);
        drawDisplay(painter, option, option.rect, QString());

        // our custom painting
        drawRating(painter, option.rect, value.toInt(), selectable);

        drawFocus(painter, option, option.rect);
        painter->restore();
    }
    else
    {
        return QItemDelegate::paint(painter, option, index);
    }
}

void RatingComboBoxDelegate::drawRating(QPainter* painter, const QRect& rect, int rating, bool selectable) const
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    //pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(qApp->palette().color(QPalette::Text));

    if (!selectable)
    {
        painter->setOpacity(.1);
    }

    painter->setBrush(qApp->palette().color(QPalette::Link));
    // move painter while drawing polygons
    painter->translate(rect.topLeft());
    QRect drawRect = drawStarPolygons(painter, rating);
    painter->translate(drawRect.topRight());

    painter->setBrush(QBrush());
    drawStarPolygons(painter, RatingMax - rating);

    painter->restore();
}

// -------------------------------------------------------------------------

RatingComboBoxModel::RatingComboBoxModel(QObject* const parent)
    : QAbstractListModel(parent)
{
    for (int value = RatingComboBox::Null; value <= RatingComboBox::Rating5; ++value)
    {
        m_entries << (RatingComboBox::RatingValue)value;
    }
}

int RatingComboBoxModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_entries.size();
}

QVariant RatingComboBoxModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        RatingComboBox::RatingValue value = (RatingComboBox::RatingValue)index.internalId();

        if (role == Qt::DisplayRole)
        {
            return ratingValueToDisplay(value);
        }
        else if (role == RatingRole)
        {
            return (int)value;
        }
    }

    return QVariant();
}

QVariant RatingComboBoxModel::ratingValueToDisplay(RatingComboBox::RatingValue value) const
{
    switch (value)
    {
        case RatingComboBox::Null:
            return i18n("(No Value Selected)");

        case RatingComboBox::NoRating:
            return i18n("No Rating assigned");

        case RatingComboBox::Rating0:
        case RatingComboBox::Rating1:
        case RatingComboBox::Rating2:
        case RatingComboBox::Rating3:
        case RatingComboBox::Rating4:
        case RatingComboBox::Rating5:
            return (int)value;
    }

    return QVariant();
}

QModelIndex RatingComboBoxModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() || column != 0 || row >= m_entries.size())
    {
        return QModelIndex();
    }

    // third argument: RatingValue as internal data
    return createIndex(row, column, m_entries.at(row));
}

QModelIndex RatingComboBoxModel::indexForRatingValue(RatingComboBox::RatingValue value) const
{
    int row = m_entries.indexOf(value);

    if (row != -1)
    {
        return createIndex(row, 0, value);
    }

    return QModelIndex();
}

// -------------------------------------------------------------------------

RatingComboBoxWidget::RatingComboBoxWidget(QWidget* const parent)
    : RatingWidget(parent)
{
    m_value = RatingComboBox::Null;

    // generate paint event on mouse enter/leave
    setAttribute(Qt::WA_Hover);
    // set lineedit-like background, also for cached pixmaps
    setBackgroundRole(QPalette::Base);
    regeneratePixmaps();

    connect(this, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotRatingChanged(int)));
}

RatingComboBox::RatingValue RatingComboBoxWidget::ratingValue() const
{
    return m_value;
}

void RatingComboBoxWidget::setRatingValue(RatingComboBox::RatingValue value)
{
    if (m_value == value)
    {
        return;
    }

    m_value = value;

    // sync with base class
    blockSignals(true);

    if (m_value >= RatingComboBox::Rating0)
    {
        setRating(value);
    }
    else
    {
        setRating(0);
    }

    blockSignals(false);

    update();
    emit ratingValueChanged(m_value);
}

void RatingComboBoxWidget::slotRatingChanged(int rating)
{
    RatingComboBox::RatingValue newValue = (RatingComboBox::RatingValue)rating;

    if (m_value != newValue)
    {
        m_value = newValue;
        emit ratingValueChanged(m_value);
    }
}

void RatingComboBoxWidget::paintEvent(QPaintEvent* e)
{
    if (m_value >= RatingComboBox::Rating0)
    {
        //qCDebug(DIGIKAM_GENERAL_LOG) << "m_value" << m_value << "defaulting paint to parent" << this;
        RatingWidget::paintEvent(e);
    }
    else if (m_value == RatingComboBox::NoRating)
    {
        QPainter p(this);

        QPixmap pix = starPixmap();
        int width = pix.width();
        p.drawPixmap(0, 0, pix);
        // draw red cross
        p.setPen(Qt::red);
        p.drawLine(0, 0, width, width);
        p.drawLine(0, width, width, 0);
    }
    else if (m_value == RatingComboBox::Null)
    {
        QPainter p(this);

        if (underMouse() && isEnabled())
        {
            QPixmap pix = starPixmap();
            int x = 0;

            for (int i = 0; i < RatingMax; ++i)
            {
                p.drawPixmap(x, 0, pix);
                x += pix.width();
            }
        }
        else
        {
            p.setRenderHint(QPainter::Antialiasing, true);
            //pen.setJoinStyle(Qt::MiterJoin);

            QColor foreground = palette().color(QPalette::Active, QPalette::Foreground);
            QColor background = palette().color(QPalette::Active, QPalette::Background);
            foreground.setAlphaF(foreground.alphaF() * 0.5);
            background.setAlphaF(background.alphaF() * 0.5);
            QColor foregroundEnd(foreground), backgroundEnd(background);
            foregroundEnd.setAlphaF(0);
            backgroundEnd.setAlphaF(0);

            QLinearGradient grad(QPointF(0, (double)rect().height() / 2), QPointF(width(), (double)rect().height() / 2));
            grad.setColorAt(0, foreground);
            grad.setColorAt(1, foregroundEnd);
            p.setPen(QPen(grad, 0));

            grad.setColorAt(0, background);
            grad.setColorAt(1, backgroundEnd);
            p.setBrush(grad);

            drawStarPolygons(&p, 5);
        }
    }
}

// -------------------------------------------------------------------------

RatingComboBox::RatingComboBox(QWidget* const parent)
    : ModelIndexBasedComboBox(parent)
{
    m_syncing = false;

    // create a custom model that contains the rating values
    m_model   = new RatingComboBoxModel(this);
    setModel(m_model);

    // set a custom delegate which draws rating stars
    RatingComboBoxDelegate* delegate = new RatingComboBoxDelegate(this);
    view()->setItemDelegate(delegate);

    // set a line edit that carries a RatingWidget
    ProxyLineEdit* lineEdit = new ProxyLineEdit;
    m_ratingWidget          = new RatingComboBoxWidget;
    lineEdit->setWidget(m_ratingWidget);
    setLineEdit(lineEdit);

    connect(view()->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentValueChanged(QModelIndex,QModelIndex)));

    connect(m_ratingWidget, SIGNAL(ratingValueChanged(int)),
            this, SLOT(ratingWidgetChanged(int)));
}

void RatingComboBox::setRatingValue(RatingComboBox::RatingValue value)
{
    if (value > Rating5)
    {
        value = Rating5;
    }
    else if (value < Null)
    {
        value = Null;
    }

    setCurrentIndex(m_model->indexForRatingValue(value));
}

RatingComboBox::RatingValue RatingComboBox::ratingValue() const
{
    return (RatingValue)view()->currentIndex().data(RatingComboBoxModel::RatingRole).toInt();
}

void RatingComboBox::currentValueChanged(const QModelIndex& current, const QModelIndex&)
{
    if (m_syncing)
    {
        return;
    }

    RatingValue value = (RatingValue)current.data(RatingComboBoxModel::RatingRole).toInt();

    m_syncing = true;
    m_ratingWidget->setRatingValue(value);
    m_syncing = false;

    emit ratingValueChanged(value);
}

void RatingComboBox::ratingWidgetChanged(int rv)
{
    if (m_syncing)
    {
        return;
    }

    RatingValue value = (RatingValue)rv;
    QModelIndex index = m_model->indexForRatingValue(value);

    m_syncing = true;
    setCurrentIndex(index);
    m_syncing = false;

    emit ratingValueChanged(value);
}

} // namespace Digikam
