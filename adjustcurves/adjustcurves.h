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

// KDE includes.

#include <kdialogbase.h>

class QComboBox;
class QPushButton;
class QLabel;
class QHButtonGroup;
class QCheckBox;

namespace Digikam
{
class ImageCurves;
class ImageWidget;
class ImageGuideWidget;
class ColorGradientWidget;
class CurvesWidget;
}

namespace DigikamAdjustCurvesImagesPlugin
{

class AdjustCurveDialog : public KDialogBase
{
    Q_OBJECT

public:

    AdjustCurveDialog(QWidget *parent, uint *imageData, uint width, uint height);
    ~AdjustCurveDialog();

protected:

    void closeEvent(QCloseEvent *e);
    
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
    
    QComboBox                    *m_channelCB;    
    QComboBox                    *m_scaleCB;  
    QComboBox                    *m_typeCB;  
    
    QLabel                       *m_labelPos;
    
    QPushButton                  *m_loadButton;
    QPushButton                  *m_saveButton;
    QPushButton                  *m_helpButton;
    QPushButton                  *m_resetButton;
    QPushButton                  *m_pickBlack;
    QPushButton                  *m_pickGray;
    QPushButton                  *m_pickWhite;
    
    QCheckBox                    *m_overExposureIndicatorBox;

    QHButtonGroup                *m_pickerColorButtonGroup;
    
    Digikam::CurvesWidget        *m_curvesWidget;
    
    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::ColorGradientWidget *m_vGradient;
            
    Digikam::ImageGuideWidget    *m_previewOriginalWidget;
    Digikam::ImageWidget         *m_previewTargetWidget;
    
    Digikam::ImageCurves         *m_curves;

private:

    bool loadCurvesFromFile(KURL fileUrl);
    bool saveCurvesToFile(KURL fileUrl);
    
private slots:

    void slotUser1();
    void slotEffect();
    void slotOk();
    void slotHelp();
    void slotResetAllChannels();
    void slotLoadCurves();
    void slotSaveCurves();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotCurveTypeChanged(int type);
    void slotPositionChanged(int x, int y);
    void slotSpotColorChanged(const QColor &color, bool release);
};

}  // NameSpace DigikamAdjustCurvesImagesPlugin

#endif /* ADJUSTCURVES_H */
