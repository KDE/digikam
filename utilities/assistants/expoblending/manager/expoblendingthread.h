/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#ifndef EXPO_BLENDING_THREAD_H
#define EXPO_BLENDING_THREAD_H

// Qt includes

#include <QThread>

// Local includes

#include "metaengine.h"
#include "enfusesettings.h"
#include "expoblendingactions.h"

class QProcess;

namespace Digikam
{

class ExpoBlendingActionData;

class ExpoBlendingThread : public QThread
{
    Q_OBJECT

public:

    explicit ExpoBlendingThread(QObject* const parent);
    ~ExpoBlendingThread();

    void setEnfuseVersion(const double version);
    void setPreProcessingSettings(bool align);
    void loadProcessed(const QUrl& url);
    void identifyFiles(const QList<QUrl>& urlList);
    void convertRawFiles(const QList<QUrl>& urlList);
    void preProcessFiles(const QList<QUrl>& urlList, const QString& alignPath);
    void enfusePreview(const QList<QUrl>& alignedUrls, const QUrl& outputUrl,
                       const EnfuseSettings& settings, const QString& enfusePath);
    void enfuseFinal(const QList<QUrl>& alignedUrls, const QUrl& outputUrl,
                     const EnfuseSettings& settings, const QString& enfusePath);

    void cancel();

    /**
     * Clean up all temporary results produced so far.
     */
    void cleanUpResultFiles();

Q_SIGNALS:

    void starting(const Digikam::ExpoBlendingActionData& ad);
    void finished(const Digikam::ExpoBlendingActionData& ad);

private:

    void    run();

    void    preProcessingMultithreaded(const QUrl& url, volatile bool& error);
    bool    startPreProcessing(const QList<QUrl>& inUrls, bool  align, const QString& alignPath, QString& errors);
    bool    computePreview(const QUrl& inUrl, QUrl& outUrl);
    bool    convertRaw(const QUrl& inUrl, QUrl& outUrl);

    bool    startEnfuse(const QList<QUrl>& inUrls, QUrl& outUrl,
                        const EnfuseSettings& settings,
                        const QString& enfusePath, QString& errors);

    QString getProcessError(QProcess& proc) const;

    float   getAverageSceneLuminance(const QUrl& url);
    bool    getXmpRational(const char* xmpTagName, long& num, long& den, MetaEngine* const meta);

public:

    class Private;

private:

    Private* const d;
};

} // namespace Digikam

#endif // EXPO_BLENDING_THREAD_H
