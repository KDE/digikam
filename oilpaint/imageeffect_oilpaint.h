/* ============================================================
 * File  : imageeffect_oilpaint.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-25
 * Description : a Digikam image editor plugin for to simulate 
 *               an oil painting.
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

#ifndef IMAGEEFFECT_OILPAINT_H
#define IMAGEEFFECT_OILPAINT_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QSpinBox;
class QSlider;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamOilPaintImagesPlugin
{

class ImageEffect_OilPaint : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_OilPaint(QWidget* parent);
    ~ImageEffect_OilPaint();

protected:

    void closeEvent(QCloseEvent *e);
    
private:

    bool          m_cancel;
    
    QWidget      *m_parent;
    
    QPushButton  *m_helpButton;
    
    QSpinBox     *m_brushSizeInput;
    QSpinBox     *m_smoothInput;
        
    QSlider      *m_brushSizeSlider;
    QSlider      *m_smoothSlider;
    
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private:

    void OilPaint(uint* data, int w, int h, int BrushSize, int Smoothness);
    
    inline uint MostFrequentColor(uchar* Bits, int Width, int Height, int X, 
                                  int Y, int Radius, int Intensity); 
                                  
    // Function to calcule the color intensity and return the luminance (Y)
    // component of YIQ color model.
    inline uint GetIntensity(uint Red, uint Green, uint Blue)
           { return ((uint)(Red * 0.3 + Green * 0.59 + Blue * 0.11)); } 
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    
};

}  // NameSpace DigikamOilPaintImagesPlugin

#endif /* IMAGEEFFECT_OILPAINT_H */
