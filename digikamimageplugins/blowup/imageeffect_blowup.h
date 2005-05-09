/* ============================================================
 * File  : imageeffect_blowup.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-07
 * Description : a digiKam image editor plugin to blowup 
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

#ifndef IMAGEEFFECT_BLOWUP_H
#define IMAGEEFFECT_BLOWUP_H

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

class KIntNumInput;
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

namespace DigikamBlowUpImagesPlugin
{

class ImageEffect_BlowUp : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_BlowUp(QWidget* parent);
    ~ImageEffect_BlowUp();
       
protected:

    void closeEvent(QCloseEvent *e);
    
private:

    enum RunningMode
    {
    NoneRendering=0,
    FinalRendering
    };

    int              m_currentRenderingMode;
    
    double           m_aspectRatio;
    
    QImage           m_originalImage;
    QImage           m_resizedImage;
    
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
    
    KIntNumInput    *m_newWidth;
    KIntNumInput    *m_newHeight;
    
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
    QCheckBox       *m_preserveRatioBox;
    
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
    void slotAdjustRatioFromWidth(int w);
    void slotAdjustRatioFromHeight(int h);
};
    
}  // NameSpace DigikamBlowUpImagesPlugin

#endif /* IMAGEEFFECT_BLOWUP_H */
