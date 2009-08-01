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

    bool    autoDetected()   const;
    QString mode()           const;
    QString cameraName()     const;
    QString fullCameraName() const;
    QString vendor()         const;
    QString product()        const;

    void    setAutoDetected(bool v);
    void    setMode(const QString& mode);
    void    setVendor(const QString& vendor);
    void    setProduct(const QString& product);

    static  bool    autoDetected(const QString& name);
    static  QString mode(const QString& name);
    static  QString cameraName(const QString& name);
    static  QString fullCameraName(const QString& name);
    static  QString vendor(const QString& name);
    static  QString product(const QString& name);

    static  QString createName(const QString& vendor, const QString& product,
                               const QString& mode,   bool autoDetect = false);

private:

    bool isValidCameraName(const QString& cameraName) const;

private:

    CameraNameHelperPriv* const d;
};

}

#endif /* CAMERANAMEHELPER_H */
