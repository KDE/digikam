/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : maintenance tool
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "maintenancetool.moc"

// Qt includes

#include <QTime>

// KDE includes

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "knotificationwrapper.h"

namespace Digikam
{

class MaintenanceTool::MaintenanceToolPriv
{
public:

    MaintenanceToolPriv()
    {
        notification = true;
    }

    bool  notification;
    QTime duration;
};

MaintenanceTool::MaintenanceTool(const QString& id, ProgressItem* parent)
    : ProgressItem(parent, id, QString(), QString(), true, true),
      d(new MaintenanceToolPriv)
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
    slotStart();
}

void MaintenanceTool::slotStart()
{
    d->duration.start();
}

void MaintenanceTool::slotDone()
{
    QTime now, t = now.addMSecs(d->duration.elapsed());

    if (d->notification)
    {
        // Pop-up a message to bring user when all is done.
        KNotificationWrapper(id(),
                             i18n("Process is done.\nDuration: %1", t.toString()),
                             kapp->activeWindow(), label());
    }

    emit signalComplete();

    setComplete();
}

void MaintenanceTool::slotCancel()
{
    setComplete();
}

}  // namespace Digikam
