/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-02-17
 * Description : a digiKam image editor plugin for process image 
 *               perspective adjustment.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

#ifndef IMAGEEFFECT_PERSPECTIVE_H
#define IMAGEEFFECT_PERSPECTIVE_H

// Qt includes.

#include <qrect.h>

// Digikam includes.

#include "imagedlgbase.h"

class QLabel;
class QCheckBox;
class QSpinBox;

class KColorButton;

namespace DigikamPerspectiveImagesPlugin
{

class PerspectiveWidget;

class ImageEffect_Perspective : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_Perspective(QWidget* parent);
    ~ImageEffect_Perspective();

private slots:

    void slotUpdateInfo(QRect newSize, float topLeftAngle, float topRightAngle,
                        float bottomLeftAngle, float bottomRightAngle);

private:

    void readUserSettings();
    void writeUserSettings();
    void resetValues();
    void finalRendering();

private:

    QLabel            *m_newWidthLabel;
    QLabel            *m_newHeightLabel;
    QLabel            *m_topLeftAngleLabel;
    QLabel            *m_topRightAngleLabel;
    QLabel            *m_bottomLeftAngleLabel;
    QLabel            *m_bottomRightAngleLabel;

    QCheckBox         *m_drawWhileMovingCheckBox;
    QCheckBox         *m_drawGridCheckBox;

    QSpinBox          *m_guideSize;

    KColorButton      *m_guideColorBt;

    PerspectiveWidget *m_previewWidget;
};

}  // NameSpace DigikamPerspectiveImagesPlugin

#endif /* IMAGEEFFECT_PERSPECTIVE_H */
