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

#ifndef DDATEPICKER_P_H
#define DDATEPICKER_P_H

// Qt includes

#include <QDate>
#include <QLineEdit>
#include <QValidator>
#include <QApplication>
#include <QComboBox>
#include <QToolButton>
#include <QBoxLayout>
#include <QSize>

// Local includes

#include "ddatepicker.h"

namespace Digikam
{

class DatePickerValidator : public QValidator
{
public:

    explicit DatePickerValidator(DDatePicker* const parent);

    State validate(QString& text, int&) const Q_DECL_OVERRIDE;

private:

    DDatePicker* m_picker;
};

// ------------------------------------------------------------------------------

class DatePickerYearSelector : public QLineEdit
{
    Q_OBJECT

public:

    explicit DatePickerYearSelector(const QDate& currentDate, QWidget* const parent = 0);

    int  year() const;
    void setYear(int year);

public Q_SLOTS:

    void yearEnteredSlot();

Q_SIGNALS:

    void closeMe(int);

protected:

    QIntValidator* val;
    int            result;

private:

    QDate          oldDate;

    Q_DISABLE_COPY(DatePickerYearSelector)
};

// ------------------------------------------------------------------------------

class DDatePicker::Private
{
public:

    explicit Private(DDatePicker* const qq);

    void fillWeeksCombo();
    QDate validDateInYearMonth(int year, int month);

public:

    /// the date table
    DDatePicker*         q;

    QToolButton*         closeButton;
    QComboBox*           selectWeek;
    QToolButton*         todayButton;
    QBoxLayout*          navigationLayout;

    /// the year forward button
    QToolButton*         yearForward;
    /// the year backward button
    QToolButton*         yearBackward;
    /// the month forward button
    QToolButton*         monthForward;
    /// the month backward button
    QToolButton*         monthBackward;
    /// the button for selecting the month directly
    QToolButton*         selectMonth;
    /// the button for selecting the year directly
    QToolButton*         selectYear;
    /// the line edit to enter the date directly
    QLineEdit*           line;
    /// the validator for the line edit:
    DatePickerValidator* val;
    /// the date table
    DDateTable*          table;
    /// the widest month string in pixels:
    QSize                maxMonthRect;

    /// the font size for the widget
    int                  fontsize;
};

}  // namespace Digikam

#endif // DDATEPICKER_P_H
