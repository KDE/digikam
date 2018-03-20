/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : maintenance tool
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "maintenancetool.h"

// Qt includes

#include <QTime>
#include <QTimer>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dnotificationwrapper.h"

namespace Digikam
{

class MaintenanceTool::Private
{
public:

    Private()
    {
        notification = true;
    }

    bool  notification;
    QTime duration;
};

MaintenanceTool::MaintenanceTool(const QString& id, ProgressItem* const parent)
    : ProgressItem(parent, id, QString(), QString(), true, true),
      d(new Private)
{
    connect(this, SIGNAL(progressItemCanceled(QString)),
            this, SLOT(slotCancel()));
}

MaintenanceTool::~MaintenanceTool()
{
    delete d;
}

void MaintenanceTool::setNotificationEnabled(bool b)
{
    d->notification = b;
}

void MaintenanceTool::start()
{
    // We delay start to be sure that eventloop connect signals and slots in top level.
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

void MaintenanceTool::slotStart()
{
    d->duration.start();
}

void MaintenanceTool::slotDone()
{
    QTime t = QTime::fromMSecsSinceStartOfDay(d->duration.elapsed());

    if (d->notification)
    {
        // Pop-up a message to bring user when all is done.
        DNotificationWrapper(id(),
                             i18n("Process is done.\nDuration: %1", t.toString()),
                             qApp->activeWindow(), label());
    }

    emit signalComplete();

    setComplete();
}

void MaintenanceTool::slotCancel()
{
    setComplete();
}

}  // namespace Digikam
