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
class QGridLayout;
class QGroupBox;
class QTimer;

class KProgress;
class KDoubleNumInput;

namespace Digikam
{
class ImageGuideWidget;
}

namespace DigikamLensDistortionImagesPlugin
{
class LensDistortion;

class ImageEffect_LensDistortion : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_LensDistortion(QWidget *parent);
    ~ImageEffect_LensDistortion();

protected:

    void closeEvent(QCloseEvent *e);

private:

    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };
    
    int                   m_currentRenderingMode;
        
    QWidget              *m_parent;
    
    QPushButton          *m_helpButton;

    QTimer               *m_timer;
    
    QLabel               *m_maskPreviewLabel;
        
    KDoubleNumInput      *m_mainInput;
    KDoubleNumInput      *m_edgeInput;
    KDoubleNumInput      *m_rescaleInput;
    KDoubleNumInput      *m_brightenInput;

    KProgress            *m_progressBar;

    LensDistortion       *m_lensdistortionFilter;
    
    Digikam::ImageGuideWidget *m_previewWidget;
    
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

}  // NameSpace DigikamLensDistortionImagesPlugin

#endif /* IMAGEEFFECT_LENSDISTORTION_H */
