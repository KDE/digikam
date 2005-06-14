/* ============================================================
 * File  : imageeffect_lensdistortion.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-27
 * Description : a digiKam image plugin for to reduce spherical
 *               aberration provide by lens on an image.
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

#ifndef IMAGEEFFECT_LENSDISTORTION_H
#define IMAGEEFFECT_LENSDISTORTION_H

// Qt includes.

#include <qimage.h>

// KDE includes.

#include <kdialogbase.h>

class QPushButton;
class QSpinBox;
class QSlider;
class QGridLayout;
class QGroupBox;

class KProgress;

namespace Digikam
{
class ImageGuideWidget;
}

namespace DigikamLensDistortionImagesPlugin
{

class ImageEffect_LensDistortion : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_LensDistortion(QWidget *parent);
    ~ImageEffect_LensDistortion();

private:

    double                m_normallise_radius_sq;
    double                m_centre_x;
    double                m_centre_y;
    double                m_mult_sq;
    double                m_mult_qd;
    double                m_rescale;
    double                m_brighten;
    
    bool                  m_cancel;
    
    QWidget              *m_parent;
    
    QPushButton          *m_helpButton;

    QSlider              *m_mainSlider;
    QSlider              *m_edgeSlider;        
    QSlider              *m_rescaleSlider;    
    QSlider              *m_brightenSlider;    
    
    QSpinBox             *m_mainSpinBox;
    QSpinBox             *m_edgeSpinBox;
    QSpinBox             *m_rescaleSpinBox;
    QSpinBox             *m_brightenSpinBox;

    KProgress            *m_progressBar;

    QLabel               *m_maskPreviewLabel;
    
    Digikam::ImageGuideWidget *m_previewWidget;
    
protected:

    void closeEvent(QCloseEvent *e);
    
    void wideangle(uint *data, int Width, int Height, double main, double edge, 
                   double rescale, double brighten, int centre_x, int centre_y,
                   bool progress=true);
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
};

}  // NameSpace DigikamLensDistortionImagesPlugin

#endif /* IMAGEEFFECT_LENSDISTORTION_H */
