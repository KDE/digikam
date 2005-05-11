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

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

// C++ includes.

#include <cmath>

//KDE includes.

#include <kprogress.h>
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAMIMAGEFILTERS_EXPORT ImageFilters
{
public: // Structures to use for color management filters depending of architectures.

#ifdef WORDS_BIGENDIAN  // PPC like
    struct channels
    {
    uchar   alpha;
    uchar   blue;
    uchar   green;
    uchar   red;
    };
#else                   // Intel like.
    struct channels
    {
    uchar   blue;
    uchar   green;
    uchar   red;
    uchar   alpha;
    };
#endif

    union imageData
    {
    unsigned int raw;
    channels     channel;
    };

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

    struct HSLParam 
    {
    int htransfer[256];
    int ltransfer[256];
    int stransfer[256];
    };
            
private:    // Private methods used internally.
    
    // Methods for Gaussian blur.   
    
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
       int** lpcArray = 0L;

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

    // Methods for Channel mixer.   
       
    static inline double CalculateNorm(float RedGain, float GreenGain, float BlueGain, bool bPreserveLum)
       {
       double lfSum = RedGain + GreenGain + BlueGain;

       if ((lfSum == 0.0) || (bPreserveLum == false))
           return (1.0);

       return( fabs (1.0 / lfSum) );
       }

    static inline uchar MixPixel(float RedGain, float GreenGain, float BlueGain, 
                                 uchar R, uchar G, uchar B, double Norm, 
                                 bool overIndicator=false)
       {
       double lfMix = RedGain * (double)R + GreenGain * (double)G + BlueGain * (double)B;
       lfMix *= Norm;
       
       if (overIndicator && lfMix > 255) lfMix = 0;        
       return( (uchar)CLAMP (lfMix, 0, 255) );
       }       

public:   // Public methods.

    static void equalizeImage(uint *data, int w, int h);
    static void stretchContrastImage(uint *data, int w, int h);
    static void normalizeImage(uint *data, int w, int h);
    static void autoLevelsCorrectionImage(uint *data, int w, int h);
    static void invertImage(uint *data, int w, int h);
    static void smartBlurImage(uint *data, int Width, int Height);
    static void gaussianBlurImage(uint *data, int Width, int Height, int Radius);
    static void channelMixerImage(uint *data, int Width, int Height, bool bPreserveLum, bool bMonochrome,
                                  float rrGain, float rgGain, float rbGain,
                                  float grGain, float ggGain, float gbGain,
                                  float brGain, float bgGain, float bbGain,
                                  bool overIndicator=false);
    static void changeTonality(uint *data, int width, int height, int redMask, int greenMask, int blueMask);     
    static void sharpenImage(uint* data, int w, int h, int r);
    static void hueSaturationLightnessImage(uint* data, int w, int h, double hu, double sa, double li);

};

}  // NameSpace Digikam

#endif /* IMAGE_FILTERS_H */
