/* ============================================================
 * Author: F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-12-21
 * Copyright 2005 by F.J. Cruz
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
 * ============================================================ */

#ifndef IMAGEEFFECT_ICCPROOF_H
#define IMAGEEFFECT_ICCPROOF_H

// Digikam include.

#include "imagedlgbase.h"

class QCheckBox;
class QComboBox;
class QVButtonGroup;
class QButtonGroup;
class QHButtonGroup;
class QRadioButton;

class KTabWidget;

namespace Digikam
{
class ICCTransform;
class ImageGuideWidget;
class HistogramWidget;
class ColorGradientWidget;
class DColor;
}

namespace DigikamImagesPluginCore
{

class ImageEffect_ICCProof : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:
    
    ImageEffect_ICCProof(QWidget* parent);

    ~ImageEffect_ICCProof();

private:

    enum HistogramScale
    {
        Linear = 0,
        Logarithmic
    };

    enum ColorChannel
    {
        LuminosityChannel = 0,
        RedChannel,
        GreenChannel,
        BlueChannel
    };

    uchar                           *m_destinationPreviewData;

    KTabWidget                      *m_tabsWidgets;

    QComboBox                       *m_channelCB;
    QComboBox                       *m_displayProfilesCB;
    QComboBox                       *m_inProfilesCB;
    QComboBox                       *m_outProfilesCB;
    QComboBox                       *m_renderingIntentCB;

    QCheckBox                       *m_softProofBox;
    QCheckBox                       *m_overExposureIndicatorBox;
    QCheckBox                       *m_doSofProfBox;
    QCheckBox                       *m_checkGamutBox;
    QCheckBox                       *m_BPCBox;
    QCheckBox                       *m_embeddProfileBox;

    QRadioButton                    *m_useEmbeddedProfile;
    QRadioButton                    *m_useInDefaultProfile;
    QRadioButton                    *m_useInSelectedProfile;
    QRadioButton                    *m_useOutDefaultProfile;
    QRadioButton                    *m_useOutSelectedProfile;
    QRadioButton                    *m_useDisplayDefaultProfile;
    QRadioButton                    *m_useDisplaySelectedProfile;

    QHButtonGroup                   *m_scaleBG;

    QVButtonGroup                   *m_renderingIntentBG;
    QButtonGroup                    *m_optionsBG;
    QVButtonGroup                   *m_profilesBG;

    Digikam::ImageGuideWidget       *m_previewWidget;

    Digikam::ColorGradientWidget    *m_hGradient;

    Digikam::HistogramWidget        *m_histogramWidget;

private slots:

    void slotDefault();
    void slotEffect();
    void slotOk();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorSelectedFromTarget( const Digikam::DColor &color );

};

}  // NameSpace DigikamImagesPluginCore

#endif  // IMAGEEFFECT_ICCPROOF_H
