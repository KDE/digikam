/* ============================================================
 * File  : imageeffect_sheartool.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
  * Date  : 2004-12-23
 * Description : a Digikam image editor plugin for process 
 *               shearing image.
* 
 * Copyright 2004 by Gilles Caulier
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

#ifndef IMAGEEFFECT_SHEARTOOL_H
#define IMAGEEFFECT_SHEARTOOL_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;

class KIntNumInput;

namespace Digikam
{
class ImageWidget;
}

namespace DigikamShearToolImagesPlugin
{

class ImageEffect_ShearTool : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_ShearTool(QWidget* parent);
    ~ImageEffect_ShearTool();

private:

    float                m_ratioW, m_ratioH;   // Ratio coef. between preview and original image.

    QWidget              *m_parent;
    
    QPushButton          *m_helpButton;
    
    Digikam::ImageWidget *m_previewWidget;
    
    KIntNumInput          *m_magnitudeX;
    KIntNumInput          *m_magnitudeY;

private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotUser1();
   
};

}  // NameSpace DigikamShearToolImagesPlugin

#endif /* IMAGEEFFECT_SHEARTOOL_H */
