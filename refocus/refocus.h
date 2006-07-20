/* ============================================================
 * File  : refocus.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Refocus threaded image filter.
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
  
#ifndef REFOCUS_H
#define REFOCUS_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamRefocusImagesPlugin
{

class Refocus : public Digikam::ThreadedFilter
{

public:
    
    Refocus(QImage *orgImage, QObject *parent=0, int matrixSize=5, double radius=0.9, 
            double gauss=0.0, double correlation=0.5, double noise=0.01);
    
    ~Refocus(){};

private:  // Refocus filter data.

    int m_matrixSize;
    
    double m_radius;
    double m_gauss;
    double m_correlation;
    double m_noise;
    
private:  // Refocus filter methods.

    virtual void filterImage(void);
    
    void refocusImage(const uint* data, int width, int height, int matrixSize, 
                      double radius, double gauss, double correlation, double noise);
                               
    void convolveImage(const uint *orgData, uint *destData, int width, int height, 
                       const double *const mat, int mat_size);
    
};    

}  // NameSpace DigikamRefocusImagesPlugin

#endif /* REFOCUS_H */
