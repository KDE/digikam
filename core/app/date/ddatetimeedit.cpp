/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a widget to edit time stamp.
 *
 * Copyright (C) 2005      by Tom Albers <tomalbers at kde dot nl>
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

#include "ddatetimeedit.h"

// Qt includes

#include <QDateTimeEdit>

// Local includes

#include "ddateedit.h"

namespace Digikam
{

class DDateTimeEdit::Private
{
public:

    Private() :
        timePopUp(0),
        datePopUp(0)
    {
    }

    QTimeEdit* timePopUp;
    DDateEdit* datePopUp;
};

DDateTimeEdit::DDateTimeEdit(QWidget* const parent, const QString& name)
    : DHBox(parent),
      d(new Private)
{
    setObjectName(name);

    d->datePopUp = new DDateEdit(this, QLatin1String("datepopup"));
    d->timePopUp = new QTimeEdit(QTime::currentTime(), this);

    d->timePopUp->setDisplayFormat(QLatin1String("hh:mm:ss"));

    connect(d->datePopUp, &DDateEdit::dateChanged,
            this, &DDateTimeEdit::slotDateTimeChanged);

    connect(d->timePopUp, &QTimeEdit::timeChanged,
            this, &DDateTimeEdit::slotDateTimeChanged);
}

DDateTimeEdit::~DDateTimeEdit()
{
    delete d->datePopUp;
    d->datePopUp = 0;

    delete d->timePopUp;
    d->timePopUp = 0;

    delete d;
}

QDateTime DDateTimeEdit::dateTime() const
{
    return QDateTime(d->datePopUp->date(), d->timePopUp->time());
}

void DDateTimeEdit::setDateTime(const QDateTime& dateTime)
{
    d->datePopUp->setDate(dateTime.date());
    d->timePopUp->setTime(dateTime.time());
}

void DDateTimeEdit::slotDateTimeChanged()
{
    emit dateTimeChanged(dateTime());
}

}  // namespace Digikam
