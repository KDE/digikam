/* ============================================================
 * File  : adjustlevels.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-20
 * Description : image histogram adjust levels. 
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

#ifndef ADJUSTLEVELS_H
#define ADJUSTLEVELS_H

// KDE includes.

#include <kdialogbase.h>

class QComboBox;
class QSpinBox;
class QPushButton;
class QHButtonGroup;
class QCheckBox;

class KDoubleSpinBox;
class KGradientSelector;
class KDoubleNumInput;

namespace Digikam
{
class HistogramWidget;
class ImageLevels;
class ImageWidget;
class ImageGuideWidget;
}

namespace DigikamAdjustLevelsImagesPlugin
{

class AdjustLevelDialog : public KDialogBase
{
    Q_OBJECT

public:

    AdjustLevelDialog(QWidget *parent, uint *imageData, uint width, uint height);
    ~AdjustLevelDialog();

protected:

    void closeEvent(QCloseEvent *e);
    
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

    QComboBox                 *m_channelCB;    
    QComboBox                 *m_scaleCB;    
    
    QSpinBox                  *m_minInput;
    QSpinBox                  *m_maxInput;
    QSpinBox                  *m_minOutput;
    QSpinBox                  *m_maxOutput;
    
    KDoubleNumInput           *m_gammaInput;
    
    QPushButton               *m_loadButton;
    QPushButton               *m_saveButton;
    QPushButton               *m_autoButton;
    QPushButton               *m_resetButton;
    QPushButton               *m_helpButton;
    QPushButton               *m_pickBlack;
    QPushButton               *m_pickGray;
    QPushButton               *m_pickWhite;
    
    QCheckBox                    *m_overExposureIndicatorBox;
    
    QHButtonGroup             *m_pickerColorButtonGroup;
    
    KGradientSelector         *m_hGradientMinInput;
    KGradientSelector         *m_hGradientMaxInput;
    KGradientSelector         *m_hGradientMinOutput;
    KGradientSelector         *m_hGradientMaxOutput;
    
    Digikam::HistogramWidget  *m_histogramWidget;
    
    Digikam::ImageGuideWidget *m_previewOriginalWidget;
    Digikam::ImageWidget      *m_previewTargetWidget;
    
    Digikam::ImageLevels      *m_levels;

private:

    void adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut);
    bool loadLevelsFromFile(KURL fileUrl);
    bool saveLevelsToFile(KURL fileUrl);
    
private slots:

    void slotUser1();
    void slotEffect();
    void slotOk();
    void slotHelp();
    void slotResetAllChannels();
    void slotAutoLevels();
    void slotLoadLevels();
    void slotSaveLevels();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotAdjustSliders();
    void slotGammaInputchanged(double val);
    void slotAdjustMinInputSpinBox(int val);
    void slotAdjustMaxInputSpinBox(int val);
    void slotAdjustMinOutputSpinBox(int val);
    void slotAdjustMaxOutputSpinBox(int val);
    void slotSpotColorChanged(const QColor &color, bool release);
};

}  // NameSpace DigikamAdjustLevelsImagesPlugin

#endif /* ADJUSTLEVELS_H */
