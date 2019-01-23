/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : time adjust thread.
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (c) 2018      by Maik Qualmann <metzpinguin at gmail dot com>
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

#ifndef DIGIKAM_TIME_ADJUST_THREAD_H
#define DIGIKAM_TIME_ADJUST_THREAD_H

// Qt includes

#include <QDateTime>
#include <QMap>
#include <QUrl>

// Local includes

#include "actionthreadbase.h"
#include "timeadjustcontainer.h"

using namespace Digikam;

namespace GenericTimeAdjustPlugin
{

class TimeAdjustThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit TimeAdjustThread(QObject* const parent);
    ~TimeAdjustThread();

    void setUpdatedDates(const QMap<QUrl, QDateTime>& itemsMap);
    void setSettings(const TimeAdjustContainer& settings);
    void cancel();

Q_SIGNALS:

    void signalProcessStarted(const QUrl&);
    void signalProcessEnded(const QUrl&, int);
    void signalDateTimeForUrl(const QUrl&, const QDateTime&, bool);
    void signalCancelTask();

public:

    class Private;

private:

    Private* const d;
};

}  // namespace GenericTimeAdjustPlugin

#endif // DIGIKAM_TIME_ADJUST_THREAD_H
