/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the freerotation tool
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

#ifndef DIMGFREEROTATIONTEST_H
#define DIMGFREEROTATIONTEST_H

// Qt includes

#include <QObject>

class DImgFreeRotationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCalculateAngle();
    void testCalculateAngle_data();

private:

    double myRound(double val, int accuracy);
};

#endif /* DIMGFREEROTATIONTEST_H_ */
