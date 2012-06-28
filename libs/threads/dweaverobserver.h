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

#ifndef DWEAVEROBSERVER_H
#define DWEAVEROBSERVER_H

// KDE includes

#include <threadweaver/Job.h>
#include <threadweaver/WeaverObserver.h>
#include <threadweaver/State.h>
#include <threadweaver/Thread.h>

// Local includes

#include "digikam_export.h"

using namespace ThreadWeaver;

namespace Digikam
{

/** DWeaverObserver is a simple wrapper to plug on the ActionThread class to
    prints debug messages when signals are received.
*/
class DIGIKAM_EXPORT DWeaverObserver : public WeaverObserver
{
    Q_OBJECT

public:

    DWeaverObserver(QObject* const parent=0);
    ~DWeaverObserver();

protected Q_SLOTS:

    void slotWeaverStateChanged(ThreadWeaver::State*);
    void slotThreadStarted(ThreadWeaver::Thread*);
    void slotThreadBusy(ThreadWeaver::Thread*, ThreadWeaver::Job*);
    void slotThreadSuspended(ThreadWeaver::Thread*);
    void slotThreadExited(ThreadWeaver::Thread*);
};

}  // namespace Digikam

#endif // DWEAVEROBSERVER_H
