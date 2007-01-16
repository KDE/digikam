/* ============================================================
 * Authors: F.J. Cruz <fj.cruz@supercable.es>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-12-21
 *
 * Description : digiKam image editor tool to correct picture 
 *               colors using an ICC color profile
 *
 * Copyright 2005-2006 by F.J. Cruz
 * Copyright 2006-2007 by Gilles Caulier
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
class QToolBox;

class KURLRequester;
class KIntNumInput;

namespace Digikam
{
class ICCTransform;
class ImageWidget;
class HistogramWidget;
class ColorGradientWidget;
class DColor;
class ICCPreviewWidget;
class ImageCurves;
class CurvesWidget;
}

namespace DigikamImagesPluginCore
{

class ImageEffect_ICCProof : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:
    
    ImageEffect_ICCProof(QWidget* parent);
    ~ImageEffect_ICCProof();

protected:

    void finalRendering();

private:

    void readUserSettings();
    void writeUserSettings();
    void resetValues();

    void getICCInfo(const QString&);
    void getICCInfo(const QByteArray&);

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

    bool useDefaultProofProfile();
    bool useSelectedProofProfile();

private slots:

    void slotUser2();
    void slotUser3();
    void slotEffect();
    void slotChannelChanged(int);
    void slotScaleChanged(int);
    void slotSpotColorChanged(const Digikam::DColor &);    
    void slotColorSelectedFromTarget(const Digikam::DColor &);
    void slotToggledWidgets(bool);
    void slotInICCInfo();
    void slotProofICCInfo();
    void slotSpaceICCInfo();
    void slotCMDisabledWarning();
    void processLCMSURL(const QString&);

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
    
    enum ICCSettingsTab
    {
        GENERALPAGE=0,
        INPUTPAGE,
        WORKSPACEPAGE,
        PROOFINGPAGE,
        LIGHTNESSPAGE
    };

    bool                            m_cmEnabled;
    bool                            m_hasICC;
    
    uchar                          *m_destinationPreviewData;

    QComboBox                      *m_channelCB;
    QComboBox                      *m_renderingIntentsCB;

    QCheckBox                      *m_doSoftProofBox;
    QCheckBox                      *m_checkGamutBox;
    QCheckBox                      *m_embeddProfileBox;
    QCheckBox                      *m_BPCBox;

    QRadioButton                   *m_useEmbeddedProfile;
    QRadioButton                   *m_useInDefaultProfile;
    QRadioButton                   *m_useInSelectedProfile;
    QRadioButton                   *m_useProofDefaultProfile;
    QRadioButton                   *m_useProofSelectedProfile;
    QRadioButton                   *m_useSpaceDefaultProfile;
    QRadioButton                   *m_useSpaceSelectedProfile;
    QRadioButton                   *m_useSRGBDefaultProfile;

    QString                         m_inPath;
    QString                         m_spacePath;
    QString                         m_proofPath;

    QButtonGroup                   *m_optionsBG;
    QButtonGroup                   *m_inProfileBG;
    QButtonGroup                   *m_spaceProfileBG;
    QButtonGroup                   *m_proofProfileBG;

    QHButtonGroup                  *m_scaleBG;
    QVButtonGroup                  *m_renderingIntentBG;
    QVButtonGroup                  *m_profilesBG;
    
    QByteArray                      m_embeddedICC;

    QToolBox                       *m_toolBoxWidgets;

    KIntNumInput                   *m_cInput;

    KURLRequester                  *m_inProfilesPath;
    KURLRequester                  *m_spaceProfilePath;
    KURLRequester                  *m_proofProfilePath;
        
    Digikam::DImg                  *m_originalImage;

    Digikam::CurvesWidget          *m_curvesWidget;

    Digikam::ImageCurves           *m_curves;

    Digikam::ImageWidget           *m_previewWidget;

    Digikam::ColorGradientWidget   *m_hGradient;

    Digikam::HistogramWidget       *m_histogramWidget;

    Digikam::ICCPreviewWidget      *m_iccInPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccSpacePreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccProofPreviewWidget;
};

}  // NameSpace DigikamImagesPluginCore

#endif  // IMAGEEFFECT_ICCPROOF_H
