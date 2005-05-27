/* ============================================================
 * File  : refocus.cpp
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
  
// C++ includes. 
 
#include <cmath>

// Qt includes.

#include <qobject.h>
#include <qdatetime.h> 
#include <qevent.h>
#include <qstring.h>

// KDE includes.

#include <kapplication.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "matrix.h"
#include "refocus.h"

namespace DigikamRefocusImagesPlugin
{

Refocus::Refocus(QImage *orgImage, int matrixSize, double radius, 
                 double gauss, double correlation, double noise, QObject *parent)
       : QThread()
{ 
    m_orgImage    = orgImage->copy();
    m_parent      = parent;
    m_cancel      = false;
        
    // Get the config data

    m_matrixSize  = matrixSize;
    m_radius      = radius;
    m_gauss       = gauss;
    m_correlation = correlation;
    m_noise       = noise;
    
    m_destImage.create(m_orgImage.width(), m_orgImage.height(), 32);
        
    if (m_orgImage.width() && m_orgImage.height())
       {
       if (m_parent)
          start();             // m_parent is valide, start thread ==> run()
       else
          startComputation();  // no parent : no using thread.
       }
    else  // No image data 
       {
       if (m_parent)           // If parent then send event about a problem.
          {
          m_eventData.starting = false;
          m_eventData.success  = false;
          QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
          }
       }
}

Refocus::~Refocus()
{ 
    stopComputation();
}

void Refocus::stopComputation(void)
{
    m_cancel = true;
    wait();
}

// List of threaded operations.

void Refocus::run()
{
    startComputation();
}

void Refocus::startComputation()
{
    QDateTime startDate = QDateTime::currentDateTime();
    
    if (m_parent)
       {
       m_eventData.starting = true;
       m_eventData.success  = false;
       m_eventData.progress = 0;
       QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
       }

    refocusImage((uint*)m_orgImage.bits(), m_orgImage.width(), m_orgImage.height(), 
                 m_matrixSize, m_radius, m_gauss, m_correlation, m_noise);
    
    QDateTime endDate = QDateTime::currentDateTime();    
    
    if (!m_cancel)
       {
       if (m_parent)
          {
          m_eventData.starting = false;
          m_eventData.success  = true;
          m_eventData.progress = 0;
          QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
          }
          
       kdDebug() << "Refocus::End of computation !!! ... ( " << startDate.secsTo(endDate) << " s )" << endl;
       }
    else
       {
       if (m_parent)
          {
          m_eventData.starting = false;
          m_eventData.success  = false;
          m_eventData.progress = 0;
          QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
          }
          
       kdDebug() << "Refocus::Computation aborted... ( " << startDate.secsTo(endDate) << " s )" << endl;
       }
}

void Refocus::refocusImage(const uint* data, int width, int height, int matrixSize, 
                           double radius, double gauss, double correlation, double noise)
{
    CMat *matrix=0;
    
    // Compute matrix
    kdDebug() << "Refocus::Compute matrix..." << endl;

    CMat circle, gaussian, convolution;
    
    RefocusMatrix::make_gaussian_convolution (gauss, &gaussian, matrixSize);
    RefocusMatrix::make_circle_convolution (radius, &circle, matrixSize);
    RefocusMatrix::init_c_mat (&convolution, matrixSize);
    RefocusMatrix::convolve_star_mat (&convolution, &gaussian, &circle);

    matrix = RefocusMatrix::compute_g_matrix (&convolution, matrixSize, correlation, noise, 0.0, true);
    
    RefocusMatrix::finish_c_mat (&convolution);
    RefocusMatrix::finish_c_mat (&gaussian);
    RefocusMatrix::finish_c_mat (&circle);

    // Apply deconvolution kernel to image.
    kdDebug() << "Refocus::Apply Matrix to image..." << endl;
    convolveImage(data, (uint*)m_destImage.bits(), width, height, matrix->data, 2 * matrixSize + 1);
    
    // Clean up memory
    delete matrix;   
}

void Refocus::convolveImage(const uint *orgData, uint *destData, int width, int height, 
                            const double *const mat, int mat_size)
{
    Refocus::EventData d;
    
    double matrix[mat_size][mat_size];
    double valRed, valGreen, valBlue;
    int    x1, y1, x2, y2, index1, index2;
    
    // Big/Little Endian color manipulation compatibility.
    int red, green, blue;
    Digikam::ImageFilters::imageData imagedata;
    
    const int imageSize  = width*height;
    const int mat_offset = mat_size / 2;
    
    memcpy (&matrix, mat, mat_size* mat_size * sizeof(double));
    
    for (y1 = 0; !m_cancel && (y1 < height); y1++)
        {
        for (x1 = 0; !m_cancel && (x1 < width); x1++)
            {
            valRed = valGreen = valBlue = 0.0;
                
            for (y2 = 0; !m_cancel && (y2 < mat_size); y2++)
                {
                for (x2 = 0; !m_cancel && (x2 < mat_size); x2++)
                    {
                    index1 = width * (y1 + y2 - mat_offset) + 
                             x1 + x2 - mat_offset;
                        
                    if ( index1 >= 0 && index1 < imageSize )
                       {
                       imagedata.raw = orgData[index1];
                       red           = (int)imagedata.channel.red;
                       green         = (int)imagedata.channel.green;
                       blue          = (int)imagedata.channel.blue;
                       valRed   += matrix[y2][x2] * red;
                       valGreen += matrix[y2][x2] * green;
                       valBlue  += matrix[y2][x2] * blue;
                       }
                    }
                }
            
            index2 = y1 * width + x1;
                          
            if (index2 >= 0 && index2 < imageSize)
                {
                // To get Alpha channel value from original (unchanged)
                imagedata.raw = orgData[index2];
                
                // Overwrite RGB values to destination.
                imagedata.channel.red   = (uchar) CLAMP (valRed, 0, 255);
                imagedata.channel.green = (uchar) CLAMP (valGreen, 0, 255);
                imagedata.channel.blue  = (uchar) CLAMP (valBlue, 0, 255);
                destData[index2]        = imagedata.raw;
                }
            }
        
        // Update the progress bar in dialog.
        m_eventData.starting = true;
        m_eventData.success  = false;
        m_eventData.progress = (int)((int) (((double)y1 * 100.0) / height));
        QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
        }
}

}  // NameSpace DigikamRefocusImagesPlugin

