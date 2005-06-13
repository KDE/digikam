/* ============================================================
 * File  : imageeffect_despeckle.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-24
 * Description : noise reduction image filter for digiKam 
 *               image editor.
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

#ifndef IMAGEEFFECT_DESPECKLE_H
#define IMAGEEFFECT_DESPECKLE_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QCheckBox;
class QTimer;

class KIntNumInput;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamNoiseReductionImagesPlugin
{
class Despeckle;

class ImageEffect_Despeckle : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Despeckle(QWidget* parent);
    ~ImageEffect_Despeckle();

protected:

    void closeEvent(QCloseEvent *e);
       
private:

    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };
    
    int           m_currentRenderingMode;
    
    QWidget      *m_parent;
        
    QTimer       *m_timer;
        
    QPushButton  *m_helpButton;
    
    KIntNumInput *m_radiusInput;
    KIntNumInput *m_blackLevelInput;
    KIntNumInput *m_whiteLevelInput;
        
    QCheckBox    *m_useAdaptativeMethod;
    QCheckBox    *m_useRecursiveMethod;
    
    Despeckle    *m_despeckleFilter;
    
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;

private:
    
    void abortPreview(void);
    void customEvent(QCustomEvent *event);
            
private slots:

    void slotHelp();
    void slotUser1();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotTimer(); 
        
};

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* IMAGEEFFECT_DESPECKLE_H */
