/* ============================================================
 * File  : imageeffect_whitebalance.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-11
 * Description : a digiKam image editor plugin to correct 
 *               image white balance 
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

#ifndef IMAGEEFFECT_WHITEBALANCE_H
#define IMAGEEFFECT_WHITEBALANCE_H

// Qt include.

#include <qcolor.h>

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QLabel;
class QComboBox;
class QPushButton;

namespace Digikam
{
class ImageGuideWidget;
class ImageWidget;
class ColorGradientWidget;
class CurvesWidget;
class ImageCurves;
}

namespace DigikamWhiteBalanceImagesPlugin
{

class ImageEffect_WhiteBalance : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_WhiteBalance(QWidget* parent, uint *imageData, uint width, uint height);
    ~ImageEffect_WhiteBalance();
    
protected:

    void closeEvent(QCloseEvent *e);    

private:

    enum TargetColor 
    {
    WhiteColor=0,
    BlackColor
    };
    
    QWidget                      *m_parent;
    
    QPushButton                  *m_helpButton;
    
    QComboBox                    *m_channelCB;    
    QComboBox                    *m_scaleCB;  
    
    QPushButton                  *m_blackColorButton;
    QPushButton                  *m_whiteColorButton;
    
    QColor                        m_blackColor;
    QColor                        m_whiteColor;
    
    Digikam::CurvesWidget        *m_whiteBalanceCurvesWidget;
    Digikam::ImageCurves         *m_whiteBalanceCurves;
    
    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::ColorGradientWidget *m_vGradient;
    
    Digikam::ImageGuideWidget    *m_previewOriginalWidget;
    Digikam::ImageWidget         *m_previewTargetWidget; 

private:
        
    void setWhiteColor(QColor color);
    void setBlackColor(QColor color);
    void whiteBalance(uint *data, int w, int h, QColor bColor, QColor wColor);

private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotUser1();
    void slotColorSelectedFromImage( const QColor &color );
    void slotScaleChanged(int scale);
    void slotChannelChanged(int channel);
};

}  // NameSpace DigikamWhiteBalanceImagesPlugin

#endif /* IMAGEEFFECT_WHITEBALANCE_H */
