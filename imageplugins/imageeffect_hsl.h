/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-07-16
 * Description : digiKam image editor Hue/Saturation/Lightness 
 *               correction tool
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

#ifndef IMAGEEFFECT_HSL_H
#define IMAGEEFFECT_HSL_H

// Digikam include.

#include "imagedlgbase.h"

class QCheckBox;
class QComboBox;
class QHButtonGroup;

class KDoubleNumInput;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageWidget;
class DColor;
}

namespace DigikamImagesPluginCore
{

class ImageEffect_HSL : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_HSL(QWidget *parent);
    ~ImageEffect_HSL();

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

    uchar                        *m_destinationPreviewData;

    QComboBox                    *m_channelCB;    
    
    QHButtonGroup                *m_scaleBG;  
    
    QCheckBox                    *m_overExposureIndicatorBox;

    KDoubleNumInput              *m_hInput;
    KDoubleNumInput              *m_sInput;
    KDoubleNumInput              *m_lInput;
    
    Digikam::ImageWidget         *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;
    
    Digikam::HistogramWidget     *m_histogramWidget;
    
private slots:

    void slotDefault();
    void slotEffect();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorSelectedFromTarget( const Digikam::DColor &color );

protected:

    void finalRendering();
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_HSL_H */
