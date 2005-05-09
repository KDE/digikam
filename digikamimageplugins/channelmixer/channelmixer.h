/* ============================================================
 * File  : channelmixer.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-26
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

#ifndef CHANNELMIXER_H
#define CHANNELMIXER_H

// KDE includes.

#include <kdialogbase.h>

class QComboBox;
class QPushButton;
class QCheckBox;

class KDoubleNumInput;

namespace Digikam
{
class ImageGuideWidget;
class HistogramWidget;
class ImageWidget;
class ColorGradientWidget;
}

namespace DigikamChannelMixerImagesPlugin
{

class ChannelMixerDialog : public KDialogBase
{
    Q_OBJECT

public:

    ChannelMixerDialog(QWidget *parent, uint *imageData, uint width, uint height);
    ~ChannelMixerDialog();

private:

    enum ColorChannelGains 
    {
    RedChannelGains=0,
    GreenChannelGains,
    BlueChannelGains
    };

    enum HistogramScale
    {
    Linear=0,
    Logarithmic
    };

protected:

    void closeEvent(QCloseEvent *e);
    
private:
    
    double                        m_redRedGain;
    double                        m_redGreenGain;
    double                        m_redBlueGain;
    double                        m_greenRedGain;
    double                        m_greenGreenGain;
    double                        m_greenBlueGain;
    double                        m_blueRedGain;
    double                        m_blueGreenGain;
    double                        m_blueBlueGain;
    double                        m_blackRedGain;
    double                        m_blackGreenGain;
    double                        m_blackBlueGain;
    
    QComboBox                    *m_channelCB;    
    QComboBox                    *m_scaleCB;    
    
    KDoubleNumInput              *m_redGain;
    KDoubleNumInput              *m_greenGain;
    KDoubleNumInput              *m_blueGain;
    
    QPushButton                  *m_loadButton;
    QPushButton                  *m_saveButton;
    QPushButton                  *m_resetButton;
    QPushButton                  *m_helpButton;
    
    QCheckBox                    *m_preserveLuminosity;
    QCheckBox                    *m_monochrome;
    QCheckBox                    *m_overExposureIndicatorBox;
    
    Digikam::ColorGradientWidget *m_hGradient;
    
    Digikam::HistogramWidget     *m_histogramWidget;
    
    Digikam::ImageWidget         *m_previewOriginalWidget;
    Digikam::ImageGuideWidget    *m_previewTargetWidget;
    
    uint                         *m_destinationPreviewData;

    void adjustSliders(void);
    
private slots:

    void slotUser1();
    void slotEffect();
    void slotOk();
    void slotHelp();
    void slotResetAllGains();
    void slotLoadGains();
    void slotSaveGains();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotGainsChanged();
    void slotMonochromeActived(bool mono);
    void slotColorSelectedFromTarget(const QColor &color);    
};

}  // NameSpace DigikamChannelMixerImagesPlugin

#endif /* CHANNELMIXER_H */
