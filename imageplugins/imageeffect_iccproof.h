/* ============================================================
 * Authors: F.J. Cruz <fj.cruz@supercable.es>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-12-21
 * Copyright 2005-2006 by F.J. Cruz
 *           2006 by Gilles Caulier
 *
 * Description : digiKam image editor tool to correct picture 
 *               colors using an ICC color profile
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

namespace Digikam
{
class ICCTransform;
class ImageWidget;
class HistogramWidget;
class ColorGradientWidget;
class DColor;
class ICCPreviewWidget;
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
    
    enum ICCSettingsTab
    {
        GENERALPAGE=0,
        INPUTPAGE,
        WORKSPACEPAGE,
        PROOFINGPAGE,
        DISPLAYPAGE
    };

    bool                            m_cmEnabled;
    bool                            m_hasICC;
    
    uchar                          *m_destinationPreviewData;

    QComboBox                      *m_channelCB;
    QComboBox                      *m_renderingIntentsCB;

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

    QString                         m_inPath;
    QString                         m_displayPath;
    QString                         m_spacePath;
    QString                         m_proofPath;

    QButtonGroup                   *m_inProfileBG;
    QButtonGroup                   *m_spaceProfileBG;
    QButtonGroup                   *m_proofProfileBG;
    QButtonGroup                   *m_displayProfileBG;
    
    QByteArray                      m_embeddedICC;

    QToolBox                       *m_toolBoxWidgets;

    KURLRequester                  *m_displayProfilePath;
    KURLRequester                  *m_inProfilesPath;
    KURLRequester                  *m_proofProfilePath;
    KURLRequester                  *m_spaceProfilePath;
        
    Digikam::ImageWidget           *m_previewWidget;

    Digikam::ColorGradientWidget   *m_hGradient;

    Digikam::HistogramWidget       *m_histogramWidget;

    Digikam::ICCPreviewWidget      *m_iccDisplayPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccInPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccOutPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccProofPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccSpacePreviewWidget;

private:

    void readSettings();
    void writeSettings();

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

    void slotUser2();
    void slotUser3();
    void slotDefault();
    void slotTry();
    void slotEffect();
    void slotChannelChanged(int);
    void slotScaleChanged(int);
    void slotColorSelectedFromTarget(const Digikam::DColor &);
    void slotToggledWidgets(bool);
    void slotInICCInfo();
    void slotProofICCInfo();
    void slotSpaceICCInfo();
    void slotDisplayICCInfo();
    void slotCMDisabledWarning();
    void processLCMSURL(const QString&);

protected:

    void finalRendering();
};

}  // NameSpace DigikamImagesPluginCore

#endif  // IMAGEEFFECT_ICCPROOF_H
