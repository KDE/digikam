/* ============================================================
 * File  : imageeffect_antivignetting.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-25
 * Description : a digiKam image plugin for to reduce 
 *               vignetting on an image.
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

#ifndef IMAGEEFFECT_ANTIVIGNETTING_H
#define IMAGEEFFECT_ANTIVIGNETTING_H

// KDE includes.

#include <kdialogbase.h>

class QPushButton;
class QSpinBox;
class QSlider;
class QLabel;
class QTabWidget;

class KProgress;

namespace Digikam
{
class ImageWidget;
}

namespace DigikamAntiVignettingImagesPlugin
{

class ImageEffect_AntiVignetting : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_AntiVignetting(QWidget *parent);
    ~ImageEffect_AntiVignetting();

protected:

    void closeEvent(QCloseEvent *e);
    
    void antiVignetting(uint *data, int Width, int Height, 
                        double density, double power, double radius,
                        int xshift, int yshift, bool progress=true);
    
private:
    
    bool                  m_cancel;
    
    QWidget              *m_parent;
    
    QPushButton          *m_helpButton;

    QSlider              *m_densitySlider;
    QSlider              *m_powerSlider;        
    QSlider              *m_radiusSlider;    
    QSlider              *m_brightnessSlider;    
    QSlider              *m_contrastSlider;    
    QSlider              *m_gammaSlider;    
    
    QSpinBox             *m_densitySpinBox;
    QSpinBox             *m_powerSpinBox;
    QSpinBox             *m_radiusSpinBox;
    QSpinBox             *m_brightnessSpinBox;
    QSpinBox             *m_contrastSpinBox;
    QSpinBox             *m_gammaSpinBox;
    
    QLabel               *m_maskPreviewLabel;
    
    QTabWidget           *m_mainTab;
    
    KProgress            *m_progressBar;
    
    Digikam::ImageWidget *m_previewWidget;

private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
};

}  // NameSpace DigikamAntiVignettingImagesPlugin

#endif /* IMAGEEFFECT_ANTIVIGNETTING_H */
