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
#define CLAMP0255(a)  QMIN(QMAX(a,0), 255) 

// C++ includes.

#include <cmath>

// Digikam Includes.

#include "digikam_export.h"

// Big/little endian detection from .configure script.

#include "config.h"

namespace Digikam
{

class DIGIKAMIMAGEFILTERS_EXPORT ImageFilters
{
public: // Structures to use for color management filters depending of architectures.

// FIXME remove this endianness rule when all image plugins will support 16 bits images.

#ifdef WORDS_BIGENDIAN  // PPC like
    struct channels
    {
    uchar   alpha;
    uchar   red;
    uchar   green;
    uchar   blue;
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

    struct int_packet
    {
    unsigned int red;
    unsigned int green;
    unsigned int blue;
    unsigned int alpha;
    };

    struct NormalizeParam
    {
    unsigned short *lut;
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

    // Methods for Channel Mixer.   
       
    static inline double CalculateNorm(float RedGain, float GreenGain, float BlueGain, bool bPreserveLum)
    {
       double lfSum = RedGain + GreenGain + BlueGain;

       if ((lfSum == 0.0) || (bPreserveLum == false))
           return (1.0);

       return( fabs (1.0 / lfSum) );
    };

    static inline unsigned short MixPixel(float RedGain, float GreenGain, float BlueGain,
                                          unsigned short R, unsigned short G, unsigned short B, bool sixteenBit,
                                          double Norm, bool overIndicator=false)
    {
       double lfMix = RedGain * (double)R + GreenGain * (double)G + BlueGain * (double)B;
       lfMix *= Norm;
       int segment = sixteenBit ? 65535 : 255;
       
       if (overIndicator && lfMix > segment) lfMix = 0;
       return( (unsigned short)CLAMP (lfMix, 0, segment) );
    };
       
    static inline int setPositionAdjusted (int Width, int Height, int X, int Y)
    {
       X = (X < 0) ? 0 : (X >= Width ) ? Width  - 1 : X;
       Y = (Y < 0) ? 0 : (Y >= Height) ? Height - 1 : Y;
       return (Y*Width*4 + 4*X);
    };

public:   // Public methods.

    static void equalizeImage(uchar *data, int w, int h, bool sixteenBit);
    static void stretchContrastImage(uchar *data, int w, int h, bool sixteenBit);
    static void normalizeImage(uchar *data, int w, int h, bool sixteenBit);
    static void autoLevelsCorrectionImage(uchar *data, int w, int h, bool sixteenBit);
    static void invertImage(uchar *data, int w, int h, bool sixteenBit);
    static void channelMixerImage(uchar *data, int Width, int Height, bool sixteenBit,
                                  bool bPreserveLum, bool bMonochrome,
                                  float rrGain, float rgGain, float rbGain,
                                  float grGain, float ggGain, float gbGain,
                                  float brGain, float bgGain, float bbGain,
                                  bool overIndicator=false);
    static void changeTonality(uchar *data, int width, int height, bool sixteenBit, int redMask, int greenMask, int blueMask);

    // FIXME : support 16 bits images.
    static void gaussianBlurImage(uint *data, int Width, int Height, int Radius);
    static void sharpenImage(uint* data, int w, int h, int r);
    static void hueSaturationLightnessImage(uint* data, int w, int h, double hu, double sa, double li);
    
    static void pixelAntiAliasing(uchar *data, int Width, int Height, double X, double Y, 
                                  uchar *A, uchar *R, uchar *G, uchar *B);       
};

}  // NameSpace Digikam

#endif /* IMAGE_FILTERS_H */
