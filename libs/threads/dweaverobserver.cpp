/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : prints debugging messages about the thread activity in action thread class
 *
 * Copyright (C) 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dweaverobserver.moc"

// KDE includes

#include <kdebug.h>

namespace Digikam
{

DWeaverObserver::DWeaverObserver(QObject* const parent)
    : WeaverObserver(parent)
{
    connect(this, SIGNAL(weaverStateChanged(ThreadWeaver::State*)),
            this, SLOT(slotWeaverStateChanged(ThreadWeaver::State*)));

    connect(this, SIGNAL(threadStarted(ThreadWeaver::Thread*)),
            this, SLOT(slotThreadStarted(ThreadWeaver::Thread*)));

    connect(this, SIGNAL(threadBusy(ThreadWeaver::Thread*, ThreadWeaver::Job*)),
            this, SLOT(slotThreadBusy(ThreadWeaver::Thread*, ThreadWeaver::Job*)));

    connect(this, SIGNAL(threadSuspended(ThreadWeaver::Thread*)),
            this, SLOT(slotThreadSuspended(ThreadWeaver::Thread*)));

    connect(this, SIGNAL(threadExited(ThreadWeaver::Thread*)),
            this, SLOT(slotThreadExited(ThreadWeaver::Thread*)));
}

DWeaverObserver::~DWeaverObserver()
{
}

void DWeaverObserver::slotWeaverStateChanged(State* state)
{
    kDebug() << "DWeaverObserver: thread state changed to " << state->stateName();
}

void DWeaverObserver::slotThreadStarted(Thread* th)
{
    kDebug() << "DWeaverObserver: thread " << th->id()  <<" started";
}

void DWeaverObserver::slotThreadBusy(Thread* th, Job*)
{
    kDebug() << "DWeaverObserver: thread " << th->id()  << " busy";
}

void DWeaverObserver::slotThreadSuspended(Thread* th )
{
    kDebug() << "DWeaverObserver: thread " << th->id()  << " suspended";
}

void DWeaverObserver::slotThreadExited(Thread* th)
{
    kDebug() << "DWeaverObserver: thread " << th->id()  << " exited";
}

}  // namespace Digikam
