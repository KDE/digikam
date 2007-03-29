/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-03-11
 * Description : a digiKam image editor plugin to correct 
 *               image white balance 
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

// Digikam includes.

#include "imagedlgbase.h"

class QPushButton;
class QLabel;
class QComboBox;
class QPushButton;
class QHButtonGroup;

class KDoubleNumInput;
class KActiveLabel;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageWidget;
class DColor;
}

namespace DigikamWhiteBalanceImagesPlugin
{

class ImageEffect_WhiteBalance : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_WhiteBalance(QWidget* parent);
    ~ImageEffect_WhiteBalance();

protected:

    void finalRendering();    

private slots:

    void slotUser2();
    void slotUser3();
    void slotEffect();
    void slotColorSelectedFromOriginal(const Digikam::DColor &color);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);
    void slotScaleChanged(int scale);
    void slotChannelChanged(int channel);
    void slotTemperatureChanged(double temperature);
    void slotTemperaturePresetChanged(int tempPreset);
    void slotAutoAdjustExposure(void);
    void slotPickerColorButtonActived();    

private:

    void readUserSettings();
    void writeUserSettings();
    void resetValues();

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
        Candle=0,
        Lamp40W,
        Lamp100W,
        Lamp200W,
        Sunrise,
        StudioLamp,
        MoonLight,
        Neutral,
        DaylightD50,
        Flash,
        Sun,
        XeonLamp,
        DaylightD65,
        None
    };

    uchar                        *m_destinationPreviewData;

    int                           m_currentPreviewMode;

    QPushButton                  *m_pickTemperature;
    QPushButton                  *m_autoAdjustExposure;
    
    QComboBox                    *m_temperaturePresetCB;    
    QComboBox                    *m_channelCB;    
    
    QHButtonGroup                *m_scaleBG;  
        
    QLabel                       *m_adjTemperatureLabel;
    QLabel                       *m_temperaturePresetLabel;
    QLabel                       *m_darkLabel;
    QLabel                       *m_blackLabel;
    QLabel                       *m_mainExposureLabel;
    QLabel                       *m_fineExposureLabel;
    QLabel                       *m_gammaLabel;
    QLabel                       *m_saturationLabel;
    QLabel                       *m_greenLabel;
    
    KActiveLabel                 *m_exposureLabel;
    KActiveLabel                 *m_temperatureLabel;

    KDoubleNumInput              *m_temperatureInput;
    KDoubleNumInput              *m_darkInput;
    KDoubleNumInput              *m_blackInput;
    KDoubleNumInput              *m_mainExposureInput;
    KDoubleNumInput              *m_fineExposureInput;
    KDoubleNumInput              *m_gammaInput;
    KDoubleNumInput              *m_saturationInput;
    KDoubleNumInput              *m_greenInput;
    
    Digikam::HistogramWidget     *m_histogramWidget;
    
    Digikam::ColorGradientWidget *m_hGradient;
    
    Digikam::ImageWidget         *m_previewWidget;
};

}  // NameSpace DigikamWhiteBalanceImagesPlugin

#endif /* IMAGEEFFECT_WHITEBALANCE_H */
