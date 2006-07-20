/* ============================================================
 * File  : hotpixelfixer.cpp
 * Author: Unai Garro <ugarro at users dot sourceforge dot net>
 *         Gilles Caulier <caulier dot gilles at free dot fr> 
 * Date  : 2005-03-27
 * Description : Threaded image filter to fix hot pixels
 * 
 * Copyright 2005 by Unai Garro and Gilles Caulier
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
 * ============================================================ 
 * The algorithm for fixing the hot pixels was based on
 * the code of jpegpixi, which was released under the GPL license,
 * and is Copyright (C) 2003, 2004 Martin Dickopp
 * ============================================================*/

// C++ includes. 
 
#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qcolor.h>
#include <qregexp.h>
#include <qstringlist.h>

// Local includes.

#include "hotpixelfixer.h"

#if HAVE_FLOAT_H
# include <float.h>
#endif
#ifndef DBL_MIN
# define DBL_MIN 1e-37
#endif
#ifndef DBL_MAX
# define DBL_MAX 1e37
#endif

namespace DigikamHotPixelsImagesPlugin
{

HotPixelFixer::HotPixelFixer(QImage *orgImage, QObject *parent, const QValueList<HotPixel>& hpList, 
                             int interpolationMethod)
             : Digikam::ThreadedFilter(orgImage, parent, "HotPixels")
{
    m_hpList              = hpList;
    m_interpolationMethod = interpolationMethod;
    mWeightList.clear();
    
    initFilter();
}

HotPixelFixer::~HotPixelFixer()
{
}

void HotPixelFixer::filterImage(void)
{
    QValueList <HotPixel>::ConstIterator it;
    QValueList <HotPixel>::ConstIterator end(m_hpList.end()); 
    for (it = m_hpList.begin() ; it != end ; ++it)
        {
        HotPixel hp = *it;
        interpolate(m_orgImage, hp, m_interpolationMethod);
        }
        
    m_destImage = m_orgImage;
}

// Interpolates a pixel block
void HotPixelFixer::interpolate (QImage &img, HotPixel &hp, int method)
{
    int icomp;
    int component;
        
    const int xPos = hp.x();
    const int yPos = hp.y();
    
        // Interpolate pixel.
        switch (method)
        {
        case AVERAGE_INTERPOLATION:
        {
            // We implement the bidimendional one first.
            // TODO: implement the rest of directions (V & H) here
        
            //case twodim:
            // {
            int sum_weight = 0;
            double vr=0.0,vg=0.0,vb=0.0;
            int x, y;
            QColor col;
        
            for (x = xPos; x < xPos+hp.width(); ++x)
            {
                if (validPoint(img,QPoint(x,yPos-1)))
                {
                    col=QColor(img.pixel(x,yPos-1));
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }
                if (validPoint(img,QPoint(x,yPos+hp.height())))
                {
                    col=QColor(img.pixel(x,yPos+hp.height()));
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }
            }
        
            for (y = yPos; y < hp.height(); ++y)
            {
            
                if (validPoint(img,QPoint(xPos-1,y)))
                {
                    col=QColor(img.pixel(xPos,y));
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }
                if (validPoint(img,QPoint(xPos+hp.width(),y)))
                {
                    col=QColor(img.pixel(xPos+hp.width(),y));
                    vr += col.red();
                    vg += col.green();
                    vb += col.blue();
                    ++sum_weight;
                }
            }
        
            if (sum_weight > 0)
            {
                vr /= (double)sum_weight;
                vg /= (double)sum_weight;
                vb /= (double)sum_weight;
    
                
                for (x = 0; x < hp.width(); ++x)
                for (y = 0; y < hp.height(); ++y)
                if (validPoint(img,QPoint(xPos+x,yPos+y)))
                    img.setPixel(xPos+x,yPos+y,qRgb(vr,vg,vb));
    
            }
            break;
                } //Case average

          case LINEAR_INTERPOLATION:
             //(Bi)linear interpolation.
            weightPixels (img,hp,LINEAR_INTERPOLATION,TWODIM_DIRECTION);
            break;

          case QUADRATIC_INTERPOLATION:
            // (Bi)quadratic interpolation.
             weightPixels (img,hp,QUADRATIC_INTERPOLATION,TWODIM_DIRECTION);
            break;

          case CUBIC_INTERPOLATION:
            // (Bi)cubic interpolation. 
             weightPixels (img,hp,CUBIC_INTERPOLATION,TWODIM_DIRECTION);
        } //switch
    
}

void HotPixelFixer::weightPixels (QImage &img, HotPixel &px, int method, Direction dir)
{
    //TODO: implement direction here too
        
    for (int iComp = 0; iComp < 3; ++iComp)
    {
        // Obtain weight data block.  
    
        Weights w;
        int polynomeOrder=-1;
        
        switch (method)
        {
            case AVERAGE_INTERPOLATION:  // Gilles: to prevent warnings from compiler.
                break;
            case LINEAR_INTERPOLATION:
                polynomeOrder=1;
                break;
            case QUADRATIC_INTERPOLATION:
                polynomeOrder=2;
                break;
            case CUBIC_INTERPOLATION:
                polynomeOrder=3;
                break;        
        }
        if (polynomeOrder<0) return;
        
        // In the one-dimensional case, the width must be 1,
        // and the size must be stored in height 
        
        w.setWidth(dir == TWODIM_DIRECTION ? px.width() : 1);
        w.setHeight(dir == HORIZONTAL_DIRECTION ? px.width() : px.height());
        w.setPolynomeOrder(polynomeOrder);
        w.setTwoDim(dir == TWODIM_DIRECTION);
    
        //TODO: check this, it must not recalculate existing calculated weights
        //for now I don't think it's finding the duplicates fine, so it uses
        //the previous one always...
        
        //if (mWeightList.find(w)==mWeightList.end())
        //{
            w.calculateWeights();
            
        //    mWeightList.append(w);
            
        //}
     
        // Calculate weighted pixel sum.  
        for (int y = 0; y<px.height(); ++y)
            for (int x = 0; x < px.width(); ++x)
            {
                
            if (validPoint (img,QPoint(px.x()+x,px.y()+y)))
            {
                double sum_weight = 0.0, v = 0.0;
                size_t i;
        
                for (i = 0; i < w.positions().count(); ++i)
                {
                    // In the one-dimensional case, only the y coordinate is used.  
                    const int xx = px.x()+(dir == VERTICAL_DIRECTION ? x : 
                                   dir== HORIZONTAL_DIRECTION ? w.positions()[i].y() : w.positions()[i].x());
                    const int yy = px.y()+(dir == HORIZONTAL_DIRECTION ? y : 
                                   w.positions()[i].y());
        
                    if (validPoint (img,QPoint(xx, yy)))
                    {
                        //TODO: check this. I think it's broken
                        double weight;
                        if (dir==VERTICAL_DIRECTION)
                        {
                            weight = w[i][y][0];
                        }
                        else if (dir==HORIZONTAL_DIRECTION)
                        {
                            weight=w[i][0][x];
                        }
                        else
                        {
                            weight=w[i][y][x];
                        }
                                
                        if (iComp==0) v += weight * qRed(img.pixel(xx, yy));
                        else if (iComp==1) v += weight * qGreen(img.pixel(xx, yy));
                        else v += weight * qBlue(img.pixel(xx, yy));
                        
                        sum_weight += weight;
                    }
                    } //for (i
                    
                    QColor color=img.pixel(px.x()+x,px.y()+y);
                    int component;
                    if (fabs (v) <= DBL_MIN)
                    
                    component=0;
                    else if (sum_weight >= DBL_MIN)
                    component=v/sum_weight;
                    else if (v >= 0.0)
                    component=DBL_MAX;
                    else
                    component=-DBL_MAX;
                    
                    int r,g,b; color.getRgb(&r,&g,&b);
                    if (iComp==0) r=component;
                    else if (iComp==1) g=component;
                    else b=component;
                    color.setRgb(r,g,b);
                    img.setPixel(px.x()+x,px.y()+y,color.rgb());
                    
                } //if validPoint()
            
            } //for y,x
        }// for iComp
}

}  // NameSpace DigikamHotPixelsImagesPlugin
