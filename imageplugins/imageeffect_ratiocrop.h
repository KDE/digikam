/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-06
 * Description : Ratio crop tool for ImageEditor
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

#ifndef IMAGEEFFECT_RATIOCROP_H
#define IMAGEEFFECT_RATIOCROP_H

// KDE include.

#include <kdialogbase.h>

class QComboBox;

class KIntNumInput;

namespace Digikam
{
class ImagePanIconWidget;
}

class ImageEffect_RatioCrop : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_RatioCrop(QWidget *parent);
    ~ImageEffect_RatioCrop();

private:

    QWidget      *m_parent;
    
    QComboBox    *m_ratioCB;
    QComboBox    *m_orientCB;
    
    KIntNumInput *m_widthInput;
    KIntNumInput *m_heightInput;
    
    Digikam::ImagePanIconWidget *m_imageSelectionWidget;
    
    enum RatioAspect           // Standard photograph paper sizes.
    {
    CMS06X08 = 0,              //  6 x  8 cm
    CMS09X13,                  //  9 x 13 cm
    CMS10x15,                  // 10 x 15 cm
    CMS11x15,                  // 11 x 15 cm
    CMS13x18,                  // 13 x 18 cm
    CMS13x19,                  // 13 x 19 cm
    CMS15x20,                  // 15 x 20 cm
    CMS15x21,                  // 15 x 21 cm
    CMS18x24,                  // 18 x 24 cm
    CMS18x25,                  // 18 x 25 cm
    CMS20x27,                  // 20 x 27 cm
    CMS20x30,                  // 20 x 30 cm
    CMS21x30,                  // 21 x 30 cm
    CMS25x38,                  // 25 x 38 cm
    CMS30x40,                  // 30 x 40 cm
    CMS30x45,                  // 30 x 45 cm
    CMS40x50,                  // 40 x 50 cm
    };
    
    void updateSelectionSize(QRect rect);
    
private slots:

    void slotUser1();
    void slotOk();
    void slotWidthChanged(int w);
    void slotHeightChanged(int h);
    void slotOrientChanged(int o);
    void slotRatioChanged(void);
    void slotSelectionMoved(QRect rect, bool target);
    void slotSelectionChanged(QRect rect);
};

#endif /* IMAGEEFFECT_RATIOCROP_H */
