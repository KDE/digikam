/* ============================================================
 * File  : imageeffect_freerotation.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-28
 * Description : a Digikam image editor plugin for process image 
 *               free rotatio
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_FREEROTATION_H
#define IMAGEEFFECT_FREEROTATION_H

// Qt include.

#include <qimage.h>

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QLabel;

class KDoubleNumInput;

namespace Digikam
{
class ImageGuideWidget;
}

namespace DigikamFreeRotationImagesPlugin
{

class ImageEffect_FreeRotation : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_FreeRotation(QWidget* parent);
    ~ImageEffect_FreeRotation();

private:
    
    QLabel                    *m_newWidthLabel;
    QLabel                    *m_newHeightLabel;

    QWidget                   *m_parent;
    
    QPushButton               *m_helpButton;
    
    KDoubleNumInput           *m_angleInput;

    Digikam::ImageGuideWidget *m_previewWidget;
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotUser1();
   
};

}  // NameSpace DigikamFreeRotationImagesPlugin

#endif /* IMAGEEFFECT_FREEROTATION_H */
