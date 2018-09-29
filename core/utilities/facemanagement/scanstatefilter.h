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

#ifndef DIGIKAM_SCAN_STATE_FILTER_H
#define DIGIKAM_SCAN_STATE_FILTER_H

// Qt includes

#include <QExplicitlySharedDataPointer>
#include <QMetaMethod>
#include <QMutex>
#include <QSharedData>
#include <QWaitCondition>

// Local includes

#include "facepipeline_p.h"
#include "facedetector.h"
#include "faceutils.h"
#include "previewloadthread.h"
#include "thumbnailloadthread.h"
#include "workerobject.h"

namespace Digikam
{

class Q_DECL_HIDDEN ScanStateFilter : public DynamicThread
{
    Q_OBJECT

public:

    ScanStateFilter(FacePipeline::FilterMode mode, FacePipeline::Private* const d);

    void process(const QList<ImageInfo>& infos);
    void process(const ImageInfo& info);

    FacePipelineExtendedPackage::Ptr filter(const ImageInfo& info);

public:

    FacePipeline::Private* const     d;
    FacePipeline::FilterMode         mode;
    FacePipelineFaceTagsIface::Roles tasks;

protected Q_SLOTS:

    void dispatch();

Q_SIGNALS:

    void infosToDispatch();

protected:

    virtual void run();

protected:

    QList<ImageInfo>                        toFilter;
    QList<FacePipelineExtendedPackage::Ptr> toSend;
    QList<ImageInfo>                        toBeSkipped;
};

} // namespace Digikam

#endif // DIGIKAM_SCAN_STATE_FILTER_H
