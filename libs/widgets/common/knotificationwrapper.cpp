/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : A wrapper around KNotification which uses
 *               KPassivePopup if KNotify is unavailable
 *
 * Copyright (C) 2009 by Michael G. Hansen <mike at mghansen dot de>
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

#include "knotificationwrapper.h"

// Qt includes

#include <QDBusConnection>
#include <QDBusConnectionInterface>

// KDE includes

#include <kstandarddirs.h>
#include <knotification.h>
#include <kpassivepopup.h>

namespace Digikam
{

/**
 * @param eventId Event id for this notification, KNotification::Notification
 *                is used if this is empty
 * @param message Message to display
 */
void KNotificationWrapper(const QString& eventId, const QString& message,
                          QWidget* const widget, const QString& windowTitle,
                          const QPixmap& pixmap)
{
    // TODO: this detection is not perfect because KNotify may never be started
    //       because we never try, but at least we get notifications in any case
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.knotify"))
    {
        KPassivePopup::message(windowTitle, message, widget);
    }
    else
    {
        QPixmap logoPixmap = pixmap;
        if (logoPixmap.isNull())
        {
            logoPixmap = QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"));
        }
            
        if (eventId.isEmpty())
        {
            KNotification::event(KNotification::Notification, message, logoPixmap, widget);
        }
        else
        {
            KNotification::event(eventId, message, logoPixmap, widget);
        }
    }
}

} /* Digikam */

