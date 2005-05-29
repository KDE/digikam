/* ============================================================
 * File  : despeckle.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Despeckle threaded image filter.
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
  
#ifndef DESPECKLE_H
#define DESPECKLE_H

// Qt includes.

#include <qthread.h>
#include <qimage.h>

class QObject;

namespace DigikamNoiseReductionImagesPlugin
{

class Despeckle : public QThread
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
    
    Despeckle(QImage *orgImage, int radius, int black_level, int white_level, 
              bool adaptativeFilter, bool recursiveFilter, QObject *parent=0);
    
    ~Despeckle();
    
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

private:  // Despeckle filter data.

    int  m_radius;
    int  m_black_level;
    int  m_white_level;
    bool m_adaptativeFilter;
    bool m_recursiveFilter; 
    
private:  // Despeckle filter methods.

    void Despeckle::despeckleImage(uint* data, int w, int h, int despeckle_radius, 
                                   int black_level, int white_level, 
                                   bool adaptativeFilter, bool recursiveFilter);

};    

}  // NameSpace DigikamNoiseReductionImagesPlugin

#endif /* DESPECKLE_H */
