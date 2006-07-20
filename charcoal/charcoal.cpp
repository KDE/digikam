/* ============================================================
 * File  : charcoal.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Charcoal threaded image filter.
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

// KDE includes.

#include <kdebug.h>
#include <kimageeffect.h>

// Local includes.

#include "charcoal.h"

namespace DigikamCharcoalImagesPlugin
{

Charcoal::Charcoal(QImage *orgImage, QObject *parent, double pencil, double smooth)
        : Digikam::ThreadedFilter(orgImage, parent, "Charcoal")
{ 
    m_pencil = pencil;
    m_smooth = smooth;
    
    initFilter();
}

void Charcoal::filterImage(void)
{
    // Detects edges in the image using pixel neighborhoods and an edge
    // detection mask.
    m_destImage = KImageEffect::edge(m_orgImage, m_pencil);
    postProgress(10);
           
    if (m_cancel) 
       {
       m_destImage = m_orgImage;
       return;
       }
    postProgress(20);
    
    // Blurs the image by convolving pixel neighborhoods.
#if KDE_VERSION >= 0x30200
    m_destImage = KImageEffect::blur(m_destImage, m_pencil, m_smooth);
#else
    m_destImage = KImageEffect::blur(m_destImage, m_pencil);
#endif
    postProgress(30);
    
    if (m_cancel) 
       {
       m_destImage = m_orgImage;
       return;
       }
    postProgress(40);
    
    // Normalises the pixel values to span the full range of color values.
    // This is a contrast enhancement technique.
    KImageEffect::normalize(m_destImage);
    postProgress(50);
    
    if (m_cancel) 
       {
       m_destImage = m_orgImage;
       return;
       }
    postProgress(60);
    
    // Invert the pixels values.
    m_destImage.invertPixels(false);
    postProgress(70);
    
    if (m_cancel) 
       {
       m_destImage = m_orgImage;
       return;
       }
    postProgress(80);
    
    // Convert image to grayscale.
    KImageEffect::toGray(m_destImage);
    postProgress(90);
    
    if (m_cancel) 
       {
       m_destImage = m_orgImage;
       return;
       }
    postProgress(100);    
}

}  // NameSpace DigikamCharcoalImagesPlugin
