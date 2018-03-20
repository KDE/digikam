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

namespace Digikam
{

DDateTable::Private::Private(DDateTable* const qq)
    : QObject(qq),
      q(qq)
{
    weekDayFirstOfMonth = 0;
    numDaysThisMonth    = 0;
    numWeekRows         = 0;
    numDayColumns       = 0;
    fontsize            = 0;
    popupMenuEnabled    = false;
    useCustomColors     = false;
    hoveredPos          = -1;
    setDate(QDate::currentDate());
}

DDateTable::Private::~Private()
{
}

void DDateTable::Private::nextMonth()
{
    // setDate does validity checking for us
    q->setDate(date.addMonths(1));
}

void DDateTable::Private::previousMonth()
{
    // setDate does validity checking for us
    q->setDate(date.addMonths(-1));
}

void DDateTable::Private::beginningOfMonth()
{
    // setDate does validity checking for us
    q->setDate(QDate(date.year(), date.month(), 1));
}

void DDateTable::Private::endOfMonth()
{
    // setDate does validity checking for us
    q->setDate(QDate(date.year(), date.month() + 1, 0));
}


void DDateTable::Private::beginningOfWeek()
{
    // setDate does validity checking for us
    q->setDate(date.addDays(1 - date.dayOfWeek()));
}

void DDateTable::Private::endOfWeek()
{
    // setDate does validity checking for us
    q->setDate(date.addDays(7 - date.dayOfWeek()));
}

void DDateTable::Private::setDate(const QDate& dt)
{
    date                = dt;
    weekDayFirstOfMonth = QDate(date.year(), date.month(), 1).dayOfWeek();
    numDaysThisMonth    = date.daysInMonth();
    numDayColumns       = 7;
}

}  // namespace Digikam
