/* ============================================================
 * File  : threadedfilter.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : threaded image filter class.
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
 
 
#ifndef THREADEDFILTER_H
#define THREADEDFILTER_H

// Qt includes.

#include <qthread.h>
#include <qimage.h>
#include <qstring.h>

// KDE includes.

#include <kapplication.h>

class QObject;

namespace Digikam
{

class ThreadedFilter : public QThread
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
    
    ThreadedFilter(QImage *orgImage, QObject *parent=0);
    
    ~ThreadedFilter(){ stopComputation(); };
    
    QImage getTargetImage(void) { return m_destImage; };
    
    void startComputation(void);
    void stopComputation(void);
    
protected:

    // Copy of original Image data.
    QImage    m_orgImage;

    // Output image data.
    QImage    m_destImage;
    
    // Filter name.
    QString   m_name;
    
    // Used to stop compution loop.
    bool      m_cancel;   

    // To post event from thread to parent.    
    QObject  *m_parent;
    EventData m_eventData;
    
protected:

    // List of threaded operations.
    virtual void run(){ startComputation(); };
    
    // Main image filter method.
    virtual void filterImage(void){};
    
    // Clean up filter data if necessary. Call by stopComputation() method.
    virtual void cleanupFilter(void){};
    
};    

}  // NameSpace Digikam

#endif /* THREADEDFILTER_H */
