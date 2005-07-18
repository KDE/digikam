/* ============================================================
 * File  : freerotation.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-18
 * Description : Free rotation threaded image filter.
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
 
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qwmatrix.h> 

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "freerotation.h"

namespace DigikamFreeRotationImagesPlugin
{

FreeRotation::FreeRotation(QImage *orgImage, QObject *parent, double angle, int orgW, int orgH)
            : Digikam::ThreadedFilter(orgImage, parent, "FreeRotation")
{ 
    m_angle = angle;
    m_orgW  = orgW;
    m_orgH  = orgH;
        
    initFilter();
}

void FreeRotation::filterImage(void)
{
    QWMatrix matrix;
    matrix.rotate(m_angle);
    //m_destImage.fill(m_parent->colorGroup().background().rgb());
    m_destImage = m_orgImage.xForm(matrix);
    m_newSize = matrix.mapRect(QRect::QRect(0, 0, m_orgW, m_orgH)).size();
}

}  // NameSpace DigikamFreeRotationImagesPlugin
