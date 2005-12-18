/* ============================================================
 * File  : imageeffect_perspective.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-17
 * Description : a digiKam image editor plugin for process image 
 *               perspective adjustment.
 * 
 * Copyright 2005 by Gilles Caulier
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

#include <digikamheaders.h>

class QPushButton;
class QLabel;

namespace DigikamPerspectiveImagesPlugin
{

class PerspectiveWidget;

class ImageEffect_Perspective : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageEffect_Perspective(QWidget* parent, QString title, QFrame* banner);
    ~ImageEffect_Perspective();

private:

    QLabel            *m_newWidthLabel;
    QLabel            *m_newHeightLabel;
    QLabel            *m_topLeftAngleLabel;
    QLabel            *m_topRightAngleLabel;
    QLabel            *m_bottomLeftAngleLabel;
    QLabel            *m_bottomRightAngleLabel;
    
    PerspectiveWidget *m_previewWidget;
                                                                    
private slots:

    void slotOk();
    void slotDefault();
    void slotUpdateInfo(QRect newSize, float topLeftAngle, float topRightAngle,
                        float bottomLeftAngle, float bottomRightAngle);
};

}  // NameSpace DigikamPerspectiveImagesPlugin

#endif /* IMAGEEFFECT_PERSPECTIVE_H */
