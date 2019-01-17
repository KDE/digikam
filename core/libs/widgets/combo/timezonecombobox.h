/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-05-29
 * Description : a combobox with time zones.
 *
 * Copyright (C) 2015      by Maik Qualmann <metzpinguin at gmail dot com>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_TIMEZONE_COMBOBOX_H
#define DIGIKAM_TIMEZONE_COMBOBOX_H

// Qt includes

#include <QComboBox>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT TimeZoneComboBox : public QComboBox
{
    Q_OBJECT

public:

    explicit TimeZoneComboBox(QWidget* const parent);
    ~TimeZoneComboBox();

    void setToUTC();
    void setTimeZone(const QString& timeStr);

    QString getTimeZone()    const;
    int     timeZoneOffset() const;
};

} // namespace Digikam

#endif // DIGIKAM_TIMEZONE_COMBOBOX_H
