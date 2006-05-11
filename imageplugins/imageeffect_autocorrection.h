/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-31
 * Description : Auto-Color correction tool.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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

#ifndef IMAGEEFFECT_AUTOCORRECTION_H
#define IMAGEEFFECT_AUTOCORRECTION_H

// Qt Includes.

#include <qstring.h>

// Digikam include.

#include "imagedlgbase.h"

class QHButtonGroup;
class QComboBox;
class QListBox;
class QButtonGroup;

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

class ImageEffect_AutoCorrection : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_AutoCorrection(QWidget *parent);
    ~ImageEffect_AutoCorrection();

private:

    enum AutoCorrectionType
    {
        AutoLevelsCorrection=0,
        NormalizeCorrection,
        EqualizeCorrection,
        StretchContrastCorrection
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

    QListBox                     *m_correctionTools;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::ColorGradientWidget *m_hGradient;
    
    Digikam::HistogramWidget     *m_histogramWidget;    

    Digikam::DImg                 m_thumbnailImage;
        
private:

    void autoCorrection(uchar *data, int w, int h, bool sb, int type);
    QString previewEffectPic(QString name);
    QPixmap getThumbnailForEffect(AutoCorrectionType type);
    
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

#endif /* IMAGEEFFECT_AUTOCORRECTION_H */
