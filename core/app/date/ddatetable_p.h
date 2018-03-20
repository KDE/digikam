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

#ifndef DDATE_TABLE_P_H
#define DDATE_TABLE_P_H

#include "ddatetable.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QObject>
#include <QColor>
#include <QDate>
#include <QRectF>
#include <QHash>

namespace Digikam
{

class DDateTable::Private : public QObject
{
    Q_OBJECT

public:

    struct DatePaintingMode
    {
        QColor         fgColor;
        QColor         bgColor;
        BackgroundMode bgMode;
    };

public:

    Private(DDateTable* const qq);
    ~Private();

public Q_SLOTS:

    void setDate(const QDate&);
    void nextMonth();
    void previousMonth();
    void beginningOfMonth();
    void endOfMonth();
    void beginningOfWeek();
    void endOfWeek();

public:

    DDateTable*                   q;

    /**
    * The currently selected date.
    */
    QDate                         date;

    /**
     * The weekday number of the first day in the month [1..daysInWeek()].
     */
    int                           weekDayFirstOfMonth;

    /**
     * The number of days in the current month.
     */
    int                           numDaysThisMonth;

    /**
     * Save the size of the largest used cell content.
     */
    QRectF                        maxCell;

    /**
     * How many week rows we are to draw.
     */
    int                           numWeekRows;

    /**
     * How many day columns we are to draw, i.e. days in a week.
     */
    int                           numDayColumns;

    /**
     * The font size of the displayed text.
     */
    int                           fontsize;

    bool                          popupMenuEnabled;
    bool                          useCustomColors;

    QHash <int, DatePaintingMode> customPaintingModes;

    int                           hoveredPos;
};

}  // namespace Digikam

#endif // DDATE_TABLE_P_H
