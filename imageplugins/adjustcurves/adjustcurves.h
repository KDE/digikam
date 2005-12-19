/* ============================================================
 * File  : adjustcurves.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : image histogram adjust curves. 
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

#ifndef ADJUSTCURVES_H
#define ADJUSTCURVES_H

// Qt include.

#include <qcolor.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "imagetabdialog.h"

class QComboBox;
class QPushButton;
class QHButtonGroup;
class QCheckBox;

namespace Digikam
{
class CurvesWidget;
class ColorGradientWidget;
class ImageGuideWidget;
class ImageCurves;
class HistogramWidget;
class DImg;
}

namespace DigikamAdjustCurvesImagesPlugin
{

class AdjustCurveDialog : public DigikamImagePlugins::ImageTabDialog
{
    Q_OBJECT

public:

    AdjustCurveDialog(QWidget *parent, QString title, QFrame* banner);
    ~AdjustCurveDialog();

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
        
    QComboBox                    *m_channelCB;    
    
    QPushButton                  *m_resetButton;
    QPushButton                  *m_pickBlack;
    QPushButton                  *m_pickGray;
    QPushButton                  *m_pickWhite;
    QPushButton                  *m_curveFree;
    QPushButton                  *m_curveSmooth;
    
    QCheckBox                    *m_overExposureIndicatorBox;

    QHButtonGroup                *m_pickerColorButtonGroup;
    QHButtonGroup                *m_scaleBG;  
    QHButtonGroup                *m_curveType;
    
    Digikam::CurvesWidget        *m_curvesWidget;

    Digikam::HistogramWidget     *m_histogramWidget;
    
    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::ColorGradientWidget *m_vGradient;
        
    Digikam::ImageGuideWidget    *m_previewOriginalWidget;
    Digikam::ImageGuideWidget    *m_previewTargetWidget;

    Digikam::ImageCurves         *m_curves;
    Digikam::DImg                 m_originalImage;

private slots:

    void slotDefault();
    void slotUser2();
    void slotUser3();
    void slotEffect();
    void slotOk();
    void slotResetCurrentChannel();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotCurveTypeChanged(int type);
    void slotSpotColorChanged(const Digikam::DColor &color, bool release);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);    
};

}  // NameSpace DigikamAdjustCurvesImagesPlugin

#endif /* ADJUSTCURVES_H */
