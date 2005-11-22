/* ============================================================
 * File  : distortion.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-18
 * Description : Distortion FX threaded image filter.
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
  
#ifndef DISTORTION_FX_H
#define DISTORTION_FX_H

// Qt includes.

#include <qsize.h>

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamDistortionFXImagesPlugin
{

class DistortionFX : public Digikam::ThreadedFilter
{

public:
    
    DistortionFX(QImage *orgImage, QObject *parent=0, int effectType=0, 
                 int level=0, int iteration=0, bool antialiasing=true);
    
    ~DistortionFX(){};

public:
    
    enum DistortionFXTypes 
    {
    FishEye=0,
    Twirl,
    CilindricalHor,
    CilindricalVert,
    CilindricalHV,
    Caricature,
    MultipleCorners,
    WavesHorizontal,
    WavesVertical,
    BlockWaves1,
    BlockWaves2,
    CircularWaves1,
    CircularWaves2,
    PolarCoordinates,
    UnpolarCoordinates,
    Tile,
    Neon,
    FindEdges
    };
        
private:  

    bool   m_antiAlias;

    int    m_level;
    int    m_iteration;
    int    m_effectType;
    
private:  

    virtual void filterImage(void);
        
    // Backported from ImageProcessing version 2
    void fisheye(uchar *data, int Width, int Height, double Coeff, bool AntiAlias=true);
    void twirl(uchar *data, int Width, int Height, int Twirl, bool AntiAlias=true);
    void cilindrical(uchar *data, int Width, int Height, double Coeff, 
                     bool Horizontal, bool Vertical, bool AntiAlias=true);
    void multipleCorners(uchar *data, int Width, int Height, int Factor, bool AntiAlias=true);
    void polarCoordinates(uchar *data, int Width, int Height, bool Type, bool AntiAlias=true);
    void circularWaves(uchar *data, int Width, int Height, int X, int Y, double Amplitude, 
                       double Frequency, double Phase, bool WavesType, bool AntiAlias=true);
    
    // Backported from ImageProcessing version 1
    void waves(uchar *data, int Width, int Height, int Amplitude, int Frequency, 
               bool FillSides, bool Direction);
    void blockWaves(uchar *data, int Width, int Height, int Amplitude, int Frequency, bool Mode);
    void tile(uchar *data, int Width, int Height, int WSize, int HSize, int Random);
    void neon(uchar *data, int Width, int Height, int Intensity, int BW);
    void findEdges(uchar *data, int Width, int Height, int Intensity, int BW);
               
    inline double maximumRadius(int Height, int Width, double Angle);
    
    // This function does the same thing that ShadeColors function but using double variables.
    inline double proportionalValue (double DestValue, double SrcValue, double Shade)
       {
       if (Shade == 0.0) return DestValue;
       if (Shade == 255.0) return SrcValue;
       return ((DestValue * (255.0 - Shade) + SrcValue * Shade) / 256.0);       
       };
    
    // Return the limit defined the max and min values.
    inline int Lim_Max(int Now, int Up, int Max) 
       {
       --Max; 
       while (Now > Max - Up) --Up; 
       return (Up); 
       };
    
    inline int GetStride (int Width)
       { 
       int LineWidth = Width * 4;
       if (LineWidth % 4) return (4 - (LineWidth % 4)); 
       return (0); 
       };

    inline int setPosition (int Width, int X, int Y)
       {
       return (Y *Width*4 + 4*X); 
       };
    
    inline bool isInside (int Width, int Height, int X, int Y)
       {
       bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
       bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
       return (bIsWOk && bIsHOk);
       };
       
    inline int setPositionAdjusted (int Width, int Height, int X, int Y)
       {
       X = (X < 0) ? 0 : (X >= Width ) ? Width  - 1 : X;
       Y = (Y < 0) ? 0 : (Y >= Height) ? Height - 1 : Y;
       return (Y*Width*4 + 4*X);
       };
};    

}  // NameSpace DigikamDistortionFXImagesPlugin

#endif /* DISTORTION_FX_H */
