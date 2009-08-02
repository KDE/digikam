/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : camera name helper class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

namespace Digikam
{

class CameraNameHelperPriv;

class CameraNameHelper
{

public:

    CameraNameHelper(const QString& cameraName);
    ~CameraNameHelper();

    void    setAutoDetected(bool v);
    void    setMode(const QString& mode);
    void    setVendor(const QString& vendor);
    void    setProduct(const QString& product);

    QString cameraName()     const;
    QString fullCameraName() const;

    // --------------------------------------------------------

//    static  bool    isAutoDetected(const QString& name);
//    static  QString mode(const QString& name);
//    static  QString cameraName(const QString& name);
    static  QString fullCameraName(const QString& name, bool autoDetected = false);

    static  QString createName(const QString& vendor,
                               const QString& product = QString(),
                               const QString& mode    = QString(),
                               bool autoDetected      = false);

private:

    static  QString autoDetectedString();

    // --------------------------------------------------------

    enum CAMERANAME_TOKENS
    {
        VendorAndProduct = 1,
        Mode,
        AutoDetected
    };

    static  QString extractCameraNameToken(const QString& cameraName, int tokenID);

private:

    CameraNameHelperPriv* const d;
};

}

#endif /* CAMERANAMEHELPER_H */
