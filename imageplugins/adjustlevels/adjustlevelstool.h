/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : image histogram adjust levels.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ADJUSTLEVELSTOOL_H
#define ADJUSTLEVELSTOOL_H

// Digikam includes.

#include "editortool.h"

class QComboBox;
class QPushButton;
class QHButtonGroup;

class KGradientSelector;

namespace KDcrawIface
{
class RDoubleNumInput;
class RIntNumInput;
}

namespace Digikam
{
class HistogramWidget;
class ImageWidget;
class ImageLevels;
class DImg;
class DColor;
}

namespace DigikamAdjustLevelsImagesPlugin
{

class AdjustLevelsTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    AdjustLevelsTool(QObject *parent);
    ~AdjustLevelsTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void adjustSliders(int minIn, double gamIn, int maxIn, int minOut, int maxOut);
    bool eventFilter(QObject *o, QEvent *e);

private slots:

    void slotLoadSettings();
    void slotSaveAsSettings();
    void slotEffect();
    void slotResetSettings();
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
    void slotSpotColorChanged(const Digikam::DColor& color);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);
    void slotPickerColorButtonActived();
    void slotShowInputHistogramGuide(int v);
    void slotShowOutputHistogramGuide(int v);

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

    uchar                        *m_destinationPreviewData;

    int                           m_histoSegments;
    int                           m_currentPreviewMode;

    QComboBox                    *m_channelCB;

    QPushButton                  *m_autoButton;
    QPushButton                  *m_resetButton;
    QPushButton                  *m_pickBlack;
    QPushButton                  *m_pickGray;
    QPushButton                  *m_pickWhite;

    QHButtonGroup                *m_pickerColorButtonGroup;
    QHButtonGroup                *m_scaleBG;

    KGradientSelector            *m_hGradientMinInput;
    KGradientSelector            *m_hGradientMaxInput;
    KGradientSelector            *m_hGradientMinOutput;
    KGradientSelector            *m_hGradientMaxOutput;

    KDcrawIface::RDoubleNumInput *m_gammaInput;

    KDcrawIface::RIntNumInput    *m_minInput;
    KDcrawIface::RIntNumInput    *m_maxInput;
    KDcrawIface::RIntNumInput    *m_minOutput;
    KDcrawIface::RIntNumInput    *m_maxOutput;

    Digikam::HistogramWidget     *m_levelsHistogramWidget;
    Digikam::HistogramWidget     *m_histogramWidget;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;

    Digikam::ImageLevels         *m_levels;
    Digikam::DImg                *m_originalImage;
};

}  // NameSpace DigikamAdjustLevelsImagesPlugin

#endif /* ADJUSTLEVELSTOOL_H */
