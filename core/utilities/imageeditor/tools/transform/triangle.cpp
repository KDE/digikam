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

#define RADIAN2DEGREE(a) (a * 57.295779513082)

#include "triangle.h"

// C++ includes

#include <cstdio>
#include <cstdlib>
#include <cmath>

namespace Digikam
{

Triangle::Triangle(const QPoint& A, const QPoint& B, const QPoint& C)
{
    m_a = distanceP2P(B, C);
    m_b = distanceP2P(A, C);
    m_c = distanceP2P(A, B);
}

float Triangle::angleABC() const
{
    return( RADIAN2DEGREE( acos( (m_b*m_b - m_a*m_a - m_c*m_c ) / (-2*m_a*m_c ) ) ) );
}

float Triangle::angleACB() const
{
    return( RADIAN2DEGREE( acos( (m_c*m_c - m_a*m_a - m_b*m_b ) / (-2*m_a*m_b ) ) ) );
}

float Triangle::angleBAC() const
{
    return( RADIAN2DEGREE( acos( (m_a*m_a - m_b*m_b - m_c*m_c ) / (-2*m_b*m_c ) ) ) );
}

float Triangle::distanceP2P(const QPoint& p1, const QPoint& p2) const
{
    return(sqrt( abs( p2.x()-p1.x() ) * abs( p2.x()-p1.x() ) +
                 abs( p2.y()-p1.y() ) * abs( p2.y()-p1.y() ) ));
}

}  // namespace Digikam
