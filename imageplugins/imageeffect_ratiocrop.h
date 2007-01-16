/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Jaromir Malenko <malenko at email.cz>
 * Date   : 2004-12-06
 * Description : digiKam image editor Ratio Crop tool
 *
 * Copyright 2004-2007 by Gilles Caulier
 * Copyright 2007 by Jaromir Malenko
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

namespace Digikam
{
class ImageSelectionWidget;
}

namespace DigikamImagesPluginCore
{

class ImageEffect_RatioCrop : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_RatioCrop(QWidget *parent);
    ~ImageEffect_RatioCrop();

private:
    
    void readSettings(void);
    void writeSettings(void);
    
    void applyRatioChanges(int a);
    
private slots:

    void slotUser1();
    void slotDefault();
    void slotOk();

    void slotCenterWidth(void);
    void slotCenterHeight(void);
    void slotXChanged(int x);
    void slotYChanged(int y);
    void slotWidthChanged(int w);
    void slotHeightChanged(int h);
    void slotCustomRatioChanged(void);
    void slotOrientChanged(int o);
    void slotAutoOrientChanged(bool a);
    void slotRatioChanged(int a);
    void slotSelectionChanged(QRect rect );
    void slotSelectionWidthChanged(int newWidth);
    void slotSelectionHeightChanged(int newHeight);
    void slotSelectionOrientationChanged(int);
    void slotGuideTypeChanged(int t);
    void slotGoldenGuideTypeChanged(void);

private:
    
    QLabel                        *m_customLabel1;
    QLabel                        *m_customLabel2;
    QLabel                        *m_orientLabel;
    QLabel                        *m_colorGuideLabel;
    
    QComboBox                     *m_ratioCB;
    QComboBox                     *m_orientCB;
    QComboBox                     *m_guideLinesCB;
    
    QPushButton                   *m_centerWidth;
    QPushButton                   *m_centerHeight;
    
    QCheckBox                     *m_goldenSectionBox;
    QCheckBox                     *m_goldenSpiralSectionBox;
    QCheckBox                     *m_goldenSpiralBox;
    QCheckBox                     *m_goldenTriangleBox;
    QCheckBox                     *m_flipHorBox;
    QCheckBox                     *m_flipVerBox;
    QCheckBox                     *m_autoOrientation;
    
    QSpinBox                      *m_guideSize;
    
    KIntNumInput                  *m_widthInput;
    KIntNumInput                  *m_heightInput;
    KIntNumInput                  *m_xInput;    
    KIntNumInput                  *m_yInput;    
     
    KIntSpinBox                   *m_customRatioNInput;
    KIntSpinBox                   *m_customRatioDInput;
    
    KColorButton                  *m_guideColorBt;
    
    Digikam::ImageSelectionWidget *m_imageSelectionWidget;
};

}  // NameSpace DigikamImagesPluginCore

#endif /* IMAGEEFFECT_RATIOCROP_H */
