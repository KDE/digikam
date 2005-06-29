/* ============================================================
 * File  : imageeffect_raindrop.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-09-30
 * Description : a digiKam image plugin for to add
 *               raindrops on an image.
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

#ifndef IMAGEEFFECT_RAINDROP_H
#define IMAGEEFFECT_RAINDROP_H

// KDE includes.

#include <kdialogbase.h>

class QPushButton;
class QSpinBox;
class QSlider;
class QTimer;

class KProgress;
class KIntNumInput;

namespace Digikam
{
class ImageWidget;
}

namespace DigikamRainDropImagesPlugin
{
class RainDrop;

class ImageEffect_RainDrop : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_RainDrop(QWidget *parent);
    ~ImageEffect_RainDrop();

protected:

    void closeEvent(QCloseEvent *e);
    //void resizeEvent(QResizeEvent *e);
    
private:
    
    
    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };
    
    int           m_currentRenderingMode;
        
    QWidget      *m_parent;
    
    QPushButton  *m_helpButton;

    QTimer       *m_timer;
            
    KIntNumInput *m_dropInput;
    KIntNumInput *m_amountInput;
    KIntNumInput *m_coeffInput;    
    
    KProgress    *m_progressBar;
    
    RainDrop     *m_raindropFilter;
    
    Digikam::ImageWidget *m_previewWidget;
    
private:
    
    void abortPreview(void);
    void customEvent(QCustomEvent *event);
        
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotTimer();
    void slotResized(void);
};

}  // NameSpace DigikamRainDropImagesPlugin

#endif /* IMAGEEFFECT_RAINDROP_H */
