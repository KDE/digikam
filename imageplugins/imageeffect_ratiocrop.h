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
    
    enum RatioAspect           // Standard photograph paper ratio.
    {
    RATIO03X04 = 0,            //  3 x  4
    RATIO02x03,                //  2 x  3
    RATIO05x07,                //  5 x  7
    RATIO04X05,                //  4 x  5
    RATIO07x10,                //  7 x 10
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
