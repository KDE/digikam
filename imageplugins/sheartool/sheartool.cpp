/* ============================================================
 * File  : sheartool.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-18
 * Description : Shear tool threaded image filter.
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
 
#define DEGREES_TO_RADIANS(deg) ((deg) * 3.141592653589793238462 / 180.0)
 
// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qwmatrix.h> 

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "sheartool.h"

namespace DigikamShearToolImagesPlugin
{

ShearTool::ShearTool(QImage *orgImage, QObject *parent, float hAngle, float vAngle,
                     bool antialiasing, int orgW, int orgH)
            : Digikam::ThreadedFilter(orgImage, parent, "ShearTool")
{ 
    m_hAngle    = hAngle;
    m_vAngle    = vAngle;
    m_orgW      = orgW;
    m_orgH      = orgH;
    m_antiAlias = antialiasing;
        
    initFilter();
}

void ShearTool::filterImage(void)
{
    // TODO: using new shear tool algorithm with antialiasing filter.     
    QWMatrix matrix;
    matrix.shear( tan(DEGREES_TO_RADIANS(m_hAngle) ), tan(DEGREES_TO_RADIANS(m_vAngle) ));
    m_destImage = m_orgImage.xForm(matrix);
    m_newSize = matrix.mapRect(QRect::QRect(0, 0, m_orgW, m_orgH)).size();
}

}  // NameSpace DigikamShearToolImagesPlugin
