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

#ifndef DIGIKAM_PARALLEL_PIPES_H
#define DIGIKAM_PARALLEL_PIPES_H

// Local includes

#include "facepipeline_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN ParallelPipes : public QObject
{
    Q_OBJECT

public:

    explicit ParallelPipes();
    ~ParallelPipes();

    void schedule();
    void deactivate(WorkerObject::DeactivatingMode mode = WorkerObject::FlushSignals);
    void wait();

    void add(WorkerObject* const worker);
    void setPriority(QThread::Priority priority);

public:

    QList<WorkerObject*> m_workers;

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    QList<QMetaMethod> m_methods;
    int                m_currentIndex;
};

} // namespace Digikam

#endif // DIGIKAM_PARALLEL_PIPES_H
