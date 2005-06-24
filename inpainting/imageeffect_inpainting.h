/* ============================================================
 * File  : imageeffect_inpainting.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint 
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

#ifndef IMAGEEFFECT_INPAINTING_H
#define IMAGEEFFECT_INPAINTING_H

// Qt include.

#include <qimage.h>
#include <qrect.h>
#include <qstring.h>

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QLabel;
class QCheckBox;
class QTimer;
class QCustomEvent;
class QComboBox;
class QTabWidget;

class KDoubleNumInput;
class KProgress;

namespace DigikamImagePlugins
{
class CimgIface;
}

namespace Digikam
{
class ImageIface;
}

namespace DigikamInPaintingImagesPlugin
{

class ImageEffect_InPainting
{
public:

    static void inPainting(QWidget *parent);
};


class ImageEffect_InPainting_Dialog : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_InPainting_Dialog(QWidget* parent);
    ~ImageEffect_InPainting_Dialog();
       
protected:

    void closeEvent(QCloseEvent *e);
    
private:

    enum InPaintingFilteringPreset
    {
    NoPreset=0,
    RemoveSmallArtefact,
    RemoveMediumArtefact,
    RemoveLargeArtefact
    };
    
    enum RunningMode
    {
    NoneRendering=0,
    FinalRendering
    };

    int              m_currentRenderingMode;
    
    QImage           m_originalImage;
    QImage           m_cropImage;
    QImage           m_maskImage;
    
    QRect            m_maskRect;
    
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
    
    // Preset Settings.
    QComboBox       *m_inpaintingTypeCB;  
    
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
    
    QTabWidget      *m_mainTab;
    
    KProgress       *m_progressBar;
    
    DigikamImagePlugins::CimgIface       *m_cimgInterface;
    
    Digikam::ImageIface                  *m_iface;    
    
    void customEvent(QCustomEvent *event);
    
private slots:

    void slotHelp();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotUser2();
    void slotUser3();
    void processCImgURL(const QString&);
};
    
}  // NameSpace DigikamInPaintingImagesPlugin

#endif /* IMAGEEFFECT_INPAINTING_H */
