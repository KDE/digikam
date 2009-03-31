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

#include <QColor>

// Local includes.

#include "editortool.h"

class QPushButton;
class QLabel;
class QPushButton;
class QToolButton;

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


private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();
    void slotEffect();
    void slotColorSelectedFromOriginal(const Digikam::DColor &color);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);
    void slotTemperatureChanged(double temperature);
    void slotTemperaturePresetChanged(int tempPreset);
    void slotAutoAdjustExposure(void);
    void slotPickerColorButtonActived();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void blockWidgetSignals(bool b);

private:

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

    QToolButton                  *m_pickTemperature;
    QToolButton                  *m_autoAdjustExposure;

    QLabel                       *m_adjTemperatureLabel;
    QLabel                       *m_temperaturePresetLabel;
    QLabel                       *m_darkLabel;
    QLabel                       *m_blackLabel;
    QLabel                       *m_mainExposureLabel;
    QLabel                       *m_fineExposureLabel;
    QLabel                       *m_gammaLabel;
    QLabel                       *m_saturationLabel;
    QLabel                       *m_greenLabel;
    QLabel                       *m_exposureLabel;
    QLabel                       *m_temperatureLabel;

    KDcrawIface::RComboBox       *m_temperaturePresetCB;

    KDcrawIface::RDoubleNumInput *m_temperatureInput;
    KDcrawIface::RDoubleNumInput *m_darkInput;
    KDcrawIface::RDoubleNumInput *m_blackInput;
    KDcrawIface::RDoubleNumInput *m_mainExposureInput;
    KDcrawIface::RDoubleNumInput *m_fineExposureInput;
    KDcrawIface::RDoubleNumInput *m_gammaInput;
    KDcrawIface::RDoubleNumInput *m_saturationInput;
    KDcrawIface::RDoubleNumInput *m_greenInput;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // namespace DigikamWhiteBalanceImagesPlugin

#endif /* WHITEBALANCETOOL_H */
