/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-05-29
 * Description : a combobox with time zones.
 *
 * Copyright (C) 2015      by Maik Qualmann <metzpinguin at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "timezonecombobox.h"

namespace Digikam
{

TimeZoneComboBox::TimeZoneComboBox(QWidget* const parent)
    : QComboBox(parent)
{
    QStringList timeZones;
    timeZones << QLatin1String("")       << QLatin1String("-12:00") << QLatin1String("-11:00") << QLatin1String("-10:00")
              << QLatin1String("-09:30") << QLatin1String("-09:00") << QLatin1String("-08:00") << QLatin1String("-07:00")
              << QLatin1String("-06:00") << QLatin1String("-05:00") << QLatin1String("-04:30") << QLatin1String("-04:00")
              << QLatin1String("-03:30") << QLatin1String("-03:00") << QLatin1String("-02:00") << QLatin1String("-01:00")
              << QLatin1String("+00:00") << QLatin1String("+01:00") << QLatin1String("+02:00") << QLatin1String("+03:00")
              << QLatin1String("+03:30") << QLatin1String("+04:00") << QLatin1String("+04:30") << QLatin1String("+05:00")
              << QLatin1String("+05:30") << QLatin1String("+05:45") << QLatin1String("+06:00") << QLatin1String("+06:30")
              << QLatin1String("+07:00") << QLatin1String("+08:00") << QLatin1String("+09:00") << QLatin1String("+09:30")
              << QLatin1String("+10:00") << QLatin1String("+10:30") << QLatin1String("+11:00") << QLatin1String("+11:30")
              << QLatin1String("+12:00") << QLatin1String("+12:45") << QLatin1String("+13:00") << QLatin1String("+13:45")
              << QLatin1String("+14:00");

    addItems(timeZones);
}

TimeZoneComboBox::~TimeZoneComboBox()
{
}

void TimeZoneComboBox::setToUTC()
{
    setCurrentIndex(findText(QLatin1String("+00:00")));
}

void TimeZoneComboBox::setTimeZone(const QString& timeStr)
{
    if (timeStr.length() < 6)
    {
        setCurrentIndex(0);
        return;
    }

    QString timeZone = timeStr.right(6);

    if (timeZone.endsWith(QLatin1String("Z")))
    {
        setToUTC();
    }
    else if (timeZone.mid(3, 1) == QLatin1String(":") &&
            (timeZone.left(1) == QLatin1String("+") ||
             timeZone.left(1) == QLatin1String("-")))
    {
        setCurrentIndex(findText(timeZone));
    }
    else
    {
        setCurrentIndex(0);
    }
}

QString TimeZoneComboBox::getTimeZone() const
{
    return currentText();
}

}  // namespace Digikam
