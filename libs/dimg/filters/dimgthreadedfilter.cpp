/* ============================================================
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

// Qt includes.

#include <qobject.h>
#include <qevent.h>
#include <qdeepcopy.h>

// Local includes.

#include "ddebug.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

DImgThreadedFilter::DImgThreadedFilter(DImg *orgImage, QObject *parent, 
                                       const QString& name)
                  : QThread()
{
    // remove meta data
    m_orgImage      = orgImage->copyImageData();
    m_parent        = parent;
    m_cancel        = false;

    // See B.K.O #133026: make a deep copy of Qstring to prevent crash 
    // on Hyperthreading computer.
    m_name          = QDeepCopy<QString>(name);

    m_master        = 0;
    m_slave         = 0;
    m_progressBegin = 0;
    m_progressSpan  = 100;
}

DImgThreadedFilter::DImgThreadedFilter(DImgThreadedFilter *master, const DImg &orgImage, 
                                       const DImg &destImage, int progressBegin, int progressEnd, 
                                       const QString& name)
{
    m_orgImage      = orgImage;
    m_destImage     = destImage;
    m_parent        = 0;
    m_cancel        = false;

    // See B.K.O #133026: make a deep copy of Qstring to prevent crash 
    // on Hyperthreading computer.
    m_name          = QDeepCopy<QString>(name);

    m_master        = master;
    m_slave         = 0;
    m_progressBegin = progressBegin;
    m_progressSpan  = progressEnd - progressBegin;

    m_master->setSlave(this);
}

DImgThreadedFilter::~DImgThreadedFilter()
{
    stopComputation();
    if (m_master)
        m_master->setSlave(0);
}

void DImgThreadedFilter::initFilter(void)
{
    m_destImage.reset();         
    m_destImage = DImg(m_orgImage.width(), m_orgImage.height(),
                       m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
    
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
          postProgress(0, false, false);
          DDebug() << m_name << "::No valid image data !!! ..." << endl;
       }
    }
}

void DImgThreadedFilter::stopComputation(void)
{
    m_cancel = true;
    if (m_slave)
    {
        m_slave->m_cancel = true;
        // do not wait on slave, it is not running in its own separate thread!
        //m_slave->cleanupFilter();
    }
    wait();
    cleanupFilter();
}

void DImgThreadedFilter::postProgress(int progress, bool starting, bool success)
{
    if (m_master)
    {
        progress = modulateProgress(progress);
        m_master->postProgress(progress, starting, success);
    }
    else if (m_parent)
    {
       EventData *eventData = new EventData();
       eventData->progress  = progress;
       eventData->starting  = starting;
       eventData->success   = success;
       QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, eventData));
    }
}

void DImgThreadedFilter::startComputation()
{
    // See B.K.O #133026: do not use kdDebug() statements in threaded implementation
    // to prevent crash under Hyperthreaded CPU.

    if (m_parent)
       postProgress(0, true, false);

    filterImage();
    
    if (!m_cancel)
    {
       if (m_parent)
          postProgress(0, false, true);
    }
    else
    {
       if (m_parent)
          postProgress(0, false, false);
    }
}

void DImgThreadedFilter::setSlave(DImgThreadedFilter *slave)
{
    m_slave = slave;
}

int DImgThreadedFilter::modulateProgress(int progress)
{
    return m_progressBegin + (int)((double)progress * (double)m_progressSpan / 100.0);
}

}  // NameSpace Digikam
