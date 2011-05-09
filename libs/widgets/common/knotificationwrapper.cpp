/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : A wrapper around KNotification which uses
 *               KPassivePopup if KNotify is unavailable
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
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

#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <knotification.h>
#include <kpassivepopup.h>
#include <kdebug.h>

namespace Digikam
{

/** Re-implementation of KPassivePopup to move pop-up notification
    window on the bottom right corner of parent window. The goal is to simulate
    the position of KDE notifier pop-up from task bar if this one is not available,
    as for ex under Windows, Gnome, or using a remote connection through ssh.
 */
class NotificationPassivePopup : public KPassivePopup
{
public:

    NotificationPassivePopup(QWidget* const parent)
        : KPassivePopup(parent), m_parent(parent)
    {
    }

    void showNotification(const QString& caption, const QString& text, const QPixmap& icon)
    {
        setView(caption, text, icon);
        QPoint ppos = m_parent->pos();
        QSize psize = m_parent->frameSize();
        int offsetx = minimumSizeHint().width()  + 30;
        int offsety = minimumSizeHint().height() + 30;
        show(QPoint(ppos.x() + psize.width()  - offsetx,
                    ppos.y() + psize.height() - offsety));
    }

private:

    QWidget* m_parent;
};

// ----------------------------------------------------------------------------------------------

void KNotificationWrapper(const QString& eventId, const QString& message,
                          QWidget* const parent, const QString& windowTitle,
                          const QPixmap& pixmap)
{
    QPixmap logoPixmap = pixmap;

    if (logoPixmap.isNull())
    {
        if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
        {
            logoPixmap = QPixmap(SmallIcon("digikam"));
        }
        else
        {
            logoPixmap = QPixmap(SmallIcon("showfoto"));
        }
    }

    // TODO: this detection is not perfect because KNotify may never be started
    //       because we never try, but at least we get notifications in any case
    //       In a regular KDE session, KNotify should be running already.
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.knotify"))
    {
        NotificationPassivePopup* popup = new NotificationPassivePopup(parent);
        popup->showNotification(windowTitle, message, logoPixmap);
    }
    else
    {
        if (eventId.isEmpty())
        {
            KNotification::event(KNotification::Notification, message, logoPixmap, parent);
        }
        else
        {
            KNotification::event(eventId, message, logoPixmap, parent);
        }
    }
}

} // namespace Digikam
