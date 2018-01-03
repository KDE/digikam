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

#ifndef TIMEZONE_COMBOBOX_H
#define TIMEZONE_COMBOBOX_H

// Qt includes

#include <QComboBox>

namespace Digikam
{

class TimeZoneComboBox : public QComboBox
{
    Q_OBJECT

public:

    TimeZoneComboBox(QWidget* const parent);
    ~TimeZoneComboBox();

    void setToUTC();
    void setTimeZone(const QString& timeStr);

    QString getTimeZone() const;
};

}  // namespace Digikam

#endif // TIMEZONE_COMBOBOX_H
