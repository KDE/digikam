/* ============================================================
 * File  : triangle.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-01-18
 * Description : triangle geometry calculation class.
 * 
 * Copyright 2005 by Gilles Caulier
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
 
// C++ includes.

#include <cstdio>
#include <cstdlib>
#include <cmath>

// Local includes.

#include "triangle.h"

namespace DigikamPerspectiveImagesPlugin
{

Triangle::Triangle(QPoint A, QPoint B, QPoint C)
{
    m_a = distanceP2P(B, C);
    m_b = distanceP2P(A, C);
    m_c = distanceP2P(A, B);
}

float Triangle::angleABC(void)
{
    return( 57.295779513082 * acos( (m_b*m_b - m_a*m_a - m_c*m_c ) / (-2*m_a*m_c ) ) );
}

float Triangle::angleACB(void)
{
    return( 57.295779513082 * acos( (m_c*m_c - m_a*m_a - m_b*m_b ) / (-2*m_a*m_b ) ) );
}

float Triangle::angleBAC(void)
{
    return( 57.295779513082 * acos( (m_a*m_a - m_b*m_b - m_c*m_c ) / (-2*m_b*m_c ) ) );
}

float Triangle::distanceP2P(const QPoint& p1, const QPoint& p2)
{
    return(sqrt( abs( p2.x()-p1.x() ) * abs( p2.x()-p1.x() ) +
                 abs( p2.y()-p1.y() ) * abs( p2.y()-p1.y() ) ));
}

}  // NameSpace DigikamPerspectiveImagesPlugin
