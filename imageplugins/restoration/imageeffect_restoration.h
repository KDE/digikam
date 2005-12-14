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

#include <qstring.h>

// Digikam includes.

#include <digikamheaders.h>

class QLabel;
class QCheckBox;
class QComboBox;
class QTabWidget;
class QFrame;

class KDoubleNumInput;
class KIntNumInput;

namespace DigikamRestorationImagesPlugin
{

class ImageEffect_Restoration : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Restoration(QWidget* parent, QString title, QFrame* banner);
    ~ImageEffect_Restoration();
    
private:

    enum RestorationFilteringPreset
    {
    NoPreset=0,
    ReduceUniformNoise,
    ReduceJPEGArtefacts,
    ReduceTexturing,
    };

    QLabel          *m_detailLabel;
    QLabel          *m_gradientLabel;
    QLabel          *m_timeStepLabel;
    QLabel          *m_blurLabel;
    QLabel          *m_blurItLabel;
    QLabel          *m_angularStepLabel;
    QLabel          *m_integralStepLabel;
    QLabel          *m_gaussianLabel;

    QTabWidget      *m_mainTab;        
            
    // Preset Settings.
    QComboBox       *m_restorationTypeCB;  
    
    // Smoothing settings.
    KDoubleNumInput *m_detailInput;
    KDoubleNumInput *m_gradientInput;
    KDoubleNumInput *m_timeStepInput;
    KDoubleNumInput *m_blurInput;
    KDoubleNumInput *m_blurItInput;
    
    // Advanced Settings.
    KDoubleNumInput *m_angularStepInput;
    KDoubleNumInput *m_integralStepInput;
    KDoubleNumInput *m_gaussianInput;
    
    QCheckBox       *m_linearInterpolationBox;
    QCheckBox       *m_normalizeBox;
    
private slots:

    void slotCheckSettings(void);
    void slotUser2();
    void slotUser3();
    void processCImgURL(const QString&);

protected:
    
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);
};
    
}  // NameSpace DigikamRestorationImagesPlugin

#endif /* IMAGEEFFECT_RESTORATION_H */
