/* ============================================================
 * File  : imageeffect_filmgrain.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a Digikam image editor plugin for to add film 
 *               grain on an image.
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

#ifndef IMAGEEFFECT_FILMGRAIN_H
#define IMAGEEFFECT_FILMGRAIN_H

// Qt include.

#include <qimage.h>

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QSlider;
class QLCDNumber;

class KProgress;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamFilmGrainImagesPlugin
{

class ImageEffect_FilmGrain : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_FilmGrain(QWidget* parent);
    ~ImageEffect_FilmGrain();

protected:

    void closeEvent(QCloseEvent *e);
    
private:

    bool         m_cancel;

    QWidget     *m_parent;
    
    QPushButton *m_helpButton;
    
    QSlider     *m_sensibilitySlider;
    
    QLCDNumber  *m_sensibilityLCDValue;
    
    KProgress   *m_progressBar;
    
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private:

    void FilmGrain(uint* data, int Width, int Height, int Sensibility);
    inline uchar LimitValues (int ColorValue);
    
    int randomize_value (int now, int min, int max, int mod_p, int rand_max, int holdness);
    
    void scatter_hsv_scatter (uchar *r, uchar *g, uchar *b, 
                              int hue, int saturation, int value, int holdness);

    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotSensibilityChanged(int);
   
};

}  // NameSpace DigikamFilmGrainImagesPlugin

#endif /* IMAGEEFFECT_FILMGRAIN_H */
