/* ============================================================
 * File  : dimgthreadedfilter.h
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
 
#ifndef DIMGTHREADEDFILTER_H
#define DIMGTHREADEDFILTER_H

// Qt includes.

#include <qthread.h>
#include <qstring.h>

// KDE includes.

#include <kapplication.h>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

class QObject;

namespace Digikam
{

class DIGIKAM_EXPORT DImgThreadedFilter : public QThread
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
    
    DImgThreadedFilter(DImg *orgImage, QObject *parent=0, QString name=QString::null);
    
    ~DImgThreadedFilter(){ stopComputation(); };
    
    DImg getTargetImage(void) { return m_destImage; };
    
    void startComputation(void);
    void stopComputation(void);
    
protected:

    // Copy of original Image data.
    DImg      m_orgImage;

    // Output image data.
    DImg      m_destImage;
    
    // Filter name.
    QString   m_name;
    
    // Used to stop compution loop.
    bool      m_cancel;   

    // To post event from thread to parent.    
    QObject  *m_parent;
    
protected:

    // Start filter operation before threaded method. Must be calls by your constructor.
    virtual void initFilter(void);
        
    // List of threaded operations by filter.
    virtual void run(){ startComputation(); };
    
    // Main image filter method.
    virtual void filterImage(void){};
    
    // Clean up filter data if necessary. Call by stopComputation() method.
    virtual void cleanupFilter(void){};
    
    // Post Event to parent about progress. Warning: you need to delete 
    // 'EventData' instance to 'customEvent' parent implementation.
    void postProgress(int progress=0, bool starting=true, bool success=false);
    
};    

}  // NameSpace Digikam

#endif /* DIMGTHREADEDFILTER_H */
