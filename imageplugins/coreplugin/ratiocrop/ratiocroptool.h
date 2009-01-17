/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : digiKam image editor Ratio Crop tool
 *
 * Copyright (C) 2007 by Jaromir Malenko <malenko at email dot cz>
 * Copyright (C) 2008 by Roberto Castagnola <roberto dot castagnola at gmail dot com>
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RATIOCROPTOOL_H
#define RATIOCROPTOOL_H

// Digikam includes.

#include "editortool.h"

class QCheckBox;
class QLabel;
class QToolButton;

class KColorButton;

namespace KDcrawIface
{
class RComboBox;
class RIntNumInput;
}

namespace DigikamImagesPluginCore
{

class ImageSelectionWidget;

class RatioCropTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    RatioCropTool(QObject *parent);
    ~RatioCropTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();

    void applyRatioChanges(int a);
    void setRatioCBText(int orientation);

private slots:

    void slotMaxAspectRatio();
    void slotResetSettings();

    void slotCenterWidth();
    void slotCenterHeight();
    void slotXChanged(int x);
    void slotYChanged(int y);
    void slotWidthChanged(int w);
    void slotHeightChanged(int h);
    void slotCustomRatioChanged();
    void slotCustomNRatioChanged(int a);
    void slotCustomDRatioChanged(int a);
    void slotPreciseCropChanged(bool a);
    void slotOrientChanged(int o);
    void slotAutoOrientChanged(bool a);
    void slotRatioChanged(int a);
    void slotSelectionChanged(QRect rect );
    void slotSelectionOrientationChanged(int);
    void slotGuideTypeChanged(int t);
    void slotGoldenGuideTypeChanged();

private:

    bool                         m_originalIsLandscape;

    QLabel                      *m_customLabel1;
    QLabel                      *m_customLabel2;
    QLabel                      *m_orientLabel;
    QLabel                      *m_colorGuideLabel;


    QToolButton                 *m_centerWidth;
    QToolButton                 *m_centerHeight;

    QCheckBox                   *m_goldenSectionBox;
    QCheckBox                   *m_goldenSpiralSectionBox;
    QCheckBox                   *m_goldenSpiralBox;
    QCheckBox                   *m_goldenTriangleBox;
    QCheckBox                   *m_flipHorBox;
    QCheckBox                   *m_flipVerBox;
    QCheckBox                   *m_autoOrientation;
    QCheckBox                   *m_preciseCrop;

    KDcrawIface::RComboBox      *m_guideLinesCB;
    KDcrawIface::RComboBox      *m_orientCB;
    KDcrawIface::RComboBox      *m_ratioCB;

    KDcrawIface::RIntNumInput   *m_customRatioDInput;
    KDcrawIface::RIntNumInput   *m_customRatioNInput;
    KDcrawIface::RIntNumInput   *m_guideSize;
    KDcrawIface::RIntNumInput   *m_heightInput;
    KDcrawIface::RIntNumInput   *m_widthInput;
    KDcrawIface::RIntNumInput   *m_xInput;
    KDcrawIface::RIntNumInput   *m_yInput;

    KColorButton                *m_guideColorBt;

    ImageSelectionWidget        *m_imageSelectionWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* RATIOCROPTOOL_H */
