/* ============================================================
 * File  : imageeffect_restoration.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-26
 * Description : a digiKam image editor plugin to restore 
 *               a photograph
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

#ifndef IMAGEEFFECT_RESTORATION_H
#define IMAGEEFFECT_RESTORATION_H

// Qt include.

#include <qimage.h>

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QLabel;
class QCheckBox;
class QTimer;
class QComboBox;
class QCustomEvent;

class KDoubleNumInput;
class KIntNumInput;
class KProgress;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamImagePlugins
{
class CimgIface;
}

namespace DigikamRestorationImagesPlugin
{

class ImageEffect_Restoration : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Restoration(QWidget* parent);
    ~ImageEffect_Restoration();
       
protected:

    void closeEvent(QCloseEvent *e);
    
private:

    enum RestorationFilteringPreset
    {
    NoPreset=0,
    ReduceUniformNoise,
    ReduceJPEGArtefacts,
    ReduceTexturing,
    };

    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };

    bool             m_dirty;
    
    int              m_currentRenderingMode;
    
    uint            *m_originalData;
    
    QImage           m_previewImage;
    QWidget         *m_parent;
    
    QPushButton     *m_helpButton;
    
    QLabel          *m_detailLabel;
    QLabel          *m_gradientLabel;
    QLabel          *m_timeStepLabel;
    QLabel          *m_blurLabel;
    QLabel          *m_blurItLabel;
    QLabel          *m_angularStepLabel;
    QLabel          *m_integralStepLabel;
    QLabel          *m_gaussianLabel;
    
    KDoubleNumInput *m_detailInput;
    KDoubleNumInput *m_gradientInput;
    KDoubleNumInput *m_timeStepInput;
    KDoubleNumInput *m_blurInput;
    KDoubleNumInput *m_angularStepInput;
    KDoubleNumInput *m_integralStepInput;
    KDoubleNumInput *m_gaussianInput;
    KDoubleNumInput *m_blurItInput;
    
    QCheckBox       *m_linearInterpolationBox;
    QCheckBox       *m_normalizeBox;
    
    QComboBox       *m_restorationTypeCB;  
        
    QTimer          *m_timer;
    
    KProgress       *m_progressBar;
    
    DigikamImagePlugins::CimgIface       *m_cimgInterface;
    
    Digikam::ImagePreviewWidget          *m_imagePreviewWidget;
    
    void customEvent(QCustomEvent *event);
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotTimer();

};
    
}  // NameSpace DigikamRestorationImagesPlugin

#endif /* IMAGEEFFECT_RESTORATION_H */
