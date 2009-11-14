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

#ifndef KNOTIFICATIONWRAPPER_H
#define KNOTIFICATIONWRAPPER_H

#include <QString>
#include <QPixmap>

class QWidget;

// Local includes

#include "digikam_export.h"

namespace Digikam
{
    
void DIGIKAM_EXPORT KNotificationWrapper(const QString& eventId, const QString& message,
                          QWidget* const widget, const QString& windowTitle,
                          const QPixmap& pixmap = QPixmap());
    
} // namespace Digikam

#endif /* KNOTIFICATIONWRAPPER_H */

