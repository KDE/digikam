/* ============================================================
 * File  : imageeffect_emboss.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a Digikam image editor plugin for to emboss 
 *               an image.
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

#ifndef IMAGEEFFECT_EMBOSS_H
#define IMAGEEFFECT_EMBOSS_H

// Qt include.

#include <qimage.h>

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QSlider;

class KDoubleSpinBox;
class KProgress;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamEmbossImagesPlugin
{

class ImageEffect_Emboss : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Emboss(QWidget* parent);
    ~ImageEffect_Emboss();

protected:

    void closeEvent(QCloseEvent *e);
    
private:

    bool             m_cancel;

    QWidget         *m_parent;
    
    QPushButton     *m_helpButton;
    
    QSlider         *m_radiusSlider;
    QSlider         *m_sigmaSlider;
    
    KDoubleSpinBox  *m_radiusInput;
    KDoubleSpinBox  *m_sigmaInput;
    
    KProgress    *m_progressBar;
    
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private:

    void Emboss(uint* data, int w, int h, float Depth);
    inline int Lim_Max (int Now, int Up, int Max);
    inline uchar LimitValues (int ColorValue);
    
    /*
    QImage Emboss(QImage &image, double radius, double sigma);    
    int getOptimalKernelWidth(double radius, double sigma);
    bool convolveImage(QImage *image, QImage *dest, const unsigned int order, const double *kernel);
    */
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    
    void slotSliderRadiusChanged(int v);
    void slotSpinBoxRadiusChanged(double v);
    void slotSliderSigmaChanged(int v);
    void slotSpinBoxSigmaChanged(double v);
    
};

}  // NameSpace DigikamEmbossImagesPlugin

#endif /* IMAGEEFFECT_EMBOSS_H */
