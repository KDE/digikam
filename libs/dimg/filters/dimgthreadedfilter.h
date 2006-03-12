/* ============================================================
 * File  : dimgthreadedfilter.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-25
 * Description : threaded image filter class.
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
    
    ~DImgThreadedFilter();
    
    DImg getTargetImage(void) { return m_destImage; };
    
    void startComputation(void);
    void stopComputation(void);
    
    const QString &filterName() { return m_name; };
    
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
    
protected:

    // Support for chaining two filters as master and thread

    // The current slave. Any filter might want to use another filter while processing.
    DImgThreadedFilter *m_slave;
    // The master of this slave filter. Progress info will be routed to this one.
    DImgThreadedFilter *m_master;

    /*
      Constructor for slave mode:
      Constructs a new slave filter with the specified master.
      The filter will be executed in the current thread.
      orgImage and destImage will not be copied.
      progressBegin and progressEnd can indicate the progress span
      that the slave filter uses in the parent filter's progress.
      Any derived filter class that is publicly available to other filters
      should implement an additional constructor using this constructor.
    */
    DImgThreadedFilter(DImgThreadedFilter *master, const DImg &orgImage, const DImg &destImage,
                       int progressBegin=0, int progressEnd=100, QString name=QString::null);

    // inform the master that there is currently a slave. At destruction of the slave, call with slave=0.
    void setSlave(DImgThreadedFilter *slave);

    // The progress span that a slave filter uses in the parent filter's progress
    int m_progressBegin;
    int m_progressSpan;
    // This method modulates the progress value from the 0..100 span to the span of this slave.
    // Called by postProgress if master is not null.
    virtual int modulateProgress(int progress);

};

}  // NameSpace Digikam

#endif /* DIMGTHREADEDFILTER_H */
