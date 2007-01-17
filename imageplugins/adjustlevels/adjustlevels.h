/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-07-20
 * Description : image histogram adjust levels. 
 * 
 * Copyright 2004-2007 by Gilles Caulier
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

#ifndef ADJUSTLEVELS_H
#define ADJUSTLEVELS_H

// Digikam includes.

#include <digikamheaders.h>

class QComboBox;
class QSpinBox;
class QPushButton;
class QHButtonGroup;

class KDoubleSpinBox;
class KGradientSelector;
class KDoubleNumInput;

namespace DigikamAdjustLevelsImagesPlugin
{

class AdjustLevelDialog : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    AdjustLevelDialog(QWidget *parent, QString title, QFrame* banner);
    ~AdjustLevelDialog();

private:

    void readUserSettings();
    void writeUserSettings();
    void resetValues();
    void finalRendering(); 
    void adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut);
    
private slots:

    void slotUser2();
    void slotUser3();
    void slotEffect();
    void slotResetCurrentChannel();
    void slotAutoLevels();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotAdjustSliders();
    void slotGammaInputchanged(double val);
    void slotAdjustMinInputSpinBox(int val);
    void slotAdjustMaxInputSpinBox(int val);
    void slotAdjustMinOutputSpinBox(int val);
    void slotAdjustMaxOutputSpinBox(int val);
    void slotSpotColorChanged(const Digikam::DColor &color);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);
    void slotPickerColorButtonActived();    

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
        BlueChannel,
        AlphaChannel
    };
    
    enum ColorPicker
    {
        BlackTonal=0,
        GrayTonal,
        WhiteTonal
    };

    uchar                     *m_destinationPreviewData;
    
    int                        m_histoSegments;
    int                        m_currentPreviewMode;
    
    QComboBox                 *m_channelCB;    
    
    QSpinBox                  *m_minInput;
    QSpinBox                  *m_maxInput;
    QSpinBox                  *m_minOutput;
    QSpinBox                  *m_maxOutput;
    
    KDoubleNumInput           *m_gammaInput;
    
    QPushButton               *m_autoButton;
    QPushButton               *m_resetButton;
    QPushButton               *m_pickBlack;
    QPushButton               *m_pickGray;
    QPushButton               *m_pickWhite;
    
    QHButtonGroup             *m_pickerColorButtonGroup;
    QHButtonGroup             *m_scaleBG;
    
    KGradientSelector         *m_hGradientMinInput;
    KGradientSelector         *m_hGradientMaxInput;
    KGradientSelector         *m_hGradientMinOutput;
    KGradientSelector         *m_hGradientMaxOutput;
    
    Digikam::HistogramWidget  *m_levelsHistogramWidget;
    Digikam::HistogramWidget  *m_histogramWidget;
    
    Digikam::ImageWidget      *m_previewWidget;
    
    Digikam::ImageLevels      *m_levels;
    Digikam::DImg              m_originalImage;
};

}  // NameSpace DigikamAdjustLevelsImagesPlugin

#endif /* ADJUSTLEVELS_H */
