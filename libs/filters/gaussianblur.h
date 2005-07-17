/* ============================================================
 * File  : gaussianblur.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-17-07
 * Description : A Gaussian Blur threaded image filter.
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
  
#ifndef GAUSSIAN_BLUR_H
#define GAUSSIAN_BLUR_H

// Digikam Includes.

#include "digikam_export.h"

// Local includes.

#include "threadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT GaussianBlur : public Digikam::ThreadedFilter
{

public:
    
    GaussianBlur(QImage *orgImage, QObject *parent=0, int radius=3);
    
    ~GaussianBlur(){};
    
private:  // Gaussian blur filter data.

    int m_radius;
    
private:  // Gaussian blur filter methods.

    virtual void filterImage(void);
    
    void gaussianBlurImage(uint *data, int Width, int Height, int Radius);

    // function to allocate a 2d array   
    static inline int** Alloc2DArray (int Columns, int Rows)
       {
       // First, we declare our future 2d array to be returned
       int** lpcArray = 0L;

       // Now, we alloc the main pointer with Columns
       lpcArray = new int*[Columns];
        
       for (int i = 0; i < Columns; i++)
           lpcArray[i] = new int[Rows];

       return (lpcArray);
       };   
    
    // Function to deallocates the 2d array previously created
    static inline void Free2DArray (int** lpcArray, int Columns)
       {
       // loop to dealocate the columns
       for (int i = 0; i < Columns; i++)
           delete [] lpcArray[i];

       // now, we delete the main pointer
       delete [] lpcArray;
       };   
       
    static inline bool IsInside (int Width, int Height, int X, int Y)
       {
       bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
       bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
       return (bIsWOk && bIsHOk);
       };       
};    

}  // NameSpace Digikam

#endif /* GAUSSIAN_BLUR_H */
