/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Integrated, multithread face detection / recognition
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "parallelpipes.h"

// Qt includes

#include <QMetaObject>
#include <QMutexLocker>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "loadingdescription.h"
#include "metaenginesettings.h"
#include "tagscache.h"
#include "threadmanager.h"
#include "facebenchmarkers.h"
#include "faceworkers.h"
#include "faceimageretriever.h"

namespace Digikam
{

ParallelPipes::ParallelPipes()
    : m_currentIndex(0)
{
}

ParallelPipes::~ParallelPipes()
{
    foreach (WorkerObject* const object, m_workers)
    {
        delete object;
    }
}

void ParallelPipes::schedule()
{
    foreach (WorkerObject* const object, m_workers)
    {
        object->schedule();
    }
}

void ParallelPipes::deactivate(WorkerObject::DeactivatingMode mode)
{
    foreach (WorkerObject* const object, m_workers)
    {
        object->deactivate(mode);
    }
}

void ParallelPipes::wait()
{
    foreach (WorkerObject* const object, m_workers)
    {
        object->wait();
    }
}

void ParallelPipes::setPriority(QThread::Priority priority)
{
    foreach (WorkerObject* const object, m_workers)
    {
        object->setPriority(priority);
    }
}

void ParallelPipes::add(WorkerObject* const worker)
{
    QByteArray normalizedSignature = QMetaObject::normalizedSignature("process(FacePipelineExtendedPackage::Ptr)");
    int methodIndex                = worker->metaObject()->indexOfMethod(normalizedSignature.constData());

    if (methodIndex == -1)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Object" << worker << "does not have a slot"
                                     << normalizedSignature << " - cannot use for processing.";
        return;
    }

    m_workers << worker;
    m_methods << worker->metaObject()->method(methodIndex);

    // collect the worker's signals and bundle them to our single signal, which is further connected
    connect(worker, SIGNAL(processed(FacePipelineExtendedPackage::Ptr)),
            this, SIGNAL(processed(FacePipelineExtendedPackage::Ptr)));
}

void ParallelPipes::process(FacePipelineExtendedPackage::Ptr package)
{
    // Here, we send the package to one of the workers, in turn
    m_methods.at(m_currentIndex).invoke(m_workers.at(m_currentIndex), Qt::QueuedConnection,
                                        Q_ARG(FacePipelineExtendedPackage::Ptr, package));

    if (++m_currentIndex == m_workers.size())
    {
        m_currentIndex = 0;
    }
}

} // namespace Digikam
