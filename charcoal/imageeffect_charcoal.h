/* ============================================================
 * File  : imageeffect_charcoal.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-26
 * Description : a digikam image editor plugin for to
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
class QSlider;
class QSpinBox;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamCharcoalImagesPlugin
{

class ImageEffect_Charcoal : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Charcoal(QWidget* parent);
    ~ImageEffect_Charcoal();

protected:

    void closeEvent(QCloseEvent *e);
    
private:

    bool         m_cancel;
    
    QWidget     *m_parent;
    
    QPushButton *m_helpButton;
    
    QSlider     *m_pencilSlider;
    QSlider     *m_smoothSlider;
    
    QSpinBox    *m_pencilInput;
    QSpinBox    *m_smoothInput;
    
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private:

    QImage charcoal(QImage &src, double pencil, double smooth);
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
        
};

}  // NameSpace DigikamCharcoalImagesPlugin

#endif /* IMAGEEFFECT_CHARCOAL_H */
