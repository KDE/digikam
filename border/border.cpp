/* ============================================================
 * File  : border.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : border threaded image filter.
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

#include <qpixmap.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "border.h"

namespace DigikamBorderImagesPlugin
{

Border::Border(QImage *orgImage, QObject *parent, int orgWidth, int orgHeight,
               QString borderPath, int borderType, 
               int borderWidth1, int borderWidth2, int borderWidth3, int borderWidth4,
               QColor solidColor, 
               QColor niepceBorderColor,
               QColor niepceLineColor, 
               QColor bevelUpperLeftColor,
               QColor bevelLowerRightColor, 
               QColor decorativeFirstColor,
               QColor decorativeSecondColor)
      : Digikam::ThreadedFilter(orgImage, parent, "Border")
{ 
    m_orgWidth     = orgWidth;
    m_orgHeight    = orgHeight;

    m_borderType   = borderType;
    m_borderWidth1 = borderWidth1;
    m_borderWidth2 = borderWidth2;
    m_borderWidth3 = borderWidth3;
    m_borderWidth4 = borderWidth4;
    
    m_solidColor            = solidColor;
    m_niepceBorderColor     = niepceBorderColor;
    m_niepceLineColor       = niepceLineColor;
    m_bevelUpperLeftColor   = bevelUpperLeftColor;
    m_bevelLowerRightColor  = bevelLowerRightColor;
    m_decorativeFirstColor  = decorativeFirstColor;
    m_decorativeSecondColor = decorativeSecondColor;
    
    m_borderPath = borderPath;
    
    initFilter();
}

void Border::filterImage(void)
{
    switch (m_borderType)
       {
       case SolidBorder:
          solid(m_orgImage, m_destImage, m_solidColor, m_borderWidth1);
          break;
       
       case NiepceBorder:
          niepce(m_orgImage, m_destImage, m_niepceBorderColor, m_borderWidth1, 
                 m_niepceLineColor, m_borderWidth4);
          break;

       case BeveledBorder:
          bevel(m_orgImage, m_destImage, m_bevelUpperLeftColor,
                m_bevelLowerRightColor, m_borderWidth1);
          break;

       case PineBorder:
       case WoodBorder: 
       case PaperBorder: 
       case ParqueBorder: 
       case IceBorder:
       case LeafBorder: 
       case MarbleBorder:
       case RainBorder:
       case CratersBorder:
       case DriedBorder:
       case PinkBorder:
       case StoneBorder:
       case ChalkBorder:
       case GraniteBorder:
       case RockBorder:
       case WallBorder:
          pattern( m_orgImage, m_destImage, m_borderWidth1, 
                   m_decorativeFirstColor, m_decorativeSecondColor,
                   m_borderWidth2, m_borderWidth2 );
          break;
       }


/*    // Border tile.

    int w = m_orgImage.width();
    int h = m_orgImage.height();
    kdDebug() << "Border File:" << m_texturePath << endl;
    QImage texture(m_texturePath);
    if ( texture.isNull() ) return;
    
    m_textureImg.create(w, h, 32);
    
    for (int x = 0 ; x < w ; x+=texture.width())
       for (int y = 0 ; y < h ; y+=texture.height())
          bitBlt(&m_textureImg, x, y, &texture, 0, 0, texture.width(), texture.height(), 0);

    // Apply texture.
                        
    uint* data         = (uint*)m_orgImage.bits();
    uint* pTeData      = (uint*)m_textureImg.bits();
    uint* pOutBits     = (uint*)m_destImage.bits(); 
    uint* pTransparent = new uint[w*h];    
    memset(pTransparent, 128, w*h*sizeof(uint));

    Digikam::ImageFilters::imageData teData;    
    Digikam::ImageFilters::imageData transData;    
    Digikam::ImageFilters::imageData inData;  
    Digikam::ImageFilters::imageData outData;  
    
    register int i;
    int progress;

    // Make textured transparent layout.
    
    for (i = 0; !m_cancel && (i < w*h); i++)
        {
        // Get Alpha channel (unchanged).
        teData.raw           = pTeData[i];   
        
        // Overwrite RGB.
        teData.channel.red   = (teData.channel.red * (255 - m_blendGain) + 
                                transData.channel.red * m_blendGain) >> 8;
        teData.channel.green = (teData.channel.green * (255 - m_blendGain) + 
                                transData.channel.green * m_blendGain) >> 8;
        teData.channel.blue  = (teData.channel.blue * (255 - m_blendGain) + 
                                transData.channel.blue * m_blendGain) >> 8;
        pTeData[i]           = teData.raw; 

        // Update de progress bar in dialog.
        progress = (int) (((double)i * 50.0) / (w*h));
        
        if (progress%5 == 0)
           postProgress(progress);
        }
            
    uint tmp, tmpM;

    // Merge layout and image using overlay method.
    
    for (i = 0; !m_cancel && (i < w*h); i++)
        {     
        inData.raw            = data[i];
        outData.raw           = pOutBits[i];
        teData.raw            = pTeData[i];
        outData.channel.red   = INT_MULT(inData.channel.red, inData.channel.red + 
                                            INT_MULT(2 * teData.channel.red, 
                                                    255 - inData.channel.red, tmpM), tmp);
        outData.channel.green = INT_MULT(inData.channel.green, inData.channel.green + 
                                            INT_MULT(2 * teData.channel.green, 
                                                    255 - inData.channel.green, tmpM), tmp);
        outData.channel.blue  = INT_MULT(inData.channel.blue, inData.channel.blue + 
                                            INT_MULT(2 * teData.channel.blue, 
                                                    255 - inData.channel.blue, tmpM), tmp);
        outData.channel.alpha = inData.channel.alpha;
        pOutBits[i]           = outData.raw;
        
        // Update progress bar in dialog.
        progress = (int) (50.0 + ((double)i * 50.0) / (w*h));
        
        if (progress%5 == 0)
           postProgress(progress);
        }
        
    delete [] pTransparent;
*/
}

void Border::solid(QImage &src, QImage &dest, const QColor &fg, int borderWidth)
{
    dest.reset();
    dest.create(src.width() + borderWidth*2, src.height() + borderWidth*2, 32);
    dest.fill(fg.rgb());
       
    bitBlt( &dest, borderWidth, borderWidth, &src, 0, 0, src.width(), src.height());
}

void Border::niepce(QImage &src, QImage &dest, const QColor &fg, int borderWidth, 
                                const QColor &bg, int lineWidth)
{
    QImage tmp;
    solid(src, tmp, bg, lineWidth);
    solid(tmp, dest, fg, borderWidth);
}

void Border::bevel(QImage &src, QImage &dest, const QColor &topColor, 
                               const QColor &btmColor, int borderWidth)
{
    unsigned int *output;
    int x, y;
    int wc;
    
    dest.reset();
    dest.create(src.width() + borderWidth*2, src.height() + borderWidth*2, 32);
    
    // top
    
    for(y=0, wc = dest.width()-1; y < borderWidth; ++y, --wc)
       {
       output = (unsigned int *)dest.scanLine(y);
       
       for(x=0; x < wc; ++x)
          output[x] = topColor.rgb();
        
       for(;x < dest.width(); ++x)
          output[x] = btmColor.rgb();
       }
       
    // left and right
    
    for(; y < dest.height()-borderWidth; ++y)
       {
       output = (unsigned int *)dest.scanLine(y);
       
       for(x=0; x < borderWidth; ++x)
          output[x] = topColor.rgb();
       
       for(x = dest.width()-1; x > dest.width()-borderWidth-1; --x)
          output[x] = btmColor.rgb();
       }
       
    // bottom
    
    for(wc = borderWidth; y < dest.height(); ++y, --wc)
       {
       output = (unsigned int *)dest.scanLine(y);
       
       for(x=0; x < wc; ++x)
          output[x] = topColor.rgb();
          
       for(; x < dest.width(); ++x)
          output[x] = btmColor.rgb();
       }
       
    bitBlt( &dest, borderWidth, borderWidth, &src, 0, 0, src.width(), src.height());
}

void Border::pattern(QImage &src, QImage &dest, int borderWidth,
                     const QColor &firstColor, const QColor &secondColor, 
                     int firstWidth, int secondWidth)
{
    // Border tile.

    int w = m_orgWidth + borderWidth*2;
    int h = m_orgHeight + borderWidth*2;
    kdDebug() << "Border File:" << m_borderPath << endl;
    QImage border(m_borderPath);
    if ( border.isNull() ) return;
    
    QImage borderImg(w, h, 32);
    
    for (int x = 0 ; x < w ; x+=border.width())
       for (int y = 0 ; y < h ; y+=border.height())
          bitBlt(&borderImg, x, y, &border, 0, 0, border.width(), border.height(), 0);
    
    // First line around the pattern tile.
    QImage tmp2 = borderImg.smoothScale( src.width() + borderWidth*2, 
                                         src.height() + borderWidth*2 );
    
    solid(tmp2, dest, firstColor, firstWidth);                                                 
    
    // Second line around original image.
    QImage tmp;
    solid(src, tmp, secondColor, secondWidth);                                                 
    
    // Copy original image.                                                 
    bitBlt( &dest, borderWidth, borderWidth, 
            &tmp, 0, 0, tmp.width(), tmp.height());
}

}  // NameSpace DigikamBorderImagesPlugin
