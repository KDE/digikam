/* ============================================================
 * File  : imageeffect_infrared.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-22
 * Description : a digiKam image editor plugin for simulate 
 *               infrared film.
 * 
 * Copyright 2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_INFRARED_H
#define IMAGEEFFECT_INFRARED_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QSlider;
class QLCDNumber;
class QCheckBox;

namespace Digikam
{
class ImagePannelWidget;
}

namespace DigikamInfraredImagesPlugin
{
class Infrared;

class ImageEffect_Infrared : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Infrared(QWidget* parent);
    ~ImageEffect_Infrared();

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
    
    QCheckBox   *m_addFilmGrain; 
                
    QSlider     *m_sensibilitySlider;
    
    QLCDNumber  *m_sensibilityLCDValue;
        
    Infrared    *m_infraredFilter;
    
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

}  // NameSpace DigikamInfraredImagesPlugin

#endif /* IMAGEEFFECT_INFRARED_H */
