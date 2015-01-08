/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : A wrapper send desktop notifications
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dnotificationwrapper.h"

// Qt includes

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QProcess>

// KDE includes

#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
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

    explicit NotificationPassivePopup(QWidget* const parent)
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

static inline bool detectKDEDesktopIsRunning()
{
    const QByteArray xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");

    if (!xdgCurrentDesktop.isEmpty())
        return (xdgCurrentDesktop.toUpper() == "KDE");

    // Classic fallbacks

    if (!qgetenv("KDE_FULL_SESSION").isEmpty())
        return true;

    return false;
}

// ----------------------------------------------------------------------------------------------

void DNotificationWrapper(const QString& eventId, const QString& message,
                          QWidget* const parent, const QString& windowTitle,
                          const QPixmap& pixmap)
{
    QPixmap logoPixmap = pixmap;

    if (logoPixmap.isNull())
    {
        if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
        {
            logoPixmap = QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                         .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        else
        {
            logoPixmap = QPixmap(KStandardDirs::locate("data", "showfoto/data/logo-showfoto.png"))
                         .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    // NOTE: This detection of KDE desktop is not perfect because KNotify may never be started.
    //       But in a regular KDE session, KNotify should be running already.

    if (detectKDEDesktopIsRunning() &&
        QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.knotify"))
    {
        kDebug() << "Event is dispatched to KDE desktop notifier";

        if (eventId.isEmpty())
        {
            KNotification::event(KNotification::Notification, message, logoPixmap, parent);
        }
        else
        {
            KNotification::event(eventId, message, logoPixmap, parent);
        }
    }

#ifdef Q_OS_DARWIN

    // OSX support

    else if (MacNativeDispatchNotify(windowTitle, message))
    {
        kDebug() << "Event is dispatched to OSX desktop notifier";
        return;
    }

#endif // Q_OS_DARWIN

    // Other Linux Desktops

    else if (QProcess::execute("notify-send",
                               QStringList() << windowTitle
                                             << message
                                             << "-a"
                                             << KGlobal::mainComponent().aboutData()->appName())
             == 0)
    {
        kDebug() << "Event is dispatched to desktop notifier through DBUS";
        return;
    }

    else
    {
        if (!parent)
        {
            kWarning() << "parent is null";
            return;
        }

        kDebug() << "Event is dispatched through a passive pop-up";

        NotificationPassivePopup* const popup = new NotificationPassivePopup(parent);
        popup->showNotification(windowTitle, message, logoPixmap);
    }
}

} // namespace Digikam
