/* ============================================================
 * File  : texture.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : BLur FX threaded image filter.
 *
 * Copyright 2005 by Gilles Caulier
 * 
 * Original Blur algorithms copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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
  
#ifndef BLURFX_H
#define BLURFX_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamBlurFXImagesPlugin
{

class BlurFX : public Digikam::ThreadedFilter
{

public:
    
    BlurFX(QImage *orgImage, QObject *parent=0, int blurFXType=ZoomBlur,
           int distance=100, int level=45);
    
    ~BlurFX(){};

public:
    
    enum BlurFXTypes 
    {
    ZoomBlur=0,
    RadialBlur,
    FarBlur,
    MotionBlur,
    SoftenerBlur,
    ShakeBlur,
    FocusBlur,
    SmartBlur,
    FrostGlass,
    Mosaic
    };

private:  // BlurFX filter data.

    int m_blurFXType;
    int m_distance;
    int m_level;
    
private:  // BlurFX filter methods.

    virtual void filterImage(void);
    
    // Backported from ImageProcessing version 1
    void softenerBlur(uchar *data, int Width, int Height);
    void shakeBlur(uchar *data, int Width, int Height, int Distance);
    void frostGlass(uchar *data, int Width, int Height, int Frost);
    
    // Backported from ImageProcessing version 2
    void zoomBlur(uchar *data, int Width, int Height, int X, int Y, int Distance, QRect pArea=QRect::QRect());
    void radialBlur(uchar *data, int Width, int Height, int X, int Y, int Distance, QRect pArea=QRect::QRect());
    void focusBlur(uchar *data, int Width, int Height, int X, int Y, int BlurRadius, int BlendRadius, 
                   bool bInversed=false, QRect pArea=QRect::QRect());
    void farBlur(uchar *data, int Width, int Height, int Distance);
    void motionBlur(uchar *data, int Width, int Height, int Distance, double Angle=0.0);
    void smartBlur(uchar *data, int Width, int Height, int Radius, int Strenght);
    void mosaic(uchar *data, int Width, int Height, int SizeW, int SizeH);

private:  // Internal filter methods.

    void MakeConvolution(uchar *data, int Width, int Height, int Radius, int Kernel[]);
    
    inline QRgb RandomColor(uchar *Bits, int Width, int Height, int X, int Y, int Radius);
    
    // Return the limit defined the max and min values.
    inline int Lim_Max(int Now, int Up, int Max) 
       {
       --Max; 
       while (Now > Max - Up) --Up; 
       return (Up); 
       };
    
    // Return the luminance (Y) component of YIQ color model.
    inline uchar GetIntensity (uchar R, uchar G, uchar B) 
       {
       return ((uchar)(R * 0.3 + G * 0.59 + B * 0.11)); 
       };

    inline int GetStride (int Width)
       { 
       int LineWidth = Width * 4;
       if (LineWidth % 4) return (4 - (LineWidth % 4)); 
       return (0); 
       };
              
    // function to allocate a 2d array   
    inline int** Alloc2DArray (int Columns, int Rows)
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
    inline void Free2DArray (int** lpcArray, int Columns)
       {
       // loop to dealocate the columns
       for (int i = 0; i < Columns; i++)
           delete [] lpcArray[i];

       // now, we delete the main pointer
       delete [] lpcArray;
       }   
       
    inline bool IsInside (int Width, int Height, int X, int Y)
       {
       bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
       bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
       return (bIsWOk && bIsHOk);
       };
       
    // A color is represented in RGB value (e.g. 0xFFFFFF is white color). 
    // But R, G and B values has 256 values to be used so, this function analize 
    // the value and limits to this range.
    inline uchar LimitValues (int ColorValue)
       {
       if (ColorValue > 255) ColorValue = 255;        
       if (ColorValue < 0) ColorValue = 0;
       return ((uchar) ColorValue);
       };                      
    
    inline int GetLineWidth (int Width)
       {
       return ((Width * 4) + GetStride (Width)); 
       };
              
    inline int SetPosition (int Width, int X, int Y)
       {
       return (Y * GetLineWidth(Width) + 4 * X); 
       };

    inline int SetPositionAdjusted (int Width, int Height, int X, int Y)
       {
       X = (X < 0) ? 0 : (X >= Width ) ? Width  - 1 : X;
       Y = (Y < 0) ? 0 : (Y >= Height) ? Height - 1 : Y;
       return (Y * GetLineWidth(Width) + 4 * X);
       };

    inline bool IsColorInsideTheRange (uchar cR, uchar cG, uchar cB, 
                                       uchar nR, uchar nG, uchar nB, 
                                       int Range)
       {
       if ((nR >= cR - Range) && (nR <= cR + Range))
           if ((nG >= cG - Range) && (nG <= cG + Range))
               if ((nB >= cB - Range) && (nB <= cB + Range))
                   return (true);

       return (false);
       };    
    
};    

}  // NameSpace DigikamBlurFXImagesPlugin

#endif /* BLURFX_H */
