/* ============================================================
 * File  : unsharp.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Unsharp Mask threaded image filter.
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
 
 
#ifndef UNSHARPMASK_H
#define UNSHARPMASK_H

// Qt includes.

#include <qthread.h>
#include <qimage.h>

class QObject;

namespace DigikamUnsharpMaskImagesPlugin
{

class UnsharpMask : public QThread
{

public:

// Class used to post status of computation to parent.

class EventData
    {
    public:
    
    EventData() 
       {
       starting = false;
       success  = false; 
       }
    
    bool starting;    
    bool success;
    int  progress;
    };

public:
    
    UnsharpMask(QImage *orgImage, double radius, 
                double amount, int threshold, QObject *parent=0);
    
    ~UnsharpMask();
    
    void   startComputation(void);
    void   stopComputation(void);
    
    QImage getTargetImage(void) { return m_destImage; };
    
private:

    // Copy of original Image data.
    QImage    m_orgImage;

    // Output image data.
    QImage    m_destImage;
    
    // Used to stop compution loop.
    bool      m_cancel;   

    // To post event from thread to parent.    
    QObject  *m_parent;
    EventData m_eventData;
    
protected:

    virtual void run();

private:  // Unsharp Mask filter data.

    double m_radius;
    double m_amount;
    int    m_threshold;
    
private:  // Unsharp Mask filter methods.

    void unsharpImage(uint* data, int w, int h, double radius, 
                      double amount, int threshold);
                 
    inline void blur_line (double *ctable, double *cmatrix, int cmatrix_length,
                           uchar *cur_col, uchar *dest_col, int y, long bytes);  
                           
    int gen_convolve_matrix (double radius, double **cmatrix_p);    
    
    double* gen_lookup_table (double *cmatrix, int cmatrix_length);
    
};    

}  // NameSpace DigikamUnsharpMaskImagesPlugin

#endif /* UNSHARPMASK_H */
