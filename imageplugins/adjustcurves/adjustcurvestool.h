/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
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

#ifndef ADJUSTCURVESTOOL_H
#define ADJUSTCURVESTOOL_H

// Digikam includes.

#include "editortool.h"

// Local includes.

class QComboBox;
class QPushButton;
class QHButtonGroup;

namespace Digikam
{
class CurvesWidget;
class HistogramWidget;
class ColorGradientWidget;
class EditorToolSettings;
class ImageWidget;
class DImg;
class DColor;
}

namespace DigikamAdjustCurvesImagesPlugin
{

class AdjustCurvesTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    AdjustCurvesTool(QObject *parent);
    ~AdjustCurvesTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();

private slots:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotEffect();
    void slotResetSettings();
    void slotResetCurrentChannel();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotCurveTypeChanged(int type);
    void slotSpotColorChanged(const Digikam::DColor& color);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);
    void slotPickerColorButtonActived();

private:

    enum ColorPicker
    {
        BlackTonal=0,
        GrayTonal,
        WhiteTonal
    };

    enum ColorChannel
    {
        LuminosityChannel=0,
        RedChannel,
        GreenChannel,
        BlueChannel,
        AlphaChannel
    };

    enum CurvesDrawingType
    {
        SmoothDrawing=0,
        FreeDrawing
    };

    enum HistogramScale
    {
        Linear=0,
        Logarithmic
    };

    uchar                        *m_destinationPreviewData;

    int                           m_histoSegments;
    int                           m_currentPreviewMode;

    QComboBox                    *m_channelCB;

    QPushButton                  *m_resetButton;
    QPushButton                  *m_pickBlack;
    QPushButton                  *m_pickGray;
    QPushButton                  *m_pickWhite;
    QPushButton                  *m_curveFree;
    QPushButton                  *m_curveSmooth;

    QHButtonGroup                *m_pickerColorButtonGroup;
    QHButtonGroup                *m_scaleBG;  
    QHButtonGroup                *m_curveType;

    Digikam::CurvesWidget        *m_curvesWidget;

    Digikam::HistogramWidget     *m_histogramWidget;

    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::ColorGradientWidget *m_vGradient;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;

    Digikam::DImg                *m_originalImage;
};

}  // NameSpace DigikamAdjustCurvesImagesPlugin

#endif /* ADJUSTCURVESTOOL_H */
