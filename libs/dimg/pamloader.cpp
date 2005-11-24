/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-22-11
 * Description : A NetPbm IO file for DImg framework
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

// C ansi includes.

extern "C" 
{
#include <pam.h>
}

// QT includes.

#include <qfile.h>
#include <qimage.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "pamloader.h"

PAMLoader::PAMLoader(DImg* image)
         : DImgLoader(image)
{
}

bool PAMLoader::load(const QString& filePath)
{
    struct pam inpam;   
        
    FILE *file = fopen(QFile::encodeName(filePath), "rb");
    if (!file)
    {
        kdWarning() << k_funcinfo << "Cannot open image file." << endl;
        return false;
    }
    
    pnm_readpaminit(file, &inpam, PAM_STRUCT_SIZE(tuple_type));

    if (inpam.bytes_per_sample == 1) 
    {
        kdWarning() << "PPM in 8 bits/color/pixel." << endl;
        m_sixteenBit = false;
    } 
    else if (inpam.bytes_per_sample == 2) 
    {
        kdWarning() << "PPM in 16 bits/color/pixel." << endl;
        m_sixteenBit = true;
    }
    else 
    {
        kdWarning() << "Unsupported PPM file bytes depth." << endl;
        return false;
    }

    uchar *data  = 0;

    if (m_sixteenBit)                        
        data = new uchar[inpam.width*inpam.height*8];  // 16 bits/color/pixel
    else 
        data = new uchar[inpam.width*inpam.height*4];  // 8 bits/color/pixel
    
    tuple *tuplerow = pnm_allocpamrow(&inpam);      
    int i;    
    uchar *dst = data;
        
    for (int row = 0; row < inpam.height; row++) 
    {
        i=0;
        pnm_readpamrow(&inpam, tuplerow);
        
        for (int col = 0; col < inpam.width; col++) 
        {
            if (inpam.depth == 3)
            {
                sample *s = tuplerow[col];
                
                dst[i++] = s[PAM_BLU_PLANE];
                dst[i++] = s[PAM_GRN_PLANE];
                dst[i++] = s[PAM_RED_PLANE];
                dst[i++] = 0xFF;
            }
        }
    }
    
    pnm_freepamrow(tuplerow);        
        
    //----------------------------------------------------------

    imageWidth()  = inpam.width;
    imageHeight() = inpam.height;
    imageData()   = data;

    return true;
}

bool PAMLoader::save(const QString& /*filePath*/)
{
    return false;
}
