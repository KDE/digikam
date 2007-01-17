/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2004-12-01
 * Description : image histogram adjust curves. 
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

#ifndef ADJUSTCURVES_H
#define ADJUSTCURVES_H

// Qt include.

#include <qcolor.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

class QComboBox;
class QPushButton;
class QHButtonGroup;

namespace DigikamAdjustCurvesImagesPlugin
{

class AdjustCurveDialog : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    AdjustCurveDialog(QWidget *parent, QString title, QFrame* banner);
    ~AdjustCurveDialog();

private:

    void readUserSettings();
    void writeUserSettings();
    void resetValues();
    void finalRendering();
    
private slots:

    void slotUser2();
    void slotUser3();
    void slotEffect();
    void slotResetCurrentChannel();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotCurveTypeChanged(int type);
    void slotSpotColorChanged(const Digikam::DColor &color);
    void slotColorSelectedFromTarget(const Digikam::DColor &color);
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

    Digikam::ImageCurves         *m_curves;
    Digikam::DImg                 m_originalImage;
};

}  // NameSpace DigikamAdjustCurvesImagesPlugin

#endif /* ADJUSTCURVES_H */
