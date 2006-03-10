/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-12-06
 * Description : Black and White conversion tool.
 * 
 * Copyright 2004-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

#ifndef IMAGEEFFECT_BWSEPIA_H
#define IMAGEEFFECT_BWSEPIA_H

// Qt Includes.

#include <qstring.h>

// Digikam include.

#include "imagedlgbase.h"

class QHButtonGroup;
class QComboBox;
class QButtonGroup;

class KDoubleNumInput;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageWidget;
class DColor;
class DImg;
}

namespace DigikamImagesPluginCore
{

class ImageEffect_BWSepia : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_BWSepia(QWidget *parent);
    ~ImageEffect_BWSepia();

private:

    enum BlackWhiteConversionType
    {
        BWNeutral=0,
        BWGreenFilter,
        BWOrangeFilter,
        BWRedFilter,
        BWYellowFilter,
        BWSepia,
        BWBrown,
        BWCold,
        BWSelenium,
        BWPlatinum
    };

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
    
    QButtonGroup                 *m_bwTools;

    KDoubleNumInput              *m_cInput;
    
    Digikam::ImageWidget         *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;
    
    Digikam::HistogramWidget     *m_histogramWidget;    

private:

    void blackAndWhiteConversion(uchar *data, int w, int h, bool sb, int type);
    QString previewEffectPic(QString name);

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

#endif /* IMAGEEFFECT_BWSEPIA_H */
