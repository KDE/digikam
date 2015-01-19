/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-01-19
 * Description : message box notification settings
 *
 * Copyright (C) 2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dmsgboxnotification.h"

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

namespace Digikam
{

bool DMsgBoxNofification::readMsgBoxShouldBeShown(const QString& dontShowAgainName)
{
    if (dontShowAgainName.isEmpty()) return false;

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group("Notification Messages");
    bool value              = group.readEntry(dontShowAgainName, false);

    return value;
}
    
void DMsgBoxNofification::saveMsgBoxShouldBeShown(const QString& dontShowAgainName, bool value)
{
    if (dontShowAgainName.isEmpty()) return;

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group("Notification Messages");
    
    group.writeEntry(dontShowAgainName, value);
    config->sync();
}

}  // namespace Digikam
