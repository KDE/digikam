/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-11
 * Description : a digiKam image editor plugin to correct
 *               image white balance
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Guillaume Castagnino <casta at xwing dot info>
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

#ifndef WHITEBALANCETOOL_H
#define WHITEBALANCETOOL_H

// Qt includes.

#include <qcolor.h>

// Digikam includes.

#include "editortool.h"

class QComboBox;
class QPushButton;
class QLabel;
class QPushButton;
class QHButtonGroup;

class KActiveLabel;

namespace KDcrawIface
{
class RDoubleNumInput;
class RComboBox;
}

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageWidget;
class DColor;
class EditorToolSettings;
}

namespace DigikamWhiteBalanceImagesPlugin
{

class WhiteBalanceTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    WhiteBalanceTool(QObject* parent);
    ~WhiteBalanceTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();

private slots:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotEffect();
    void slotResetSettings();
    void slotColorSelectedFromOriginal(const Digikam::DColor &color);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);
    void slotScaleChanged(int scale);
    void slotChannelChanged(int channel);
    void slotTemperatureChanged(double temperature);
    void slotTemperaturePresetChanged(int tempPreset);
    void slotAutoAdjustExposure(void);
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

    QComboBox                    *m_channelCB;

    QPushButton                  *m_pickTemperature;
    QPushButton                  *m_autoAdjustExposure;

    KDcrawIface::RComboBox       *m_temperaturePresetCB;

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

    KDcrawIface::RDoubleNumInput *m_temperatureInput;
    KDcrawIface::RDoubleNumInput *m_darkInput;
    KDcrawIface::RDoubleNumInput *m_blackInput;
    KDcrawIface::RDoubleNumInput *m_mainExposureInput;
    KDcrawIface::RDoubleNumInput *m_fineExposureInput;
    KDcrawIface::RDoubleNumInput *m_gammaInput;
    KDcrawIface::RDoubleNumInput *m_saturationInput;
    KDcrawIface::RDoubleNumInput *m_greenInput;

    Digikam::HistogramWidget     *m_histogramWidget;

    Digikam::ColorGradientWidget *m_hGradient;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // NameSpace DigikamWhiteBalanceImagesPlugin

#endif /* WHITEBALANCETOOL_H */
