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

// Local includes

#include "editortool.h"
#include "dimg.h"

class QWidget;
class QPushButton;
class QToolButton;
class QButtonGroup;

class KComboBox;

namespace Digikam
{
class CurvesWidget;
class ColorGradientWidget;
class ImageWidget;
class EditorToolSettings;
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

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotEffect();
    void slotResetSettings();
    void slotResetCurrentChannel();
    void slotCurveTypeChanged(int type);
    void slotSpotColorChanged(const Digikam::DColor& color);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);
    void slotPickerColorButtonActived();
    void slotChannelChanged();
    void slotScaleChanged();

private:

    enum ColorPicker
    {
        BlackTonal=0,
        GrayTonal,
        WhiteTonal
    };

    enum CurvesDrawingType
    {
        SmoothDrawing=0,
        FreeDrawing
    };

    uchar                        *m_destinationPreviewData;

    int                           m_histoSegments;
    int                           m_currentPreviewMode;

    QWidget                      *m_pickerBox;

    QPushButton                  *m_resetButton;
    QToolButton                  *m_pickBlack;
    QToolButton                  *m_pickGray;
    QToolButton                  *m_pickWhite;
    QToolButton                  *m_curveFree;
    QToolButton                  *m_curveSmooth;

    QButtonGroup                 *m_pickerColorButtonGroup;
    QButtonGroup                 *m_curveType;

    KComboBox                    *m_channelCB;

    Digikam::CurvesWidget        *m_curvesWidget;

    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::ColorGradientWidget *m_vGradient;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::DImg                *m_originalImage;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // namespace DigikamAdjustCurvesImagesPlugin

#endif /* ADJUSTCURVESTOOL_H */
