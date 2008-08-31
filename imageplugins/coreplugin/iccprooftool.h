/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-21
 * Description : digiKam image editor tool to correct picture
 *               colors using an ICC color profile
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ICCPROOFTOOL_H
#define ICCPROOFTOOL_H

// Digikam includes.

#include "editortool.h"

class QCheckBox;
class QComboBox;
class QVButtonGroup;
class QButtonGroup;
class QHButtonGroup;
class QRadioButton;
class QPushButton;
class QToolBox;

class KURLRequester;

namespace KDcrawIface
{
class RIntNumInput;
class RComboBox;
}

namespace Digikam
{
class ICCTransform;
class ImageWidget;
class HistogramWidget;
class ColorGradientWidget;
class DColor;
class ICCPreviewWidget;
class CurvesWidget;
}

namespace DigikamImagesPluginCore
{

class ICCProofTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    ICCProofTool(QObject* parent);
    ~ICCProofTool();

protected:

    void finalRendering();

private:

    void readSettings();
    void writeSettings();

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

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotEffect();
    void slotResetSettings();
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

    KURLRequester                  *m_inProfilesPath;
    KURLRequester                  *m_spaceProfilePath;
    KURLRequester                  *m_proofProfilePath;

    KDcrawIface::RIntNumInput      *m_cInput;

    KDcrawIface::RComboBox         *m_renderingIntentsCB;

    Digikam::DImg                  *m_originalImage;

    Digikam::CurvesWidget          *m_curvesWidget;

    Digikam::ImageWidget           *m_previewWidget;

    Digikam::ColorGradientWidget   *m_hGradient;

    Digikam::HistogramWidget       *m_histogramWidget;

    Digikam::ICCPreviewWidget      *m_iccInPreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccSpacePreviewWidget;
    Digikam::ICCPreviewWidget      *m_iccProofPreviewWidget;

    Digikam::EditorToolSettings    *m_gboxSettings;
};

}  // NameSpace DigikamImagesPluginCore

#endif  // ICCPROOFTOOL_H
