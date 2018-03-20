/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : Date selection table.
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 1997      by Tim D. Gilman <tdgilman at best dot org>
 * Copyright (C) 1998-2001 by Mirko Boehm <mirko at kde dot org>
 * Copyright (C) 2007      by John Layt <john at layt dot net>
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

#include "ddatetable.h"
#include "ddatetable_p.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QAction>
#include <QFontDatabase>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QActionEvent>
#include <QApplication>
#include <QMenu>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DDateTable::DDateTable(const QDate& date, QWidget* const parent)
    : QWidget(parent),
      d(new Private(this))
{
    initWidget(date);
}

DDateTable::DDateTable(QWidget* const parent)
    : QWidget(parent),
      d(new Private(this))
{
    initWidget(QDate::currentDate());
}

DDateTable::~DDateTable()
{
    delete d;
}

void DDateTable::initWidget(const QDate& date)
{
    d->numWeekRows = 7;

    setFontSize(10);
    setFocusPolicy(Qt::StrongFocus);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    initAccels();
    setAttribute(Qt::WA_Hover, true);

    setDate(date);
}

void DDateTable::initAccels()
{
    QAction* const next = new QAction(this);
    next->setObjectName(QLatin1String("next"));
    next->setShortcuts(QKeySequence::keyBindings(QKeySequence::Forward));
    next->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    connect(next, SIGNAL(triggered(bool)),
            d, SLOT(nextMonth()));

    QAction* const prior = new QAction(this);
    prior->setObjectName(QLatin1String("prior"));
    prior->setShortcuts(QKeySequence::keyBindings(QKeySequence::Back));
    prior->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    connect(prior, SIGNAL(triggered(bool)),
            d, SLOT(previousMonth()));

    QAction* const beginMonth = new QAction(this);
    beginMonth->setObjectName(QLatin1String("beginMonth"));
    beginMonth->setShortcuts(QKeySequence::keyBindings(QKeySequence::MoveToStartOfDocument));
    beginMonth->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    connect(beginMonth, SIGNAL(triggered(bool)),
            d, SLOT(beginningOfMonth()));

    QAction* const endMonth = new QAction(this);
    endMonth->setObjectName(QLatin1String("endMonth"));
    endMonth->setShortcuts(QKeySequence::keyBindings(QKeySequence::MoveToEndOfDocument));
    endMonth->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    connect(endMonth, SIGNAL(triggered(bool)),
            d, SLOT(endOfMonth()));

    QAction* const beginWeek = new QAction(this);
    beginWeek->setObjectName(QLatin1String("beginWeek"));
    beginWeek->setShortcuts(QKeySequence::keyBindings(QKeySequence::MoveToStartOfLine));
    beginWeek->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    connect(beginWeek, SIGNAL(triggered(bool)),
            d, SLOT(beginningOfWeek()));

    QAction* const endWeek = new QAction(this);
    endWeek->setObjectName(QLatin1String("endWeek"));
    endWeek->setShortcuts(QKeySequence::keyBindings(QKeySequence::MoveToEndOfLine));
    endWeek->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    connect(endWeek, SIGNAL(triggered(bool)),
            d, SLOT(endOfWeek()));
}

int DDateTable::posFromDate(const QDate& date)
{
    int initialPosition = date.day();
    int offset          = (d->weekDayFirstOfMonth - locale().firstDayOfWeek() + d->numDayColumns) % d->numDayColumns;

    // make sure at least one day of the previous month is visible.
    // adjust this < 1 if more days should be forced visible:
    if (offset < 1)
    {
        offset += d->numDayColumns;
    }

    return initialPosition + offset;
}

QDate DDateTable::dateFromPos(int position)
{
    int offset = (d->weekDayFirstOfMonth - locale().firstDayOfWeek() + d->numDayColumns) % d->numDayColumns;

    // make sure at least one day of the previous month is visible.
    // adjust this < 1 if more days should be forced visible:
    if (offset < 1)
    {
        offset += d->numDayColumns;
    }

    return QDate(d->date.year(), d->date.month(), 1).addDays(position - offset);
}

void DDateTable::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    const QRect& rectToUpdate = e->rect();
    double cellWidth          = width() / (double) d->numDayColumns;
    double cellHeight         = height() / (double) d->numWeekRows;
    int leftCol               = (int)std::floor(rectToUpdate.left() / cellWidth);
    int topRow                = (int)std::floor(rectToUpdate.top() / cellHeight);
    int rightCol              = (int)std::ceil(rectToUpdate.right() / cellWidth);
    int bottomRow             = (int)std::ceil(rectToUpdate.bottom() / cellHeight);
    bottomRow                 = qMin(bottomRow, d->numWeekRows - 1);
    rightCol                  = qMin(rightCol, d->numDayColumns - 1);

    if (layoutDirection() == Qt::RightToLeft)
    {
        p.translate((d->numDayColumns - leftCol - 1) * cellWidth, topRow * cellHeight);
    }
    else
    {
        p.translate(leftCol * cellWidth, topRow * cellHeight);
    }

    for (int i = leftCol; i <= rightCol; ++i)
    {
        for (int j = topRow; j <= bottomRow; ++j)
        {
            paintCell(&p, j, i);
            p.translate(0, cellHeight);
        }

        if (layoutDirection() == Qt::RightToLeft)
        {
            p.translate(-cellWidth, 0);
        }
        else
        {
            p.translate(cellWidth, 0);
        }

        p.translate(0, -cellHeight * (bottomRow - topRow + 1));
    }
}

void DDateTable::paintCell(QPainter* painter, int row, int col)
{
    double w    = (width() / (double) d->numDayColumns) - 1;
    double h    = (height() / (double) d->numWeekRows) - 1;
    QRectF cell = QRectF(0, 0, w, h);
    QString cellText;
    QPen pen;
    QColor cellBackgroundColor, cellTextColor;
    QFont cellFont  = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    bool workingDay = false;
    int cellWeekDay, pos;

    //Calculate the position of the cell in the grid
    pos = d->numDayColumns * (row - 1) + col;

    //Calculate what day of the week the cell is
    if (col + locale().firstDayOfWeek() <= d->numDayColumns)
    {
        cellWeekDay = col + locale().firstDayOfWeek();
    }
    else
    {
        cellWeekDay = col + locale().firstDayOfWeek() - d->numDayColumns;
    }

    //FIXME This is wrong if the widget is not using the global!
    //See if cell day is normally a working day
    if (locale().weekdays().first() <= locale().weekdays().last())
    {
        if (cellWeekDay >= locale().weekdays().first() &&
            cellWeekDay <= locale().weekdays().last())
        {
            workingDay = true;
        }
    }
    else
    {
        if (cellWeekDay >= locale().weekdays().first() ||
            cellWeekDay <= locale().weekdays().last())
        {
            workingDay = true;
        }
    }

    if (row == 0)
    {
        //We are drawing a header cell

        //If not a normal working day, then use "do not work today" color
        if (workingDay)
        {
            cellTextColor = palette().color(QPalette::WindowText);
        }
        else
        {
            cellTextColor = Qt::darkRed;
        }

        cellBackgroundColor = palette().color(QPalette::Window);

        //Set the text to the short day name and bold it
        cellFont.setBold(true);
        cellText = locale().dayName(cellWeekDay, QLocale::ShortFormat);

    }
    else
    {
        //We are drawing a day cell

        //Calculate the date the cell represents
        QDate cellDate = dateFromPos(pos);
        bool validDay  = cellDate.isValid();

        // Draw the day number in the cell, if the date is not valid then we don't want to show it
        if (validDay)
        {
            cellText = QString::number(cellDate.day());
        }
        else
        {
            cellText = QLatin1String("");
        }

        if (! validDay || cellDate.month() != d->date.month())
        {
            // we are either
            // ° painting an invalid day
            // ° painting a day of the previous month or
            // ° painting a day of the following month or
            cellBackgroundColor = palette().color(backgroundRole());
            cellTextColor       = palette().color(QPalette::Disabled, QPalette::Text);
        }
        else
        {
            //Paint a day of the current month

            // Background Colour priorities will be (high-to-low):
            // * Selected Day Background Colour
            // * Customized Day Background Colour
            // * Normal Day Background Colour

            // Background Shape priorities will be (high-to-low):
            // * Customized Day Shape
            // * Normal Day Shape

            // Text Colour priorities will be (high-to-low):
            // * Customized Day Colour
            // * Day of Pray Colour (Red letter)
            // * Selected Day Colour
            // * Normal Day Colour

            //Determine various characteristics of the cell date
            bool selectedDay = (cellDate == date());
            bool currentDay  = (cellDate == QDate::currentDate());
            bool dayOfPray   = (cellDate.dayOfWeek() == Qt::Sunday);
            // TODO: Uncomment if QLocale ever gets the feature...
            //bool dayOfPray = ( cellDate.dayOfWeek() == locale().dayOfPray() );
            bool customDay   = (d->useCustomColors && d->customPaintingModes.contains(cellDate.toJulianDay()));

            //Default values for a normal cell
            cellBackgroundColor = palette().color(backgroundRole());
            cellTextColor = palette().color(foregroundRole());

            // If we are drawing the current date, then draw it bold and active
            if (currentDay)
            {
                cellFont.setBold(true);
                cellTextColor = palette().color(QPalette::LinkVisited);
            }

            // if we are drawing the day cell currently selected in the table
            if (selectedDay)
            {
                // set the background to highlighted
                cellBackgroundColor = palette().color(QPalette::Highlight);
                cellTextColor = palette().color(QPalette::HighlightedText);
            }

            //If custom colors or shape are required for this date
            if (customDay)
            {
                Private::DatePaintingMode mode = d->customPaintingModes[cellDate.toJulianDay()];

                if (mode.bgMode != NoBgMode)
                {
                    if (!selectedDay)
                    {
                        cellBackgroundColor = mode.bgColor;
                    }
                }

                cellTextColor = mode.fgColor;
            }

            //If the cell day is the day of religious observance, then always color text red unless Custom overrides
            if (! customDay && dayOfPray)
            {
                cellTextColor = Qt::darkRed;
            }

        }
    }

    //Draw the background
    if (row == 0)
    {
        painter->setPen(cellBackgroundColor);
        painter->setBrush(cellBackgroundColor);
        painter->drawRect(cell);
    }
    else if (cellBackgroundColor != palette().color(backgroundRole()) || pos == d->hoveredPos)
    {
        QStyleOptionViewItem opt;
        opt.initFrom(this);
        opt.rect = cell.toRect();

        if (cellBackgroundColor != palette().color(backgroundRole()))
        {
            opt.palette.setBrush(QPalette::Highlight, cellBackgroundColor);
            opt.state |= QStyle::State_Selected;
        }

        if (pos == d->hoveredPos && opt.state & QStyle::State_Enabled)
        {
            opt.state |= QStyle::State_MouseOver;
        }
        else
        {
            opt.state &= ~QStyle::State_MouseOver;
        }

        opt.showDecorationSelected = true;
        opt.viewItemPosition       = QStyleOptionViewItem::OnlyOne;
        style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, this);
    }

    //Draw the text
    painter->setPen(cellTextColor);
    painter->setFont(cellFont);
    painter->drawText(cell, Qt::AlignCenter, cellText, &cell);

    //Draw the base line
    if (row == 0)
    {
        painter->setPen(palette().color(foregroundRole()));
        painter->drawLine(QPointF(0, h), QPointF(w, h));
    }

    // If the day cell we just drew is bigger than the current max cell sizes,
    // then adjust the max to the current cell
    if (cell.width() > d->maxCell.width())
    {
        d->maxCell.setWidth(cell.width());
    }

    if (cell.height() > d->maxCell.height())
    {
        d->maxCell.setHeight(cell.height());
    }
}

void DDateTable::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_Up:
            // setDate does validity checking for us
            setDate(d->date.addDays(- d->numDayColumns));
            break;
        case Qt::Key_Down:
            // setDate does validity checking for us
            setDate(d->date.addDays(d->numDayColumns));
            break;
        case Qt::Key_Left:
            // setDate does validity checking for us
            setDate(d->date.addDays(-1));
            break;
        case Qt::Key_Right:
            // setDate does validity checking for us
            setDate(d->date.addDays(1));
            break;
        case Qt::Key_Minus:
            // setDate does validity checking for us
            setDate(d->date.addDays(-1));
            break;
        case Qt::Key_Plus:
            // setDate does validity checking for us
            setDate(d->date.addDays(1));
            break;
        case Qt::Key_N:
            // setDate does validity checking for us
            setDate(QDate::currentDate());
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit tableClicked();
            break;
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Meta:
        case Qt::Key_Shift:
            // Don't beep for modifiers
            break;
        default:
            if (!e->modifiers())
            {   // hm
                QApplication::beep();
            }
    }
}

void DDateTable::setFontSize(int size)
{
    QFontMetricsF metrics(fontMetrics());
    QRectF rect;

    // ----- store rectangles:
    d->fontsize = size;

    // ----- find largest day name:
    d->maxCell.setWidth(0);
    d->maxCell.setHeight(0);

    for (int weekday = 1; weekday <= 7; ++weekday)
    {
        rect = metrics.boundingRect(locale().dayName(weekday, QLocale::ShortFormat));
        d->maxCell.setWidth(qMax(d->maxCell.width(), rect.width()));
        d->maxCell.setHeight(qMax(d->maxCell.height(), rect.height()));
    }

    // ----- compare with a real wide number and add some space:
    rect = metrics.boundingRect(QLatin1String("88"));
    d->maxCell.setWidth(qMax(d->maxCell.width() + 2, rect.width()));
    d->maxCell.setHeight(qMax(d->maxCell.height() + 4, rect.height()));
}

void DDateTable::wheelEvent(QWheelEvent *e)
{
    setDate(d->date.addMonths(-(int)(e->delta() / 120)));
    e->accept();
}

bool DDateTable::event(QEvent *ev)
{
    switch (ev->type())
    {
        case QEvent::HoverMove:
        {
            QHoverEvent *e = static_cast<QHoverEvent *>(ev);
            const int row = e->pos().y() * d->numWeekRows / height();
            int col;

            if (layoutDirection() == Qt::RightToLeft)
            {
                col = d->numDayColumns - (e->pos().x() * d->numDayColumns / width()) - 1;
            }
            else
            {
                col = e->pos().x() * d->numDayColumns / width();
            }

            const int pos = row < 1 ? -1 : (d->numDayColumns * (row - 1)) + col;

            if (pos != d->hoveredPos)
            {
                d->hoveredPos = pos;
                update();
            }
            break;
        }
        case QEvent::HoverLeave:
            if (d->hoveredPos != -1)
            {
                d->hoveredPos = -1;
                update();
            }
            break;
        default:
            break;
    }

    return QWidget::event(ev);
}

void DDateTable::mousePressEvent(QMouseEvent *e)
{
    if (e->type() != QEvent::MouseButtonPress)
    {
        // the KDatePicker only reacts on mouse press events:
        return;
    }

    if (!isEnabled())
    {
        QApplication::beep();
        return;
    }

    int row, col, pos;

    QPoint mouseCoord = e->pos();
    row = mouseCoord.y() * d->numWeekRows / height();

    if (layoutDirection() == Qt::RightToLeft)
    {
        col = d->numDayColumns - (mouseCoord.x() * d->numDayColumns / width()) - 1;
    }
    else
    {
        col = mouseCoord.x() * d->numDayColumns / width();
    }

    if (row < 1 || col < 0)
    {  // the user clicked on the frame of the table
        return;
    }

    // Rows and columns are zero indexed.  The (row - 1) below is to avoid counting
    // the row with the days of the week in the calculation.

    // new position and date
    pos               = (d->numDayColumns * (row - 1)) + col;
    QDate clickedDate = dateFromPos(pos);

    // set the new date. If it is in the previous or next month, the month will
    // automatically be changed, no need to do that manually...
    // validity checking done inside setDate
    setDate(clickedDate);

    // This could be optimized to only call update over the regions
    // of old and new cell, but 99% of times there is also a call to
    // setDate that already calls update() so no need to optimize that
    // much here
    update();

    emit tableClicked();

    if (e->button() == Qt::RightButton && d->popupMenuEnabled)
    {
        QMenu* const menu = new QMenu();
        menu->addSection(locale().toString(d->date));
        emit aboutToShowContextMenu(menu, clickedDate);
        menu->popup(e->globalPos());
    }
}

bool DDateTable::setDate(const QDate& toDate)
{
    if (!toDate.isValid())
    {
        return false;
    }

    if (toDate == date())
    {
        return true;
    }

    QDate oldDate = date();
    d->setDate(toDate);
    emit(dateChanged(date(), oldDate));
    emit(dateChanged(date()));
    update();

    return true;
}

const QDate& DDateTable::date() const
{
    return d->date;
}

void DDateTable::focusInEvent(QFocusEvent* e)
{
    QWidget::focusInEvent(e);
}

void DDateTable::focusOutEvent(QFocusEvent* e)
{
    QWidget::focusOutEvent(e);
}

QSize DDateTable::sizeHint() const
{
    if (d->maxCell.height() > 0 && d->maxCell.width() > 0)
    {
        return QSize( qRound(d->maxCell.width() * d->numDayColumns),
                     (qRound(d->maxCell.height() + 2) * d->numWeekRows));
    }
    else
    {
        //qCDebug(DIGIKAM_GENERAL_LOG) << "DDateTable::sizeHint: obscure failure - " << endl;
        return QSize(-1, -1);
    }
}

void DDateTable::setPopupMenuEnabled(bool enable)
{
    d->popupMenuEnabled = enable;
}

bool DDateTable::popupMenuEnabled() const
{
    return d->popupMenuEnabled;
}

void DDateTable::setCustomDatePainting(const QDate& date, const QColor& fgColor,
                                       BackgroundMode bgMode, const QColor& bgColor)
{
    if (!fgColor.isValid())
    {
        unsetCustomDatePainting(date);
        return;
    }

    Private::DatePaintingMode mode;
    mode.bgMode          = bgMode;
    mode.fgColor         = fgColor;
    mode.bgColor         = bgColor;

    d->customPaintingModes.insert(date.toJulianDay(), mode);
    d->useCustomColors = true;
    update();
}

void DDateTable::unsetCustomDatePainting(const QDate& date)
{
    d->customPaintingModes.remove(date.toJulianDay());

    if (d->customPaintingModes.isEmpty())
    {
        d->useCustomColors = false;
    }
    update();
}

}  // namespace Digikam
