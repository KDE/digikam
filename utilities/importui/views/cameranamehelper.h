/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : camera name helper class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef CAMERANAMEHELPER_H
#define CAMERANAMEHELPER_H

// Qt includes

#include <QString>

class QAction;

namespace Digikam
{

class CameraNameHelper
{

public:

    static QString formattedFullCameraName(const QString& name, bool autoDetected = false);
    static QString formattedCameraName(const QString& name, bool autoDetected = false);

    static QString createCameraName(const QString& vendor,
                                    const QString& product      = QString(),
                                    const QString& mode         = QString(),
                                    bool           autoDetected = false);

    static bool sameDevices(const QString& deviceA, const QString& deviceB);

private:

    enum CameraName_Tokens
    {
        VendorAndProduct = 1,
        Mode
    };

private:

    static QString extractCameraNameToken(const QString& cameraName, int tokenID);
    static QString parseAndFormatCameraName(const QString& cameraName, bool parseMode, bool autoDetected);
    static QString autoDetectedString();
    static QString prepareStringForDeviceComparison(const QString& string, int tokenID);
};

} // namespace Digikam

#endif /* CAMERANAMEHELPER_H */
