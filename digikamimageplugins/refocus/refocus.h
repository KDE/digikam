/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Refocus threaded image filter.
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
 
 
#ifndef REFOCUS_H
#define REFOCUS_H

// Qt includes.

#include <qthread.h>
#include <qimage.h>
#include <qstring.h>

class QObject;

namespace DigikamRefocusImagesPlugin
{

class Refocus : public QThread
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
    
    Refocus(QImage *orgImage, int matrixSize, double radius, 
            double gauss, double correlation, double noise, QObject *parent=0);
    
    ~Refocus();
    
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

private:  // Refocus filter data.

    int m_matrixSize;
    
    double m_radius;
    double m_gauss;
    double m_correlation;
    double m_noise;
    
private:  // Refocus filter methods.

    void refocusImage(const uint* data, int width, int height, int matrixSize, 
                      double radius, double gauss, double correlation, double noise);
                               
    void convolveImage(const uint *orgData, uint *destData, int width, int height, 
                       const double *const mat, int mat_size);
    
};    

}  // NameSpace DigikamRefocusImagesPlugin

#endif /* REFOCUS_H */
