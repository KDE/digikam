/* ============================================================
 * File  : imageeffect_oilpaint.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-25
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef IMAGEEFFECT_OILPAINT_H
#define IMAGEEFFECT_OILPAINT_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QSlider;

class KDoubleSpinBox;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamOilPaintImagesPlugin
{

class ImageEffect_OilPaint : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_OilPaint(QWidget* parent);
    ~ImageEffect_OilPaint();

protected:

    void closeEvent(QCloseEvent *e);
    
private:

    QWidget         *m_parent;
    
    QPushButton     *m_helpButton;
    
    QSlider         *m_radiusSlider;
    
    KDoubleSpinBox  *m_radiusInput;

    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    
    void slotSliderRadiusChanged(int v);
    void slotSpinBoxRadiusChanged(double v);
    
};

}  // NameSpace DigikamOilPaintImagesPlugin

#endif /* IMAGEEFFECT_OILPAINT_H */
