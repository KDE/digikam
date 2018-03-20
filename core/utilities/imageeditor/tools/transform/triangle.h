/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-18
 * Description : triangle geometry calculation class.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TRIANGLE_H
#define TRIANGLE_H

// Qt includes

#include <QPoint>

namespace Digikam
{

class Triangle
{

public:

    Triangle(const QPoint& A, const QPoint& B, const QPoint& C);
    ~Triangle() {};

    float angleABC() const;
    float angleACB() const;
    float angleBAC() const;

private:

    float distanceP2P(const QPoint& p1, const QPoint& p2) const;

private:

    float m_a;
    float m_b;
    float m_c;
};

}  // namespace Digikam

#endif /* TRIANGLE_H */
