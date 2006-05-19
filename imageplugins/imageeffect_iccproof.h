/* ============================================================
 * Author: F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-12-21
 * Copyright 2005-2006 by F.J. Cruz
 * Description : digiKam image editor to correct an image using
 *               an ICC color profile
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
class QPushButton;
class QCString;

class KTabWidget;
class KURLRequester;

namespace Digikam
{
class ICCTransform;
class ImageWidget;
class HistogramWidget;
class ColorGradientWidget;
class DColor;
class ICCPreviewWidget;
class ImageIface;
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

    uchar                          *m_destinationPreviewData;

    KTabWidget                     *m_tabsWidgets;

    QComboBox                      *m_channelCB;
    QComboBox                      *m_renderingIntentsCB;

    KURLRequester                  *m_displayProfileCB;
    KURLRequester                  *m_inProfilesCB;
    KURLRequester                  *m_proofProfileCB;
    KURLRequester                  *m_spaceProfileCB;
    
    QCheckBox                      *m_doSoftProofBox;
    QCheckBox                      *m_overExposureIndicatorBox;
    QCheckBox                      *m_doSofProfBox;
    QCheckBox                      *m_checkGamutBox;
    QCheckBox                      *m_BPCBox;
    QCheckBox                      *m_embeddProfileBox;

    QRadioButton                   *m_useEmbeddedProfile;
    QRadioButton                   *m_useInDefaultProfile;
    QRadioButton                   *m_useInSelectedProfile;
    QRadioButton                   *m_useProofDefaultProfile;
    QRadioButton                   *m_useProofSelectedProfile;
    QRadioButton                   *m_useDisplayDefaultProfile;
    QRadioButton                   *m_useDisplaySelectedProfile;
    QRadioButton                   *m_useSpaceDefaultProfile;
    QRadioButton                   *m_useSpaceSelectedProfile;
    QRadioButton                   *m_useSRGBDefaultProfile;

    QHButtonGroup                  *m_scaleBG;

    QVButtonGroup                  *m_renderingIntentBG;
    QButtonGroup                   *m_optionsBG;
    QVButtonGroup                  *m_profilesBG;

    QString                         inPath;
    QString                         displayPath;
    QString                         spacePath;
    QString                         proofPath;

    bool                            cmEnabled;
    bool                            hasICC;

    QByteArray                      m_embeddedICC;

    Digikam::ImageWidget           *m_previewWidget;

    Digikam::ColorGradientWidget   *m_hGradient;

    Digikam::HistogramWidget       *m_histogramWidget;

    Digikam::ImageIface            *iface;

    Digikam::ICCPreviewWidget      *m_iccDisplayPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccInPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccOutPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccProofPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccSpacePreviewWidget;

private:

    void readSettings();
    void getICCInfo(const QString&);
    void getICCInfo(QByteArray&);

    bool useBPC();
    bool doProof();
    bool checkGamut();
    bool embedProfile();

    bool useEmbeddedProfile();
    bool useBuiltinProfile();
    bool useDefaultInProfile();
    bool useSelectedInProfile();

    bool useDefaultSpaceProfile();
    bool useSelectedSpaceProfile();

    bool useDefaultDisplayProfile();
    bool useSelectedDisplayProfile();

    bool useDefaultProofProfile();
    bool useSelectedProofProfile();

private slots:

    void slotDefault();
    void slotTry();
    void slotEffect();
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorSelectedFromTarget( const Digikam::DColor &color );
    void slotToggledWidgets(bool t);
    void slotInICCInfo();
    void slotProofICCInfo();
    void slotSpaceICCInfo();
    void slotDisplayICCInfo();
    void slotCMDisabledWarning();

protected:

    void finalRendering();
};

}  // NameSpace DigikamImagesPluginCore

#endif  // IMAGEEFFECT_ICCPROOF_H
