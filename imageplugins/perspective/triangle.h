/* ============================================================
 * File  : triangle.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-01-18
 * Description : triangle geometry calculation class.
 * 
 * Copyright 2005 Gilles Caulier
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

// Qt includes.

#include <qpoint.h>

namespace DigikamPerspectiveImagesPlugin
{

class Triangle
{
public:

    Triangle(QPoint A, QPoint B, QPoint C);
    ~Triangle(){};
    
    float angleABC(void);
    float angleACB(void);
    float angleBAC(void);
    
private:  
    
    float  m_a;
    float  m_b;
    float  m_c;
        
    float distanceP2P(const QPoint& p1, const QPoint& p2);
};

}  // NameSpace DigikamPerspectiveImagesPlugin

#endif /* TRIANGLE_H */
