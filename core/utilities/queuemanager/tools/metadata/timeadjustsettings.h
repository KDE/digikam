/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : time adjust settings widget.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
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

#ifndef TIME_ADJUST_SETTINGS_H
#define TIME_ADJUST_SETTINGS_H

// Qt includes

#include <QDateTime>
#include <QScrollArea>

// Local includes

#include "timeadjustcontainer.h"

namespace Digikam
{

class TimeAdjustContainer;

class TimeAdjustSettings : public QScrollArea
{
    Q_OBJECT

public:

    explicit TimeAdjustSettings(QWidget* const parent = 0);
    ~TimeAdjustSettings();

public:

    void setSettings(const TimeAdjustContainer& settings);
    TimeAdjustContainer settings() const;
    void detAdjustmentByClockPhotoUrl(const QUrl& url);

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotSrcTimestampChanged();
    void slotResetDateToCurrent();
    void slotAdjustmentTypeChanged();
    void slotDetAdjustmentByClockPhotoDialog();
    void slotDetAdjustmentByClockPhotoUrl(const QUrl& url);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // TIME_ADJUST_SETTINGS_H
