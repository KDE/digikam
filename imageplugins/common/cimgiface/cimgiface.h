/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-28
 * Description : CImg threaded interface.
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
 
#ifndef CIMGIFACE_H
#define CIMGIFACE_H

// QT includes.

#include <qimage.h>

// Digikam includes.

#include <digikamheaders.h>

// Local include.

#include "CImg.h"

class QObject;

namespace DigikamImagePlugins
{

class CimgIface : public Digikam::DImgThreadedFilter
{

public:

    CimgIface(Digikam::DImg *orgImage,
              uint blurIt, double timeStep, double integralStep,
              double angularStep, double blur, double detail,
              double gradient, double gaussian, bool normalize, bool linearInterpolation, 
              bool restoreMode=true, bool inpaintMode=false, bool resizeMode=false, 
              char* visuflowMode=NULL, int newWidth=0, int newHeight=0,
              QImage *inPaintingMask=0, QObject *parent=0);
    
    ~CimgIface();
    
private:

    // Inpainting temp mask file path.
    QString       m_tmpMaskFile;
    
    // Inpainting temp mask data.
    QImage        m_inPaintingMask;

private:  // CImg filter data.
    
    // CImg filter settings.
    unsigned int m_nb_iter;    // Number of smoothing iterations
    float        m_dt;         // Time step
    float        m_dlength;    // Integration step
    float        m_dtheta;     // Angular step (in degrees)
    float        m_sigma;      // Structure tensor blurring
    float        m_power1;     // Diffusion limiter along isophote
    float        m_power2;     // Diffusion limiter along gradient
    float        m_gauss_prec; // Precision of the gaussian function
    bool         m_onormalize; // Output image normalization (in [0,255])
    bool         m_linear;     // Use linear interpolation for integration

    // CImg computation modes.
    bool         m_restore;
    bool         m_inpaint;
    bool         m_resize;
    const char*  m_visuflow;
    
    // Internal use.
    cimg_library::CImg<> dest, sum, W;
    cimg_library::CImg<> img, img0, flow,G;
    cimg_library::CImgl<> eigen;
    cimg_library::CImg<unsigned char> mask;

private:  // CImg filter interface.

    virtual void initFilter(void);
    virtual void filterImage(void);
    virtual void cleanupFilter(void);
    
    bool process();
    
    // Compute smoothed structure tensor field G
    inline void compute_smoothed_tensor();

    // Compute normalized tensor field sqrt(T) in G
    inline void compute_normalized_tensor();

    // Compute LIC's along different angle projections a_\alpha
    inline void compute_LIC(int &counter);
    inline void compute_LIC_back_forward(int x, int y);
    inline void compute_W(float cost, float sint);

    // Average all the LIC's
    inline void compute_average_LIC();

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
    
};    

}  // NameSpace DigikamImagePlugins

#endif /* CIMGIFACE_H */
