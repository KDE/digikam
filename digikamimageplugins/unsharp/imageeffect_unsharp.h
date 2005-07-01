/* ============================================================
 * File  : imageeffect_unsharp.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-27
 * Description : Unsharp mask image filter for digiKam Image Editor
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

#ifndef IMAGEEFFECT_UNSHARP_H
#define IMAGEEFFECT_UNSHARP_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QTimer;

class KIntNumInput;
class KDoubleNumInput;

namespace Digikam
{
class ImagePannelWidget;
}

namespace DigikamUnsharpMaskImagesPlugin
{
class UnsharpMask;

class ImageEffect_Unsharp : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Unsharp(QWidget* parent);
    ~ImageEffect_Unsharp();

protected:
    
    void closeEvent(QCloseEvent *e);
       
private:
    
    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };
    
    int              m_currentRenderingMode;

    QWidget         *m_parent;
        
    QTimer          *m_timer;
    
    QPushButton     *m_helpButton;
    
    KDoubleNumInput *m_radiusInput;
    KDoubleNumInput *m_amountInput;
    
    KIntNumInput    *m_thresholdInput;
    
    UnsharpMask     *m_unsharpFilter;
    
    Digikam::ImagePannelWidget *m_imagePreviewWidget;

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

}  // NameSpace DigikamUnsharpMaskImagesPlugin

#endif /* IMAGEEFFECT_UNSHARP_H */
