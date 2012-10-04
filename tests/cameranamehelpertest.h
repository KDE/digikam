/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : a test for the CameraNameHelper
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

#ifndef CAMERANAMEHELPERTEST_H
#define CAMERANAMEHELPERTEST_H

// Qt includes

#include <QtCore/QObject>

class CameraNameHelperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    // core API tests
    void testCreateCameraName();
    void testCreateCameraName_data();

    void testForSameDevices();
    void testForSameDevices_data();


    // additional tests
    void testCameraNameFromGPCamera();
};

#endif /* CAMERANAMEHELPERTEST_H */
