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
class ImageSelectionWidget;
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
    
    Digikam::ImageSelectionWidget *m_imageSelectionWidget;
    
private slots:

    void slotUser1();
    void slotOk();
    void slotWidthChanged(int w);
    void slotHeightChanged(int h);
    void slotOrientChanged(int o);
    void slotRatioChanged(int a);
    void slotSelectionChanged(QRect rect);
};

#endif /* IMAGEEFFECT_RATIOCROP_H */
