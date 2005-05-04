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

class QLabel;
class QComboBox;
class QCheckBox;

class KIntNumInput;
class KIntSpinBox;

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
    
    QLabel       *m_customLabel1;
    QLabel       *m_customLabel2;
    
    QComboBox    *m_ratioCB;
    QComboBox    *m_orientCB;
    
    QCheckBox    *m_useRuleThirdLines;
    
    KIntNumInput *m_widthInput;
    KIntNumInput *m_heightInput;
    KIntNumInput *m_xInput;    
    KIntNumInput *m_yInput;    
     
    KIntSpinBox  *m_customRatioNInput;
    KIntSpinBox  *m_customRatioDInput;
    
    Digikam::ImageSelectionWidget *m_imageSelectionWidget;
    
    void readSettings(void);
    void writeSettings(void);
    
    void applyRatioChanges(int a);
    
private slots:

    void slotUser1();
    void slotUser2();
    void slotOk();
    
    void slotXChanged(int x);
    void slotYChanged(int y);
    void slotWidthChanged(int w);
    void slotHeightChanged(int h);
    void slotCustomRatioChanged(void);
    void slotOrientChanged(int o);
    void slotRatioChanged(int a);
    void slotSelectionChanged(QRect rect );
    void slotSelectionWidthChanged(int newWidth);
    void slotSelectionHeightChanged(int newHeight);
};

#endif /* IMAGEEFFECT_RATIOCROP_H */
