/* ============================================================
 * File  : imageeffect_charcoal.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digikam image editor plugin for to
 *               simulate charcoal drawing.
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

#ifndef IMAGEEFFECT_CHARCOAL_H
#define IMAGEEFFECT_CHARCOAL_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QSlider;

class KDoubleSpinBox;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamCharcoalImagesPlugin
{

class ImageEffect_Charcoal : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Charcoal(QWidget* parent);
    ~ImageEffect_Charcoal();

protected:

    void closeEvent(QCloseEvent *e);
    
private:

    QWidget         *m_parent;
    
    QPushButton     *m_helpButton;
    
    QSlider         *m_radiusSlider;
    QSlider         *m_sigmaSlider;
    
    KDoubleSpinBox  *m_radiusInput;
    KDoubleSpinBox  *m_sigmaInput;

    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    
    void slotSliderRadiusChanged(int v);
    void slotSpinBoxRadiusChanged(double v);
    void slotSliderSigmaChanged(int v);
    void slotSpinBoxSigmaChanged(double v);
    
};

}  // NameSpace DigikamCharcoalImagesPlugin

#endif /* IMAGEEFFECT_CHARCOAL_H */
