/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-04-21
 * Description : slide show tool
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

// Qt includes

#include <qtimer.h>

// KDE includes

#include <kaction.h>
#include <kconfig.h>

// Local includes.

#include "ddebug.h"
#include "slideshow.h"

namespace Digikam
{

class SlideShowPriv
{
public:

    SlideShowPriv()
    {
        timer            = 0;
        first            = 0;
        next             = 0;
        delay            = 5;
        loop             = false;
        startWithCurrent = false;
    }

    bool     loop;
    bool     startWithCurrent;

    int      delay;

    QTimer  *timer;
    
    KAction *first;
    KAction *next;
};

SlideShow::SlideShow(KAction* first, KAction* next)
{
    d = new SlideShowPriv;
    d->first = first;
    d->next  = next;
    d->timer = new QTimer(this);
        
    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()) );
}

SlideShow::~SlideShow()
{
    stop();
    delete d;
}

void SlideShow::setLoop(bool value)
{
    d->loop = value;
}

void SlideShow::setStartWithCurrent(bool value)
{
    d->startWithCurrent = value;
}

void SlideShow::setDelay(int delay)
{
    d->delay = delay;
    
    if (d->timer->isActive()) 
    {
        d->timer->changeInterval(delay*1000);
    }
}

void SlideShow::start()
{
    if (!d->first->isEnabled()
        && !d->next->isEnabled()) 
    {
        emit finished();
        return;
    }

    if (d->first->isEnabled() && !d->startWithCurrent) 
    {
        d->first->activate();
    }

    d->timer->start(d->delay*1000);
}

void SlideShow::stop()
{
    d->timer->stop();
}

void SlideShow::slotTimeout()
{
    if (!d->next->isEnabled()) 
    {
        if (d->loop) 
        {
            d->first->activate();
            return;
        }
        
        d->timer->stop();
        emit finished();
        return;
    }

    d->next->activate();
}

bool SlideShow::startWithCurrent() const 
{
    return d->startWithCurrent; 
}

bool SlideShow::loop() const 
{
    return d->loop;
}

int SlideShow::delay() const 
{
    return d->delay;            
}

}   // namespace Digikam

#include "slideshow.moc"
