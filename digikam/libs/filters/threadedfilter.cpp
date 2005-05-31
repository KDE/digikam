/* ============================================================
 * File  : threadedfilter.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : threaded image filter class.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Unsharp Mask algorithm come from plug-ins/common/unsharp.c 
 * Gimp 2.0 source file and copyrighted 
 * 1999 by Winston Chang (winstonc at cs.wisc.edu)
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
#include <qdatetime.h> 
#include <qevent.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "threadedfilter.h"

namespace Digikam
{

ThreadedFilter::ThreadedFilter(QImage *orgImage, QObject *parent)
              : QThread()
{ 
    m_orgImage = orgImage->copy();
    m_parent   = parent;
    m_cancel   = false;
    m_name     = QString::null;
        
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
          kdDebug() << m_name << "::No valid image data !!! ..." << endl;
          }
       }
}

void ThreadedFilter::stopComputation(void)
{
    m_cancel = true;
    wait();
    cleanupFilter();
}

void ThreadedFilter::startComputation()
{
    QDateTime startDate = QDateTime::currentDateTime();
    
    if (m_parent)
       {
       m_eventData.starting = true;
       m_eventData.success  = false;
       m_eventData.progress = 0;
       QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, &m_eventData));
       }

    filterImage();
    
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
          
       kdDebug() << m_name << "::End of computation !!! ... ( " << startDate.secsTo(endDate) << " s )" << endl;
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
          
       kdDebug() << m_name << "::Computation aborted... ( " << startDate.secsTo(endDate) << " s )" << endl;
       }
}

}  // NameSpace Digikam
