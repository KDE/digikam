/* ============================================================
 * File  : imageeffect_raindrop.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-09-30
 * Description : a Digikam image plugin for to simulate 
 *               a rain droppping on an image.
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

#ifndef IMAGEEFFECT_RAINDROP_H
#define IMAGEEFFECT_RAINDROP_H

// KDE includes.

#include <kdialogbase.h>

class QPushButton;
class QSpinBox;
class QSlider;
class QTimer;

class KProgress;
class KIntNumInput;

namespace Digikam
{
class ImageWidget;
}

namespace DigikamRainDropImagesPlugin
{

class ImageEffect_RainDrop : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_RainDrop(QWidget *parent);
    ~ImageEffect_RainDrop();

protected:

    void closeEvent(QCloseEvent *e);
    
private:
    
    bool                  m_cancel;
    bool                  m_dirty;
    
    QWidget              *m_parent;
    
    QPushButton          *m_helpButton;

    QTimer               *m_timer;
            
    KIntNumInput         *m_dropInput;
    KIntNumInput         *m_amountInput;
    KIntNumInput         *m_coeffInput;    
    
    KProgress            *m_progressBar;
    
    Digikam::ImageWidget *m_previewWidget;

    void rainDrops(uint *data, int Width, int Height, int MinDropSize, int MaxDropSize, int Amount, 
                   int Coeff, bool bLimitRange, int progressMin=0, int progressMax=100);
                   
    bool CreateRainDrop(uint *data, int Width, int Height, uchar *dest, uchar* pStatusBits, int X, int Y, 
                        int DropSize, double Coeff, bool bLimitRange);
    
    bool CanBeDropped(int Width, int Height, uchar *pStatusBits, int X, int Y, int DropSize, bool bLimitRange);
    
    bool SetDropStatusBits (int Width, int Height, uchar *pStatusBits, int X, int Y, int DropSize);
                        
    // A color is represented in RGB value (e.g. 0xFFFFFF is white color). 
    // But R, G and B values has 256 values to be used so, this function analize 
    // the value and limits to this range.
    inline uchar LimitValues (int ColorValue)
       {
       if (ColorValue > 255) ColorValue = 255;        
       if (ColorValue < 0) ColorValue = 0;
       return ((uchar) ColorValue);
       };
    
    inline bool IsInside (int Width, int Height, int X, int Y)
       {
       bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
       bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);
       return (bIsWOk && bIsHOk);
       };
    
    inline int GetStride (int Width)
       { 
       int LineWidth = Width * 4;
       if (LineWidth % 4) return (4 - (LineWidth % 4)); 
       return (0); 
       };

    inline int GetLineWidth (int Width)
       {
       return ((Width * 4) + GetStride (Width)); 
       };
            
    inline int SetPosition (int Width, int X, int Y)
       {
       return (Y * GetLineWidth(Width) + 4 * X); 
       };
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotTimer();
};

}  // NameSpace DigikamRainDropImagesPlugin

#endif /* IMAGEEFFECT_RAINDROP_H */
