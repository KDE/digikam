/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : image curves manipulation methods.
 * 
 * Copyright 2004 by Gilles Caulier
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
 
#ifndef IMAGECURVES_H
#define IMAGECURVES_H

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

// Clamp a >>int32<<-range int between 0 and 255 inclusive 
#define CLAMP0255(a)  CLAMP(a,0,255) 

#define ROUND(x) ((int) ((x) + 0.5))

//  Map RGB to intensity  

#define GIMP_RGB_INTENSITY_RED    0.30
#define GIMP_RGB_INTENSITY_GREEN  0.59
#define GIMP_RGB_INTENSITY_BLUE   0.11
#define GIMP_RGB_INTENSITY(r,g,b) ((r) * GIMP_RGB_INTENSITY_RED   + \
                                   (g) * GIMP_RGB_INTENSITY_GREEN + \
                                   (b) * GIMP_RGB_INTENSITY_BLUE)

// KDE includes.

#include <kurl.h>
                                                                      
namespace Digikam
{

class ImageHistogram;

class ImageCurves
{

private:

enum PixelType
{
    RedPixel = 0,  
    GreenPixel,
    BluePixel, 
    AlphaPixel
};

enum CurveType
{
  CURVE_SMOOTH = 0,   // Smooth curve type
  CURVE_FREE          // Freehand curve type.
};

struct _Curves
{
  CurveType curve_type[5];
  int       points[5][17][2];
  uchar     curve[5][256];
};

struct _Lut
{
    uchar **luts;
    int     nchannels;
};

public:
    
    ImageCurves();
    ~ImageCurves();

    typedef double CRMatrix[4][4];

    // Methods for to manipulate the curves data.        
    
    void   curvesChannelReset(int channel);
    void   curvesCalculateCurve(int channel);
    float  curvesLutFunc(int n_channels, int channel, float value);
    void   curvesLutSetup(int nchannels);
    void   curvesLutProcess(uint *srcPR, uint *destPR, int w, int h);

    // Methods for to set manually the curves values.        
    /*
    void   setLevelGammaValue(int Channel, double val);
    void   setLevelLowInputValue(int Channel, int val);
    void   setLevelHighInputValue(int Channel, int val);
    void   setLevelLowOutputValue(int Channel, int val);
    void   setLevelHighOutputValue(int Channel, int val);    
    
    double getLevelGammaValue(int Channel);
    int    getLevelLowInputValue(int Channel);
    int    getLevelHighInputValue(int Channel);
    int    getLevelLowOutputValue(int Channel);
    int    getLevelHighOutputValue(int Channel);    */

    // Methods for to save/load the curves values to/from a Gimp curves text file.        
    
    bool   saveCurvesToGimpCurvesFile(KURL fileUrl);
    bool   loadCurvesFromGimpCurvesFile(KURL fileUrl);
    
private:

    // Curves data.
    struct _Curves *m_curves;
    
    // Lut data.
    struct _Lut    *m_lut;

    // Private methods.    
    void curvesPlotCurve(int channel, int p1, int p2, int p3, int p4);
    void curvesCRCompose(CRMatrix a, CRMatrix b, CRMatrix ab);

};

}  // NameSpace Digikam


#endif /* IMAGECURVES_H */
