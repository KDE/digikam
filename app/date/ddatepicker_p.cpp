/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : A date selection widget.
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

#include "ddatepicker_p.h"

// Qt includes

#include <QFontDatabase>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

DatePickerValidator::DatePickerValidator(DDatePicker* const parent)
    : QValidator(parent),
      m_picker(parent)
{
}

QValidator::State DatePickerValidator::validate(QString& text, int&) const
{
    QLocale::FormatType formats[] =
    {
        QLocale::LongFormat,
        QLocale::ShortFormat,
        QLocale::NarrowFormat
    };

    QLocale locale = m_picker->locale();

    for (int i = 0; i < 3; i++)
    {
        QDate tmp = locale.toDate(text, formats[i]);

        if (tmp.isValid())
        {
            return Acceptable;
        }
    }

    return QValidator::Intermediate;
}

// ------------------------------------------------------------------------------

// Week numbers are defined by ISO 8601
// See http://www.merlyn.demon.co.uk/weekinfo.htm for details

DatePickerYearSelector::DatePickerYearSelector(const QDate& currentDate, QWidget* const parent)
    : QLineEdit(parent),
      val(new QIntValidator(this)),
      result(0)
{
    oldDate = currentDate;

    setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    setFrame(false);

    // TODO: Find a way to get that from QLocale
    //val->setRange( calendar->year( calendar->earliestValidDate() ),
    //               calendar->year( calendar->latestValidDate() ) );
    setValidator(val);

    connect(this, &QLineEdit::returnPressed,
            this, &DatePickerYearSelector::yearEnteredSlot);
}

void DatePickerYearSelector::yearEnteredSlot()
{
    bool ok;
    int newYear;

    // check if entered value is a number
    newYear = text().toInt(&ok);

    if (!ok)
    {
        QApplication::beep();
        return;
    }

    // check if new year will lead to a valid date
    if (QDate(newYear, oldDate.month(), oldDate.day()).isValid())
    {
        result = newYear;
        emit(closeMe(1));
    }
    else
    {
        QApplication::beep();
    }
}

int DatePickerYearSelector::year() const
{
    return result;
}

void DatePickerYearSelector::setYear(int year)
{
    setText(QString::number(year));
}

// ------------------------------------------------------------------------------

DDatePicker::Private::Private(DDatePicker* const qq)
    : q(qq)
{
    closeButton      = 0;
    selectWeek       = 0;
    todayButton      = 0;
    navigationLayout = 0;
    yearForward      = 0;
    yearBackward     = 0;
    monthForward     = 0;
    monthBackward    = 0;
    selectMonth      = 0;
    selectYear       = 0;
    line             = 0;
    val              = 0;
    table            = 0;
    fontsize         = 0;
}

void DDatePicker::Private::fillWeeksCombo()
{
    // every year can have a different number of weeks
    // it could be that we had 53,1..52 and now 1..53 which is the same number but different
    // so always fill with new values
    // We show all week numbers for all weeks between first day of year to last day of year
    // This of course can be a list like 53,1,2..52

    const QDate thisDate      = q->date();
    const int thisYear        = thisDate.year();
    QDate day(thisDate.year(), 1, 1);
    const QDate lastDayOfYear = QDate(thisDate.year() + 1, 1, 1).addDays(-1);

    selectWeek->clear();

    // Starting from the first day in the year, loop through the year a week at a time
    // adding an entry to the week combo for each week in the year

    for (; day.isValid() && day <= lastDayOfYear; day = day.addDays(7))
    {
        // Get the ISO week number for the current day and what year that week is in
        // e.g. 1st day of this year may fall in week 53 of previous year
        int weekYear       = thisYear;
        const int week     = day.weekNumber(&weekYear);
        QString weekString = i18n("Week %1", QString::number(week));

        // show that this is a week from a different year
        if (weekYear != thisYear)
        {
            weekString += QLatin1Char('*');
        }

        // when the week is selected, go to the same weekday as the one
        // that is currently selected in the date table
        QDate targetDate = day.addDays(thisDate.dayOfWeek() - day.dayOfWeek());
        selectWeek->addItem(weekString, targetDate);

        // make sure that the week of the lastDayOfYear is always inserted: in Chinese calendar
        // system, this is not always the case
        if (day < lastDayOfYear           &&
            day.daysTo(lastDayOfYear) < 7 &&
            lastDayOfYear.weekNumber() != day.weekNumber())
        {
            day = lastDayOfYear.addDays(-7);
        }
    }
}

QDate DDatePicker::Private::validDateInYearMonth(int year, int month)
{
    QDate newDate;

    // Try to create a valid date in this year and month
    // First try the first of the month, then try last of month
    if (QDate(year, month, 1).isValid())
    {
        newDate = QDate(year, month, 1);
    }
    else if (QDate(year, month + 1, 1).isValid())
    {
        newDate = QDate(year, month + 1, 1).addDays(-1);
    }
    else
    {
        newDate = QDate::fromJulianDay(0);
    }

    return newDate;
}

}  // namespace Digikam
