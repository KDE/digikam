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

// Local include.

#include "CImg.h"

class QPushButton;
class QLabel;
class QCheckBox;
class QTimer;
class QComboBox;

class KDoubleNumInput;
class KIntNumInput;
class KProgress;

namespace Digikam
{
class ImagePreviewWidget;
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

    enum RestorationType
    {
    FilteringMode=0,
    InPaintingMode
    };

    bool             m_cancel;
    bool             m_dirty;
    
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
    
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;
    
    void processRestoration(uint* data, int width, int height);
    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotTimer();
    void slotRestorationTypeChanged(int type);
    
private:  // CImg filter interface.

    bool process();
    
    // Compute smoothed structure tensor field G
    void compute_smoothed_tensor();

    // Compute normalized tensor field sqrt(T) in G
    void compute_normalized_tensor();

    // Compute LIC's along different angle projections a_\alpha
    void compute_LIC(int &counter);
    void compute_LIC_back_forward(int x, int y);
    void compute_W(float cost, float sint);

    // Average all the LIC's
    void compute_average_LIC();

    // Prepare datas
    bool prepare();
    bool prepare_restore();
    bool prepare_inpaint();
    bool prepare_resize();
    bool prepare_visuflow();

    // Check arguments
    bool check_args();

    // Clean up memory (CImg datas) to save memory
    void cleanup();

    void get_geom(const char *geom, int &geom_w, int &geom_h);
    
private:  // CImg filter data.
    
    unsigned int nb_iter; // Number of smoothing iterations
    float dt;             // Time step
    float dlength;        // Integration step
    float dtheta;         // Angular step (in degrees)
    float sigma;          // Structure tensor blurring
    float power1;         // Diffusion limiter along isophote
    float power2;         // Diffusion limiter along gradient
    float gauss_prec;     // Precision of the gaussian function
    bool  onormalize;     // Output image normalization (in [0,255])
    bool  linear;         // Use linear interpolation for integration

    // internal use
    bool restore;
    bool inpaint;
    bool resize;
    const char* visuflow;
    cimg_library::CImg<> dest, sum, W;
    cimg_library::CImg<> img, img0, flow,G;
    cimg_library::CImgl<> eigen;
    cimg_library::CImg<unsigned char> mask;
};

}  // NameSpace DigikamRestorationImagesPlugin

#endif /* IMAGEEFFECT_RESTORATION_H */
