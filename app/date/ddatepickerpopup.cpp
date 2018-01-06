/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-04-21
 * Description : a menu widget to pick a date.
 *
 * Copyright (C) 2004      by Bram Schoenmakers <bramschoenmakers at kde dot nl>
 * Copyright (C) 2006      by Mikolaj Machowski <mikmach at wp dot pl>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ddatepickerpopup.h"

// Qt includes

#include <QDateTime>
#include <QWidgetAction>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class DDatePickerAction : public QWidgetAction
{
public:

    DDatePickerAction(DDatePicker* const widget, QObject* const parent)
        : QWidgetAction(parent),
          m_datePicker(widget),
          m_originalParent(widget->parentWidget())
    {
    }

protected:

    QWidget* createWidget(QWidget* const parent)
    {
        m_datePicker->setParent(parent);
        return m_datePicker;
    }

    void deleteWidget(QWidget* const widget)
    {
        if (widget != m_datePicker)
        {
            return;
        }

        m_datePicker->setParent(m_originalParent);
    }

private:

    DDatePicker* m_datePicker;
    QWidget*     m_originalParent;
};

// ---------------------------------------------------------------------------------------

class DDatePickerPopup::Private
{
public:

    Private() :
        datePicker(0)
    {
    }

    DDatePicker* datePicker;
    Items        items;
};

DDatePickerPopup::DDatePickerPopup(Items items, const QDate& date, QWidget* const parent)
    : QMenu(parent),
      d(new Private)
{
    d->items      = items;
    d->datePicker = new DDatePicker(this);
    d->datePicker->setCloseButton(false);

    connect(d->datePicker, &DDatePicker::dateEntered,
            this, &DDatePickerPopup::slotDateChanged);

    connect(d->datePicker, &DDatePicker::dateSelected,
            this, &DDatePickerPopup::slotDateChanged);

    d->datePicker->setDate(date);

    buildMenu();
}

DDatePickerPopup::~DDatePickerPopup()
{
    delete d;
}

void DDatePickerPopup::buildMenu()
{
    if (isVisible())
    {
        return;
    }

    clear();

    if (d->items & DatePicker)
    {
        addAction(new DDatePickerAction(d->datePicker, this));

        if ((d->items & NoDate ) || (d->items & Words))
        {
            addSeparator();
        }
    }

    if (d->items & Words)
    {
        addAction(i18n("&Today"),       this, SLOT(slotToday()));
        addAction(i18n("To&morrow"),    this, SLOT(slotTomorrow()));
        addAction(i18n("Next &Week"),   this, SLOT(slotNextWeek()));
        addAction(i18n("Next M&onth"),  this, SLOT(slotNextMonth()));
        addAction(i18n("Y&esterday"),   this, SLOT(slotYesterday()));
        addAction(i18n("Last &Monday"), this, SLOT(slotPrevMonday()));
        addAction(i18n("Last &Friday"), this, SLOT(slotPrevFriday()));
        addAction(i18n("Last &Week"),   this, SLOT(slotPrevWeek()));
        addAction(i18n("Last M&onth"),  this, SLOT(slotPrevMonth()));

        if (d->items & NoDate)
        {
            addSeparator();
        }
    }

    if (d->items & NoDate)
    {
        addAction(i18n("No Date"), this, SLOT(slotNoDate()));
    }
}

DDatePicker* DDatePickerPopup::datePicker() const
{
    return d->datePicker;
}

void DDatePickerPopup::setDate(const QDate& date)
{
    d->datePicker->setDate(date);
}

#if 0
void DDatePickerPopup::setItems(int items)
{
    d->items = items;
    buildMenu();
}
#endif

int DDatePickerPopup::items() const
{
    return d->items;
}

void DDatePickerPopup::slotDateChanged(const QDate& date)
{
    emit dateChanged(date);
    hide();
}

void DDatePickerPopup::slotToday()
{
    emit dateChanged(QDate::currentDate());
}

void DDatePickerPopup::slotTomorrow()
{
    emit dateChanged(QDate::currentDate().addDays(1));
}

void DDatePickerPopup::slotNoDate()
{
    emit dateChanged(QDate());
}

void DDatePickerPopup::slotNextWeek()
{
    emit dateChanged(QDate::currentDate().addDays(7));
}

void DDatePickerPopup::slotNextMonth()
{
    emit dateChanged(QDate::currentDate().addMonths(1));
}

void DDatePickerPopup::slotYesterday()
{
    emit dateChanged(QDate::currentDate().addDays(-1));
}

void DDatePickerPopup::slotPrevFriday()
{
    QDate date = QDate::currentDate();
    int day    = date.dayOfWeek();

    if (day < 6)
    {
        date = date.addDays(5 - 7 - day);
    }
    else
    {
        date = date.addDays(5 - day);
    }

    emit dateChanged(date);
}

void DDatePickerPopup::slotPrevMonday()
{
    QDate date = QDate::currentDate();
    emit dateChanged(date.addDays(1 - date.dayOfWeek()));
}

void DDatePickerPopup::slotPrevWeek()
{
    emit dateChanged(QDate::currentDate().addDays(-7));
}

void DDatePickerPopup::slotPrevMonth()
{
    emit dateChanged(QDate::currentDate().addMonths(-1));
}

}  // namespace Digikam
