/* ============================================================
 * File  : imageeffect_blurfx.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-09
 * Description : 
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

#ifndef IMAGEEFFECT_BLURFX_H
#define IMAGEEFFECT_BLURFX_H

// Qt includes.

#include <qcolor.h>
#include <qrect.h>

// KDE includes.

#include <kdialogbase.h>

class QPushButton;
class QComboBox;
class QLabel;
class QTimer;
class QRect;

class KProgress;
class KIntNumInput;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamBlurFXImagesPlugin
{

class ImageEffect_BlurFX : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_BlurFX(QWidget *parent);
    ~ImageEffect_BlurFX();

protected:

    void closeEvent(QCloseEvent *e);
    
    // Backported from ImageProcessing version 1
    void softenerBlur(uint *data, int Width, int Height);
    void shakeBlur(uint *data, int Width, int Height, int Distance);
    void frostGlass(uint *data, int Width, int Height, int Frost);
    
    // Backported from ImageProcessing version 2
    void zoomBlur(uint *data, int Width, int Height, int X, int Y, int Distance, QRect pArea=QRect::QRect());
    void radialBlur(uint *data, int Width, int Height, int X, int Y, int Distance, QRect pArea=QRect::QRect());
    void focusBlur(uint *data, int Width, int Height, int X, int Y, int BlurRadius, int BlendRadius, 
                   bool bInversed=false, QRect pArea=QRect::QRect());
    void farBlur(uint *data, int Width, int Height, int Distance);
    void motionBlur(uint *data, int Width, int Height, int Distance, double Angle=0.0);
    void smartBlur(uint *data, int Width, int Height, int Radius, int Strenght);
    void mosaic(uint *data, int Width, int Height, int SizeW, int SizeH);

private:
    
    bool                         m_cancel;
    bool                         m_dirty;

    QWidget                     *m_parent;
    
    QPushButton                 *m_helpButton;

    QComboBox                   *m_effectType;
    
    QTimer                      *m_timer;
    
    QLabel                      *m_effectTypeLabel;
    QLabel                      *m_distanceLabel;
    QLabel                      *m_levelLabel;
    
    KIntNumInput                *m_distanceInput;
    KIntNumInput                *m_levelInput;
    
    KProgress                   *m_progressBar;
    
    Digikam::ImagePreviewWidget *m_previewWidget;

private:

    void MakeConvolution(uint *data, int Width, int Height, int Radius, int Kernel[]);
    
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
                                          
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotEffectTypeChanged(int type);
    void slotTimer();
};

}  // NameSpace DigikamBlurFXImagesPlugin

#endif /* IMAGEEFFECT_BLURFX_H */
