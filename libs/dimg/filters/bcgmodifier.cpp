/* ============================================================
 * File  : bcgmodifier.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-03-06
 * Description : a Brighness/Contrast/Gamma modifier methods
 *               for DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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

#define CLAMP_0_255(x)   QMAX(QMIN(x, 255), 0)
#define CLAMP_0_65535(x) QMAX(QMIN(x, 65535), 0)

// C ansi includes.

#include <stdio.h>
#include <math.h>

// Local includes.

#include "dimg.h"
#include "bcgmodifier.h"

namespace Digikam
{

BCGModifier::BCGModifier()
{
    reset();
}

BCGModifier::~BCGModifier()
{
}

bool BCGModifier::modified() const
{
    return m_modified;
}

void BCGModifier::setOverIndicator(bool overIndicator)
{
    m_overIndicator = overIndicator;
}
    
void BCGModifier::reset()
{
    // initialize to linear mapping

    for (int i=0; i<65536; i++)
    {
        m_map16[i] = i;
    }

    for (int i=0; i<256; i++)
    {
        m_map[i] = i;
    }

    m_modified      = false;
    m_overIndicator = false;
}

void BCGModifier::applyBCG(DImg& image)
{
    if (!m_modified || image.isNull())
        return;

    if (!image.sixteenBit())                    // 8 bits image.
    {
        uchar* data = (uchar*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
            if (m_map[data[0]] == -1 || m_map[data[1]] == -1 || m_map[data[2]] == -1)
            {
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
            }
            else
            {            
                data[0] = m_map[data[0]];
                data[1] = m_map[data[1]];
                data[2] = m_map[data[2]];
            }
            
            data += 4;
        }
    }
    else                                        // 16 bits image.
    {
        ushort* data = (ushort*) image.bits();

        for (uint i=0; i<image.width()*image.height(); i++)
        {
            if (m_map16[data[0]] == -1 || m_map16[data[1]] == -1 || m_map16[data[2]] == -1)
{
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
            }
            else
            {            
                data[0] = m_map16[data[0]];
                data[1] = m_map16[data[1]];
                data[2] = m_map16[data[2]];
            }

            data += 4;
        }
    }
}

void BCGModifier::setGamma(double val)
{
    val = (val < 0.01) ? 0.01 : val;
    int val2;

    for (int i=0; i<65536; i++)
    {
        val2 = (int)(pow(((double)m_map16[i] / 65535), (1 / val)) * 65535);
        if (m_overIndicator && val2 > 65535)
            m_map16[i] = -1;
        else
            m_map16[i] = CLAMP_0_65535(val2);
    }

    for (int i=0; i<256; i++)
    {
        val2 = (int)(pow(((double)m_map[i] / 255), (1 / val)) * 255);
        if (m_overIndicator && val2 > 255)
            m_map[i] = -1;
        else
            m_map[i] = CLAMP_0_255(val2);
    }
    
    m_modified = true;
}

void BCGModifier::setBrightness(double v)
{
    int val = (int)(v * 65535);
    int val2;

    for (int i = 0; i < 65536; i++)
    {
        val2 = m_map16[i] + val;
        if (m_overIndicator && val2 > 65535)
            m_map16[i] = -1;
        else
            m_map16[i] = CLAMP_0_65535(val2);
    }

    val = (int)(v * 255);
    
    for (int i = 0; i < 256; i++)
    {
        val2 = m_map[i] + val;
        if (m_overIndicator && val2 > 255)
            m_map[i] = -1;
        else
            m_map[i] = CLAMP_0_255(val2);
    }
    
    m_modified = true;
}

void BCGModifier::setContrast(double val)
{
    int val2;

    for (int i = 0; i < 65536; i++)
    {
        val2 = (int)(((double)m_map16[i] - 32767) * val) + 32767;
        if (m_overIndicator && val2 > 65535)
            m_map16[i] = -1;
        else
            m_map16[i] = CLAMP_0_65535(val2);
    }                                 

    for (int i = 0; i < 256; i++)
    {
        val2 = (int)(((double)m_map[i] - 127) * val) + 127;
        if (m_overIndicator && val2 > 255)
            m_map[i] = -1;
        else
            m_map[i] = CLAMP_0_255(val2);
    }
    
    m_modified = true;
}

}  // NameSpace Digikam
