/* ============================================================
 * File  : dimgthreadedfilter.cpp
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
  
// Qt includes.

#include <qobject.h>
#include <qdatetime.h> 
#include <qevent.h>

// KDE includes.

#include <kdebug.h>

// Local includes.

#include "dimgthreadedfilter.h"

namespace Digikam
{

DImgThreadedFilter::DImgThreadedFilter(DImg *orgImage, QObject *parent, QString name)
                  : QThread()
{ 
    m_orgImage = DImg(*orgImage);
    m_parent   = parent;
    m_cancel   = false;
    m_name     = name;
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
          kdDebug() << m_name << "::No valid image data !!! ..." << endl;
       }
    }
}

void DImgThreadedFilter::stopComputation(void)
{
    m_cancel = true;
    wait();
    cleanupFilter();
}

void DImgThreadedFilter::postProgress(int progress, bool starting, bool success)
{
    if (m_parent)
    {    
       EventData *eventData = new EventData();
       eventData->progress = progress;
       eventData->starting = starting;
       eventData->success  = success;
       QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, eventData));
    }
}

void DImgThreadedFilter::startComputation()
{
    QDateTime startDate = QDateTime::currentDateTime();
    
    if (m_parent)
       postProgress(0, true, false);

    filterImage();
    
    QDateTime endDate = QDateTime::currentDateTime();    
    
    if (!m_cancel)
    {
       if (m_parent)
          postProgress(0, false, true);
          
       kdDebug() << m_name << "::End of computation !!! ... ( " << startDate.secsTo(endDate) << " s )" << endl;
    }
    else
    {
       if (m_parent)
          postProgress(0, false, false);
          
       kdDebug() << m_name << "::Computation aborted... ( " << startDate.secsTo(endDate) << " s )" << endl;
    }
}

}  // NameSpace Digikam
