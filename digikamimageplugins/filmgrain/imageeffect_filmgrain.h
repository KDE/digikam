/* ============================================================
 * File  : imageeffect_filmgrain.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin for add film 
 *               grain on an image.
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

#ifndef IMAGEEFFECT_FILMGRAIN_H
#define IMAGEEFFECT_FILMGRAIN_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QSlider;
class QLCDNumber;

namespace Digikam
{
class ImagePannelWidget;
}

namespace DigikamFilmGrainImagesPlugin
{
class FilmGrain;

class ImageEffect_FilmGrain : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_FilmGrain(QWidget* parent);
    ~ImageEffect_FilmGrain();

protected:

    void closeEvent(QCloseEvent *e);
        
private:

    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };
    
    int          m_currentRenderingMode;

    QWidget     *m_parent;
    
    QPushButton *m_helpButton;
    
    QSlider     *m_sensibilitySlider;
    
    QLCDNumber  *m_sensibilityLCDValue;
    
    FilmGrain   *m_filmgrainFilter;
    
    Digikam::ImagePannelWidget *m_imagePreviewWidget;

private:
    
    void abortPreview(void);
    void customEvent(QCustomEvent *event);
            
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotSensibilityChanged(int);
    void slotFocusChanged(void);    
   
};

}  // NameSpace DigikamFilmGrainImagesPlugin

#endif /* IMAGEEFFECT_FILMGRAIN_H */
