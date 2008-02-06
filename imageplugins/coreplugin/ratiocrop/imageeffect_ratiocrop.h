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
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEEFFECT_RATIOCROP_H
#define IMAGEEFFECT_RATIOCROP_H

// Digikam include.

#include "imagedlgbase.h"

class QLabel;
class QComboBox;
class QPushButton;
class QCheckBox;
class QSpinBox;

class KIntNumInput;
class KIntSpinBox;
class KColorButton;

namespace DigikamImagesPluginCore
{

class ImageSelectionWidget;

class ImageEffect_RatioCrop : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_RatioCrop(QWidget *parent);
    ~ImageEffect_RatioCrop();

private:

    void readSettings();
    void writeSettings();

    void finalRendering();
    void applyRatioChanges(int a);
    void setRatioCBText(int orientation);

private slots:

    void slotUser1();
    void slotDefault();

    void slotCenterWidth();
    void slotCenterHeight();
    void slotXChanged(int x);
    void slotYChanged(int y);
    void slotWidthChanged(int w);
    void slotHeightChanged(int h);
    void slotCustomRatioChanged();
    void slotCustomNRatioChanged(int a);
    void slotCustomDRatioChanged(int a);
    void slotOrientChanged(int o);
    void slotAutoOrientChanged(bool a);
    void slotRatioChanged(int a);
    void slotSelectionChanged(QRect rect );
    void slotSelectionOrientationChanged(int);
    void slotGuideTypeChanged(int t);
    void slotGoldenGuideTypeChanged();

private:

    bool                  m_originalIsLandscape;

    QLabel               *m_customLabel1;
    QLabel               *m_customLabel2;
    QLabel               *m_orientLabel;
    QLabel               *m_colorGuideLabel;

    QComboBox            *m_ratioCB;
    QComboBox            *m_orientCB;
    QComboBox            *m_guideLinesCB;

    QPushButton          *m_centerWidth;
    QPushButton          *m_centerHeight;

    QCheckBox            *m_goldenSectionBox;
    QCheckBox            *m_goldenSpiralSectionBox;
    QCheckBox            *m_goldenSpiralBox;
    QCheckBox            *m_goldenTriangleBox;
    QCheckBox            *m_flipHorBox;
    QCheckBox            *m_flipVerBox;
    QCheckBox            *m_autoOrientation;

    QSpinBox             *m_guideSize;

    KIntNumInput         *m_widthInput;
    KIntNumInput         *m_heightInput;
    KIntNumInput         *m_xInput;
    KIntNumInput         *m_yInput;

    KIntSpinBox          *m_customRatioNInput;
    KIntSpinBox          *m_customRatioDInput;

    KColorButton         *m_guideColorBt;

    ImageSelectionWidget *m_imageSelectionWidget;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_RATIOCROP_H */
