/* ============================================================
 * File  : imageeffect_refocus.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-29
 * Description : a digiKam image editor plugin to refocus 
 *               an image.
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

#ifndef IMAGEEFFECT_REFOCUS_H
#define IMAGEEFFECT_REFOCUS_H

// Qt include.

#include <qimage.h>

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QTimer;

class KIntNumInput;
class KDoubleNumInput;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamRefocusImagesPlugin
{

class ImageEffect_Refocus : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_Refocus(QWidget* parent);
    ~ImageEffect_Refocus();

protected:

    void closeEvent(QCloseEvent *e);
    
private:

    bool             m_cancel;
    bool             m_dirty;

    QWidget         *m_parent;
    
    QImage           m_img;
    
    QTimer          *m_timer;
    
    QPushButton     *m_helpButton;
    
    KIntNumInput    *m_matrixSize;
    
    KDoubleNumInput *m_radius;
    KDoubleNumInput *m_gauss;
    KDoubleNumInput *m_correlation;
    KDoubleNumInput *m_noise;
    
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
private:
    
    void abortPreview(void);

    void refocus(uint* data, int width, int height, int matrixSize, 
                 double radius, double gauss, double correlation, double noise);
                 
    void convolve_image(const uint *orgData, uint *destData, int width, int height, const double *const mat, int mat_size);
                         
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotUser2();
    void slotUser3();
    void slotTimer();   
};

}  // NameSpace DigikamRefocusImagesPlugin

#endif /* IMAGEEFFECT_REFOCUS_H */
