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

#include <kiconloader.h>
#include <knotification.h>
#include <kpassivepopup.h>

namespace Digikam
{

/**
 * @brief Show a notification using KNotify, or KPassivePopup if KNotify is unavailable
 * @param eventId     Event id for this notification, KNotification::Notification
 *                    is used if this is empty. Events have to be configured in
 *                    digikam.notifyrc
 * @param message     Message to display
 * @param widget      Widget which owns the notification
 * @param windowTitle Title of the notification window (only used for KPassivePopup)
 * @param pixmap      Pixmap to show in the notification, in addition to the digikam logo.
 */
void KNotificationWrapper(const QString& eventId, const QString& message,
                          QWidget* const widget, const QString& windowTitle,
                          const QPixmap& pixmap)
{
    // TODO: this detection is not perfect because KNotify may never be started
    //       because we never try, but at least we get notifications in any case
    //       In a regular KDE session, KNotify should be running already.
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.knotify"))
    {
        KPassivePopup::message(windowTitle, message, widget);
    }
    else
    {
        if (eventId.isEmpty())
        {
            // no event id given, we need to provide a logo here:
            QPixmap logoPixmap = pixmap;
            if (logoPixmap.isNull())
            {
                logoPixmap = QPixmap(SmallIcon("digikam"));
            }

            KNotification::event(KNotification::Notification, message, logoPixmap, widget);
        }
        else
        {
            // even if pixmap is empty, a digikam logo is taken from
            // digikam.notifyrc
            KNotification::event(eventId, message, pixmap, widget);
        }
    }
}

} // namespace Digikam
