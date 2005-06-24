/* ============================================================
 * File  : imageeffect_distortionfx.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-11
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

#ifndef IMAGEEFFECT_DISTORTIONFX_H
#define IMAGEEFFECT_DISTORTIONFX_H

// Qt includes.

#include <qcolor.h>

// KDE includes.

#include <kdialogbase.h>

class QPushButton;
class QComboBox;
class QLabel;
class QTimer;

class KProgress;
class KIntNumInput;

namespace Digikam
{
class ImageWidget;
}

namespace DigikamDistortionFXImagesPlugin
{

class ImageEffect_DistortionFX : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_DistortionFX(QWidget *parent);
    ~ImageEffect_DistortionFX();

protected:

    void closeEvent(QCloseEvent *e);
    
    // Backported from ImageProcessing version 2
    void fisheye(uint *data, int Width, int Height, double Coeff, bool AntiAlias=true);
    void twirl(uint *data, int Width, int Height, int Twirl, bool AntiAlias=true);
    void cilindrical(uint *data, int Width, int Height, double Coeff, bool Horizontal, bool Vertical, bool AntiAlias=true);
    void multipleCorners(uint *data, int Width, int Height, int Factor, bool AntiAlias=true);
    void polarCoordinates(uint *data, int Width, int Height, bool Type, bool AntiAlias=true);
    void circularWaves(uint *data, int Width, int Height, int X, int Y, double Amplitude, 
                       double Frequency, double Phase, bool WavesType, bool AntiAlias=true);
    
    // Backported from ImageProcessing version 1
    void waves(uint *data, int Width, int Height, int Amplitude, int Frequency, bool FillSides, bool Direction);
    void blockWaves(uint *data, int Width, int Height, int Amplitude, int Frequency, bool Mode);
    void tile(uint *data, int Width, int Height, int WSize, int HSize, int Random);
    void neon(uint *data, int Width, int Height, int Intensity, int BW);
    void findEdges(uint *data, int Width, int Height, int Intensity, int BW);
               
private:
    
    bool                  m_cancel;
    bool                  m_dirty;

    QWidget              *m_parent;
    
    QPushButton          *m_helpButton;

    QComboBox            *m_effectType;

    QTimer               *m_timer;
    
    QLabel               *m_effectTypeLabel;
    QLabel               *m_levelLabel;
    QLabel               *m_iterationLabel;
    
    KProgress            *m_progressBar;
    
    KIntNumInput         *m_levelInput;
    KIntNumInput         *m_iterationInput;

    Digikam::ImageWidget *m_previewWidget;
    
private:    
    
    inline double MaximumRadius(int Height, int Width, double Angle);
    inline void AntiAliasing (uint *data, int Width, int Height, double X, double Y, uchar *R, uchar *G, uchar *B);
    
    inline int GetLineWidth (int Width)
       {
       return ((Width * 4) + GetStride (Width)); 
       };
    
    // This function does the same thing that ShadeColors function but using double variables.
    inline double ProportionalValue (double DestValue, double SrcValue, double Shade)
       {
       if (Shade == 0.0) return DestValue;
       if (Shade == 255.0) return SrcValue;
       return ((DestValue * (255.0 - Shade) + SrcValue * Shade) / 256.0);       
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

    inline int SetPositionAdjusted (int Width, int Height, int X, int Y)
       {
       X = (X < 0) ? 0 : (X >= Width ) ? Width  - 1 : X;
       Y = (Y < 0) ? 0 : (Y >= Height) ? Height - 1 : Y;
       return (Y * GetLineWidth(Width) + 4 * X);
       };
       
    inline int SetPosition (int Width, int X, int Y)
       {
       return (Y * GetLineWidth(Width) + 4 * X); 
       };
       
    inline bool IsInside (int Width, int Height, int X, int Y)
       {
       bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
       bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
       return (bIsWOk && bIsHOk);
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

}  // NameSpace DigikamDistortionFXImagesPlugin

#endif /* IMAGEEFFECT_DISTORTIONFX_H */
