/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-21
 * Description : slideshow for showfoto
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

// Qt includes

#include <qtimer.h>

// KDE includes

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>

// Local includes.

#include "slideshow.h"

namespace ShowFoto
{

SlideShow::SlideShow(KAction* first, KAction* next)
         : m_first(first), m_next(next), m_delay(5), 
           m_loop(false), m_startWithCurrent(false)
{
    m_timer = new QTimer(this);
    
    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()) );
}

void SlideShow::setLoop(bool value)
{
    m_loop = value;
}

void SlideShow::setStartWithCurrent(bool value)
{
    m_startWithCurrent = value;
}

void SlideShow::setDelay(int delay)
{
    m_delay = delay;
    
    if (m_timer->isActive()) 
    {
        m_timer->changeInterval(delay*1000);
    }
}

void SlideShow::start()
{
    if (!m_first->isEnabled()
        && !m_next->isEnabled()) 
    {
        emit finished();
        return;
    }
    if (m_first->isEnabled() && !m_startWithCurrent) 
    {
        m_first->activate();
    }
    m_timer->start(m_delay*1000);
}

void SlideShow::stop()
{
    m_timer->stop();
}

void SlideShow::slotTimeout()
{
    if (!m_next->isEnabled()) 
    {
        if (m_loop) 
        {
            m_first->activate();
            return;
        }
        
        m_timer->stop();
        emit finished();
        return;
    }

    m_next->activate();
}

}   // namespace ShowFoto

#include "slideshow.moc"
