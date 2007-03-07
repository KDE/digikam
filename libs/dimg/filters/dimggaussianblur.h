/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-17-07
 * Description : A Gaussian Blur threaded image filter.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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
  
#ifndef DIMGGAUSSIAN_BLUR_H
#define DIMGGAUSSIAN_BLUR_H

// Digikam Includes.

#include "digikam_export.h"

// Local includes.

#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT DImgGaussianBlur : public DImgThreadedFilter
{

public:
    
    DImgGaussianBlur(DImg *orgImage, QObject *parent=0, int radius=3);

    // Constructor for slave mode: execute immediately in current thread with specified master filter
    DImgGaussianBlur(DImgThreadedFilter *parentFilter, const DImg &orgImage, const DImg &destImage,
                     int progressBegin=0, int progressEnd=100, int radius=3);
    
    ~DImgGaussianBlur(){};
    
private:  // Gaussian blur filter data.

    int m_radius;
    
private:  // Gaussian blur filter methods.

    virtual void filterImage(void);
    
    void gaussianBlurImage(uchar *data, int width, int height, bool sixteenBit, int radius);

    // function to allocate a 2d array   
    int** Alloc2DArray (int Columns, int Rows)
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
    void Free2DArray (int** lpcArray, int Columns)
    {
       // loop to dealocate the columns
       for (int i = 0; i < Columns; i++)
           delete [] lpcArray[i];

       // now, we delete the main pointer
       delete [] lpcArray;
    };
       
    inline bool IsInside (int Width, int Height, int X, int Y)
    {
       bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
       bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
       return (bIsWOk && bIsHOk);
    };
};    

}  // NameSpace Digikam

#endif /* DIMGGAUSSIAN_BLUR_H */
