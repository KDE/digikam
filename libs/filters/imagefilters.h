/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-24-01
 * Description : image filters methods. 
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#ifndef IMAGE_FILTERS_H
#define IMAGE_FILTERS_H

namespace Digikam
{

class ImageFilters
{

private:    // Private structures used internally.

struct double_packet
    {
    double red;
    double green;
    double blue;
    double alpha;
    };

struct short_packet
    {
    unsigned short int red;
    unsigned short int green;
    unsigned short int blue;
    unsigned short int alpha;
    };

struct NormalizeParam 
    {
    uchar  lut[256];
    double min;
    double max;
    };

private:    // Private methods used internally.
    
    static inline int GetStride (int Width)
       { 
       int LineWidth = Width * 4;
       if (LineWidth % 4) return (4 - (LineWidth % 4)); 
       return (0); 
       };

    // function to allocate a 2d array   
    static inline int** Alloc2DArray (int Columns, int Rows)
       {
       // First, we declare our future 2d array to be returned
       int** lpcArray = NULL;

       // Now, we alloc the main pointer with Columns
       lpcArray = new int*[Columns];
        
       for (int i = 0; i < Columns; i++)
           lpcArray[i] = new int[Rows];

       return (lpcArray);
       }   
    
    // Function to deallocates the 2d array previously created
    static inline void Free2DArray (int** lpcArray, int Columns)
       {
       // loop to dealocate the columns
       for (int i = 0; i < Columns; i++)
           delete [] lpcArray[i];

       // now, we delete the main pointer
       delete [] lpcArray;
       }   
       
    static inline bool IsInside (int Width, int Height, int X, int Y)
       {
       bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
       bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
       return (bIsWOk && bIsHOk);
       };       

    // A color is represented in RGB value (e.g. 0xFFFFFF is white color). 
    // But R, G and B values has 256 values to be used so, this function analize 
    // the value and limits to this range.
    static inline uchar LimitValues (int ColorValue)
       {
       if (ColorValue > 255) ColorValue = 255;        
       if (ColorValue < 0) ColorValue = 0;
       return ((uchar) ColorValue);
       };        

public:   // Public methods.

    static void equalizeImage(uint *data, int w, int h);
    static void stretchContrastImage(uint *data, int w, int h);
    static void normalizeImage(uint *data, int w, int h);
    static void autoLevelsCorrectionImage(uint *data, int w, int h);
    static void invertImage(uint *data, int w, int h);
    static void smartBlurImage(uint *data, int Width, int Height);
    static void gaussianBlurImage(uint *data, int Width, int Height, int Radius);
};

}  // NameSpace Digikam

#endif /* IMAGE_FILTERS_H */
