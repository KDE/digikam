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

// Qt includes.

#include <qthread.h>

// Local include.

#include "CImg.h"

class QObject;

namespace DigikamImagePlugins
{

class CimgIface : public QThread
{

public:

class EventData
    {
    public:
    
    EventData() 
       {
       starting = false;
       success = false; 
       }
    
    bool starting;    
    bool success;
    int  progress;
    };

public:
    
    CimgIface(uint *data, uint width, uint height,
              uint blurIt, double timeStep, double integralStep,
              double angularStep, double blur, double detail,
              double gradient, double gaussian, bool normalize, bool normalize, 
              bool restoreMode=true, bool inpaintMode=false, bool resizeMode=false, 
              char* visuflowMode=NULL, QObject *parent=0);
    
    ~CimgIface();
    
    void startComputation(void);
    void stopComputation(void);

private:

    uint     *m_imageData;
    int       m_imageWidth;
    int       m_imageHeight;
    
    bool      m_cancel;   // Used to stop thread during calculations.
    
    QObject  *m_parent;

protected:

    virtual void run();
    
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
    
    unsigned int nb_iter;    // Number of smoothing iterations
    float        dt;         // Time step
    float        dlength;    // Integration step
    float        dtheta;     // Angular step (in degrees)
    float        sigma;      // Structure tensor blurring
    float        power1;     // Diffusion limiter along isophote
    float        power2;     // Diffusion limiter along gradient
    float        gauss_prec; // Precision of the gaussian function
    bool         onormalize; // Output image normalization (in [0,255])
    bool         linear;     // Use linear interpolation for integration

    // Internal use
    bool        restore;
    bool        inpaint;
    bool        resize;
    const char* visuflow;
    
    cimg_library::CImg<> dest, sum, W;
    cimg_library::CImg<> img, img0, flow,G;
    cimg_library::CImgl<> eigen;
    cimg_library::CImg<unsigned char> mask;
    
};    

}  // NameSpace Digikam

#endif /* CIMGIFACE_H */
