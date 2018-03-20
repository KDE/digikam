/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PANO_ACTION_THREAD_H
#define PANO_ACTION_THREAD_H

// Qt includes

#include <QThread>
#include <QUrl>

// KDE includes

#include <ThreadWeaver/Sequence>

// Local includes

#include "panoactions.h"
#include "ptotype.h"

namespace Digikam
{

struct PanoActionData;

class PanoActionThread : public QObject
{
    Q_OBJECT

public:

    explicit PanoActionThread(QObject* const parent);
    ~PanoActionThread();

public:

    void preProcessFiles(const QList<QUrl>& urlList,
                         PanoramaItemUrlsMap& preProcessedMap,
                         QUrl& baseUrl,
                         QUrl& cpFindPtoUrl,
                         QUrl& cpCleanPtoUrl,
                         bool celeste,
                         PanoramaFileType fileType,
                         bool gPano,
                         const QString& huginVersion,
                         const QString& cpCleanPath,
                         const QString& cpFindPath);

    void optimizeProject(QUrl& ptoUrl,
                         QUrl& optimizePtoUrl,
                         QUrl& viewCropPtoUrl,
                         bool levelHorizon,
                         bool buildGPano,
                         const QString& autooptimiserPath,
                         const QString& panoModifyPath);

    void generatePanoramaPreview(QSharedPointer<const PTOType> ptoData,
                                 QUrl& previewPtoUrl,
                                 QUrl& previewMkUrl,
                                 QUrl& previewUrl,
                                 const PanoramaItemUrlsMap& preProcessedUrlsMap,
                                 const QString& makePath,
                                 const QString& pto2mkPath,
                                 const QString& huginExecutorPath,
                                 bool hugin2015,
                                 const QString& enblendPath,
                                 const QString& nonaPath);

    void compileProject(QSharedPointer<const PTOType> basePtoData,
                        QUrl& panoPtoUrl,
                        QUrl& mkUrl,
                        QUrl& panoUrl,
                        const PanoramaItemUrlsMap& preProcessedUrlsMap,
                        PanoramaFileType fileType,
                        const QRect& crop,
                        const QString& makePath,
                        const QString& pto2mkPath,
                        const QString& huginExecutorPath,
                        bool hugin2015,
                        const QString& enblendPath,
                        const QString& nonaPath);

    void copyFiles(const QUrl& ptoUrl,
                   const QUrl& panoUrl,
                   const QUrl& finalPanoUrl,
                   const PanoramaItemUrlsMap& preProcessedUrlsMap,
                   bool savePTO,
                   bool addGPlusMetadata);

    void cancel();
    void finish();

Q_SIGNALS:

    void starting(const Digikam::PanoActionData& ad);
    void stepFinished(const Digikam::PanoActionData& ad);
    void jobCollectionFinished(const Digikam::PanoActionData& ad);

    void cpFindPtoReady(const QUrl& cpFindPtoUrl);
    void cpCleanPtoReady(const QUrl& cpCleanPtoUrl);
    void optimizePtoReady(const QUrl& optimizePtoUrl);
    void previewFileReady(const QUrl& previewFileUrl);
    void panoFileReady(const QUrl& panoFileUrl);

private Q_SLOTS:

    void slotDone(ThreadWeaver::JobPointer j);
    void slotStepDone(ThreadWeaver::JobPointer j);
    void slotStarting(ThreadWeaver::JobPointer j);

private:

    void appendStitchingJobs(QSharedPointer<ThreadWeaver::Sequence>& js,
                             const QUrl& ptoUrl,
                             QUrl& mkUrl,
                             QUrl& outputUrl,
                             const PanoramaItemUrlsMap& preProcessedUrlsMap,
                             PanoramaFileType fileType,
                             const QString& makePath,
                             const QString& pto2mkPath,
                             const QString& enblendPath,
                             const QString& nonaPath,
                             bool preview);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // PANO_ACTION_THREAD_H
