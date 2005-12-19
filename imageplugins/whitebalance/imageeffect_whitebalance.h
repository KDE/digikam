/* ============================================================
 * File  : imageeffect_whitebalance.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-11
 * Description : a digiKam image editor plugin to correct 
 *               image white balance 
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

#ifndef IMAGEEFFECT_WHITEBALANCE_H
#define IMAGEEFFECT_WHITEBALANCE_H

// Qt include.

#include <qcolor.h>

// Local includes.

#include "imagetabdialog.h"

class QPushButton;
class QLabel;
class QComboBox;
class QPushButton;
class QCheckBox;
class QHButtonGroup;

class KDoubleNumInput;

namespace Digikam
{
class ImageGuideWidget;
class ColorGradientWidget;
class HistogramWidget;
class DColor;
}

namespace DigikamWhiteBalanceImagesPlugin
{

class ImageEffect_WhiteBalance : public DigikamImagePlugins::ImageTabDialog
{
    Q_OBJECT

public:

    ImageEffect_WhiteBalance(QWidget* parent, QString title, QFrame* banner);
    ~ImageEffect_WhiteBalance();

private:    
    
    enum HistogramScale
    {
    Linear=0,
    Logarithmic
    };
    
    enum ColorChannel
    {
    LuminosityChannel=0,
    RedChannel,
    GreenChannel,
    BlueChannel
    };
    
    enum TemperaturePreset
    {
    Lamp40W=0,
    Lamp200W,
    Sunrise,
    Tungsten,
    Neutral,
    Xenon,
    Sun,
    Flash,
    Sky,
    None
    };

    bool                          m_clipSat;
    bool                          m_overExp;
    bool                          m_WBind;

    double                        m_saturation;
    double                        m_temperature;    
    double                        m_gamma;
    double                        m_black;
    double                        m_exposition;
    double                        m_dark;
    double                        m_green;

    int                           m_BP, m_WP;
    
    uint                          m_rgbMax;
    
    float                         m_curve[65536];
    float                         m_mr, m_mg, m_mb;
        
    uchar                        *m_destinationPreviewData;

    QPushButton                  *m_pickTemperature;
    QPushButton                  *m_autoAdjustExposure;
    
    QComboBox                    *m_temperaturePresetCB;    
    QComboBox                    *m_channelCB;    
    
    QHButtonGroup                *m_scaleBG;  
        
    QCheckBox                    *m_overExposureIndicatorBox;
    
    QLabel                       *m_temperatureLabel;
    QLabel                       *m_temperaturePresetLabel;
    QLabel                       *m_darkLabel;
    QLabel                       *m_blackLabel;
    QLabel                       *m_exposureLabel;
    QLabel                       *m_gammaLabel;
    QLabel                       *m_saturationLabel;
    QLabel                       *m_greenLabel;
    
    KDoubleNumInput              *m_temperatureInput;
    KDoubleNumInput              *m_darkInput;
    KDoubleNumInput              *m_blackInput;
    KDoubleNumInput              *m_exposureInput;
    KDoubleNumInput              *m_gammaInput;
    KDoubleNumInput              *m_saturationInput;
    KDoubleNumInput              *m_greenInput;
    
    Digikam::HistogramWidget     *m_histogramWidget;
    
    Digikam::ColorGradientWidget *m_hGradient;
    
    Digikam::ImageGuideWidget    *m_previewOriginalWidget;
    Digikam::ImageGuideWidget    *m_previewTargetWidget; 

private:
        
    void setRGBmult(void);
    void setLUTv(void);
    void whiteBalance(uchar *data, int width, int height, bool sixteenBit);
    inline unsigned short pixelColor(int colorMult, int index, int value);
    
private slots:

    void slotDefault();
    void slotUser2();
    void slotUser3();
    void slotEffect();
    void slotOk();
    void slotColorSelectedFromOriginal(const Digikam::DColor &color, bool release);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);
    void slotScaleChanged(int scale);
    void slotChannelChanged(int channel);
    void slotTemperatureChanged(double temperature);
    void slotTemperaturePresetChanged(int tempPreset);
    void slotAutoAdjustExposure(void);
};

}  // NameSpace DigikamWhiteBalanceImagesPlugin

#endif /* IMAGEEFFECT_WHITEBALANCE_H */
