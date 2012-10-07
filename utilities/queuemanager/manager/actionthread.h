/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Pankaj Kumar <me at panks dot me>
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

#ifndef ACTIONTHREAD_H
#define ACTIONTHREAD_H

// Qt includes

#include <QThread>

// KDE includes

#include <kurl.h>
#include <threadweaver/Job.h>

// Libkdcraw includes

#include <libkdcraw/ractionthreadbase.h>

// Local includes

#include "batchtool.h"
#include "actions.h"
#include "drawdecoding.h"

using namespace KDcrawIface;

namespace Digikam
{

class ActionThread : public RActionThreadBase
{
    Q_OBJECT

public:

    explicit ActionThread(QObject* const parent);
    ~ActionThread();

    void setWorkingUrl(const KUrl& url);
    void setResetExifOrientationAllowed(bool set);
    void setRawDecodingSettings(const DRawDecoding& settings);

    void processFile(const AssignedBatchTools& item);

    void cancel();

Q_SIGNALS:

    void starting(const Digikam::ActionData& ad);
    void finished(const Digikam::ActionData& ad);

public:

    class Private;

private:

    Private* const d;
};

// ---------------------------------------------------------------------------------------------------------

class Task : public Job
{
    Q_OBJECT

public:

    Task(QObject* const parent, const AssignedBatchTools& item, ActionThread::Private* const d);
    ~Task();

Q_SIGNALS:

    void signalStarting(const Digikam::ActionData& ad);
    void signalFinished(const Digikam::ActionData& ad);

protected:

    void run();

private:

    AssignedBatchTools     m_item;
    ActionThread::Private* m_d;
};

}  // namespace Digikam

#endif /* ACTIONTHREAD_H */

