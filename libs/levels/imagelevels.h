/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-29
 * Description : image levels manipulation methods.
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
 
#ifndef IMAGELEVELS_H
#define IMAGELEVELS_H

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

/*  Map RGB to intensity  */

#define GIMP_RGB_INTENSITY_RED    0.30
#define GIMP_RGB_INTENSITY_GREEN  0.59
#define GIMP_RGB_INTENSITY_BLUE   0.11
#define GIMP_RGB_INTENSITY(r,g,b) ((r) * GIMP_RGB_INTENSITY_RED   + \
                                   (g) * GIMP_RGB_INTENSITY_GREEN + \
                                   (b) * GIMP_RGB_INTENSITY_BLUE)

// Qt includes.

#include <qcolor.h>
                                   
// KDE includes.

#include <kurl.h>
#include "digikam_export.h"
						
namespace Digikam
{

class ImageHistogram;

class DIGIKAM_EXPORT ImageLevels
{

private:

enum PixelType
{
    RedPixel = 0,  
    GreenPixel,
    BluePixel, 
    AlphaPixel
};

struct _Levels
{
    double  gamma[5];
    
    int     low_input[5];
    int     high_input[5];
    
    int     low_output[5];
    int     high_output[5];
    
    uchar   input[5][256]; // This is used only by the gui : rebuild the color gradient 
                           // widget colors in according with the new input levels.
};

struct _Lut
{
    uchar **luts;
    int     nchannels;
};

public:
    
    ImageLevels();
    ~ImageLevels();

    // Methods for to manipulate the levels data.        
    
    void   levelsChannelReset(int channel);
    void   levelsAuto(Digikam::ImageHistogram *hist);
    void   levelsChannelAuto(Digikam::ImageHistogram *hist, int channel);
    int    levelsInputFromColor(int channel, QColor color);    
    void   levelsBlackToneAdjustByColors(int channel, QColor color);
    void   levelsGrayToneAdjustByColors(int channel, QColor color);
    void   levelsWhiteToneAdjustByColors(int channel, QColor color);
    void   levelsCalculateTransfers();
    float  levelsLutFunc(int n_channels, int channel, float value);
    void   levelsLutSetup(int nchannels, bool overIndicator=false);
    void   levelsLutProcess(uint *srcPR, uint *destPR, int w, int h);

    // Methods for to set manually the levels values.        
    
    void   setLevelGammaValue(int Channel, double val);
    void   setLevelLowInputValue(int Channel, int val);
    void   setLevelHighInputValue(int Channel, int val);
    void   setLevelLowOutputValue(int Channel, int val);
    void   setLevelHighOutputValue(int Channel, int val);    
    
    double getLevelGammaValue(int Channel);
    int    getLevelLowInputValue(int Channel);
    int    getLevelHighInputValue(int Channel);
    int    getLevelLowOutputValue(int Channel);
    int    getLevelHighOutputValue(int Channel);    

    // Methods for to save/load the levels values to/from a Gimp levels text file.        
    
    bool   saveLevelsToGimpLevelsFile(KURL fileUrl);
    bool   loadLevelsFromGimpLevelsFile(KURL fileUrl);
    
private:

    // Levels data.
    struct _Levels *m_levels;
    
    // Lut data.
    struct _Lut    *m_lut;
};

}  // NameSpace Digikam

#endif /* IMAGELEVELS_H */
