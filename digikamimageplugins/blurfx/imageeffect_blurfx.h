/* ============================================================
 * File  : imageeffect_blurfx.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-09
 * Description : 
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

#ifndef IMAGEEFFECT_BLURFX_H
#define IMAGEEFFECT_BLURFX_H

// Qt includes.

#include <qcolor.h>
#include <qrect.h>

// KDE includes.

#include <kdialogbase.h>

class QPushButton;
class QComboBox;
class QLabel;
class QTimer;
class QRect;

class KProgress;
class KIntNumInput;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamBlurFXImagesPlugin
{
class BlurFX;

class ImageEffect_BlurFX : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_BlurFX(QWidget *parent);
    ~ImageEffect_BlurFX();

protected:

    void closeEvent(QCloseEvent *e);
    
private:
    
    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };
    
    int                          m_currentRenderingMode;

    QWidget                     *m_parent;
    
    QPushButton                 *m_helpButton;

    QComboBox                   *m_effectType;
    
    QTimer                      *m_timer;
    
    QLabel                      *m_effectTypeLabel;
    QLabel                      *m_distanceLabel;
    QLabel                      *m_levelLabel;
    
    KIntNumInput                *m_distanceInput;
    KIntNumInput                *m_levelInput;
    
    BlurFX                      *m_BlurFXFilter;
        
    Digikam::ImagePreviewWidget *m_previewWidget;

private:
    
    void abortPreview(void);
    void customEvent(QCustomEvent *event);
        
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotEffectTypeChanged(int type);
    void slotTimer();
};

}  // NameSpace DigikamBlurFXImagesPlugin

#endif /* IMAGEEFFECT_BLURFX_H */
