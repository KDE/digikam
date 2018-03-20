/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : A wrapper send desktop notifications
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DNOTIFICATIONWRAPPER_H
#define DNOTIFICATIONWRAPPER_H

#include <QString>
#include <QPixmap>

class QWidget;

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * @brief Show a notification using KNotify, or KPassivePopup if KNotify is unavailable
 * @param eventId     Event id for this notification, KNotification::Notification
 *                    is used if this is empty. Events have to be configured in
 *                    digikam.notifyrc
 * @param message     Message to display
 * @param parent      Widget which owns the notification
 * @param windowTitle Title of the notification window (only used for KPassivePopup)
 * @param pixmap      Pixmap to show in the notification, in addition to the digikam logo.
 */
void DIGIKAM_EXPORT DNotificationWrapper(const QString& eventId, const QString& message,
                                         QWidget* const parent, const QString& windowTitle,
                                         const QPixmap& pixmap = QPixmap());
} // namespace Digikam

#ifdef Q_OS_DARWIN

bool MacNativeDispatchNotify(const QString& summary, const QString& message);

#endif

#endif /* DNOTIFICATIONWRAPPER_H */

