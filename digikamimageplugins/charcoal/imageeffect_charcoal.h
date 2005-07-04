/* ============================================================
 * File  : imageeffect_charcoal.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin for 
 *               simulate charcoal drawing.
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

#ifndef IMAGEEFFECT_CHARCOAL_H
#define IMAGEEFFECT_CHARCOAL_H

// Qt include.

#include <qimage.h>

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QTimer;

class KIntNumInput;

namespace Digikam
{
class ImagePannelWidget;
}

namespace DigikamCharcoalImagesPlugin
{
class Charcoal;

class ImageEffect_Charcoal : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Charcoal(QWidget* parent);
    ~ImageEffect_Charcoal();

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
    
    KIntNumInput *m_pencilInput;
    KIntNumInput *m_smoothInput;
    
    Charcoal     *m_charcoalFilter;
    
    Digikam::ImagePannelWidget *m_imagePreviewWidget;

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
    void slotFocusChanged(void);                 
};

}  // NameSpace DigikamCharcoalImagesPlugin

#endif /* IMAGEEFFECT_CHARCOAL_H */
