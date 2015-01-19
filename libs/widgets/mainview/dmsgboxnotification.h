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

#ifndef DMSGBOXNOTIFICATION_H
#define DMSGBOXNOTIFICATION_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMsgBoxNofification
{

public:

    /**
     * @return true if the corresponding message box should be shown.
     * @param dontShowAgainName the name that identify the message box. If
     * empty, this method return false.
     * @param result is set to the result that was chosen the last
     * time the message box was shown.
     */
    static bool readMsgBoxShouldBeShown(const QString& dontShowAgainName);
    
    /**
     * Save the fact that the message box should not be shown again.
     * @param dontShowAgainName the name that identify the message box. If
     * empty, this method does nothing.
     * @param value the value chosen in the message box to show it again next time.
     */
    static void saveMsgBoxShouldBeShown(const QString& dontShowAgainName, bool value);
};

}  // namespace Digikam

#endif  // DMSGBOXNOTIFICATION_H
