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

#include "expoblendingthread.h"

// C++ includes

#include <cmath>

// Under Win32, log2f is not defined...
#ifdef _WIN32
#define log2f(x) (logf(x)*1.4426950408889634f)
#endif

#ifdef __FreeBSD__
#include <osreldate.h>
#    if __FreeBSD_version < 802502
#        define log2f(x) (logf(x)*1.4426950408889634f)
#    endif
#endif

// Qt includes

#include <QPair>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QDateTime>
#include <QFileInfo>
#include <QPointer>
#include <QFuture>
#include <QtConcurrent>
#include <QTemporaryDir>
#include <QProcess>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"
#include "digikam_globals.h"
#include "drawdecoder.h"
#include "metaengine.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "drawdecoderwidget.h"
#include "drawdecoding.h"

namespace Digikam
{

class RawObserver;

class ExpoBlendingThread::Private
{
public:

    explicit Private()
      : cancel(false),
        align(false),
        enfuseVersion4x(true),
        rawObserver(0)
    {
    }

    struct Task
    {
        bool                        align;
        QList<QUrl>                 urls;
        QUrl                        outputUrl;
        QString                     binaryPath;
        ExpoBlendingAction          action;
        EnfuseSettings              enfuseSettings;
    };

    volatile bool                   cancel;
    bool                            align;
    bool                            enfuseVersion4x;

    QMutex                          mutex;
    QMutex                          lock;

    QWaitCondition                  condVar;

    QList<Task*>                    todo;

    QSharedPointer<QTemporaryDir>   preprocessingTmpDir;
    QSharedPointer<QProcess>        enfuseProcess;
    QSharedPointer<QProcess>        alignProcess;

    RawObserver*                    rawObserver;

    /**
     * List of results files produced by enfuse that may need cleaning.
     * Only access this through the provided mutex.
     */
    QList<QUrl>                     enfuseTmpUrls;
    QMutex                          enfuseTmpUrlsMutex;

    // Preprocessing
    QList<QUrl>                     mixedUrls;     // Original non-RAW + Raw converted urls to align.
    ExpoBlendingItemUrlsMap         preProcessedUrlsMap;

    MetaEngine                      meta;
};

class RawObserver : public DImgLoaderObserver
{
public:

    explicit RawObserver(ExpoBlendingThread::Private* const priv)
        : DImgLoaderObserver(),
          d(priv)
    {
    }

    ~RawObserver()
    {
    }

    bool continueQuery(const DImg* const)
    {
        return (!d->cancel);
    }

private:

    ExpoBlendingThread::Private* const d;
};

ExpoBlendingThread::ExpoBlendingThread(QObject* const parent)
    : QThread(parent),
      d(new Private)
{
    d->rawObserver = new RawObserver(d);
    qRegisterMetaType<ExpoBlendingActionData>();
}

ExpoBlendingThread::~ExpoBlendingThread()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "ExpoBlendingThread shutting down."
                                 << "Canceling all actions and waiting for them";

    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Thread finished";

    cleanUpResultFiles();

    delete d;
}

void ExpoBlendingThread::setEnfuseVersion(const double version)
{
    d->enfuseVersion4x = (version >= 4.0);
}

void ExpoBlendingThread::cleanUpResultFiles()
{
    // Cleanup all tmp files created by Enfuse process.
    QMutexLocker(&d->enfuseTmpUrlsMutex);

    foreach(const QUrl& url, d->enfuseTmpUrls)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Removing temp file " << url.toLocalFile();
        QFile(url.toLocalFile()).remove();
    }

    d->enfuseTmpUrls.clear();
}

void ExpoBlendingThread::setPreProcessingSettings(bool align)
{
    d->align = align;
}

void ExpoBlendingThread::identifyFiles(const QList<QUrl>& urlList)
{
    foreach(const QUrl& url, urlList)
    {
        Private::Task* const t = new Private::Task;
        t->action              = EXPOBLENDING_IDENTIFY;
        t->urls.append(url);

        QMutexLocker lock(&d->mutex);
        d->todo << t;
        d->condVar.wakeAll();
    }
}

void ExpoBlendingThread::loadProcessed(const QUrl& url)
{
    Private::Task* const t = new Private::Task;
    t->action              = EXPOBLENDING_LOAD;
    t->urls.append(url);

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ExpoBlendingThread::preProcessFiles(const QList<QUrl>& urlList, const QString& alignPath)
{
    Private::Task* const t = new Private::Task;
    t->action              = EXPOBLENDING_PREPROCESSING;
    t->urls                = urlList;
    t->align               = d->align;
    t->binaryPath          = alignPath;

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ExpoBlendingThread::enfusePreview(const QList<QUrl>& alignedUrls, const QUrl& outputUrl,
                                       const EnfuseSettings& settings, const QString& enfusePath)
{
    Private::Task* const t = new Private::Task;
    t->action              = EXPOBLENDING_ENFUSEPREVIEW;
    t->urls                = alignedUrls;
    t->outputUrl           = outputUrl;
    t->enfuseSettings      = settings;
    t->binaryPath          = enfusePath;

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ExpoBlendingThread::enfuseFinal(const QList<QUrl>& alignedUrls, const QUrl& outputUrl,
                                     const EnfuseSettings& settings, const QString& enfusePath)
{
    Private::Task* const t = new Private::Task;
    t->action              = EXPOBLENDING_ENFUSEFINAL;
    t->urls                = alignedUrls;
    t->outputUrl           = outputUrl;
    t->enfuseSettings      = settings;
    t->binaryPath          = enfusePath;

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ExpoBlendingThread::cancel()
{
    QMutexLocker lock(&d->mutex);
    d->todo.clear();
    d->cancel = true;

    if (d->enfuseProcess)
        d->enfuseProcess->kill();

    if (d->alignProcess)
        d->alignProcess->kill();

    d->condVar.wakeAll();
}

void ExpoBlendingThread::run()
{
    d->cancel = false;

    while (!d->cancel)
    {
        Private::Task* t = 0;
        {
            QMutexLocker lock(&d->mutex);

            if (!d->todo.isEmpty())
                t = d->todo.takeFirst();
            else
                d->condVar.wait(&d->mutex);
        }

        if (t)
        {
            switch (t->action)
            {
                case EXPOBLENDING_IDENTIFY:
                {
                    // Identify Exposure.

                    QString avLum;

                    if (!t->urls.isEmpty())
                    {
                        float val = getAverageSceneLuminance(t->urls[0]);

                        if (val != -1)
                            avLum.setNum(log2f(val), 'g', 2);
                    }

                    ExpoBlendingActionData ad;
                    ad.action  = t->action;
                    ad.inUrls  = t->urls;
                    ad.message = avLum.isEmpty() ? i18nc("average scene luminance value unknown", "unknown") : avLum;
                    ad.success = avLum.isEmpty();
                    emit finished(ad);
                    break;
                }

                case EXPOBLENDING_PREPROCESSING:
                {
                    ExpoBlendingActionData ad1;
                    ad1.action   = EXPOBLENDING_PREPROCESSING;
                    ad1.inUrls   = t->urls;
                    ad1.starting = true;
                    emit starting(ad1);

                    QString errors;

                    bool result  = startPreProcessing(t->urls, t->align, t->binaryPath, errors);

                    ExpoBlendingActionData ad2;
                    ad2.action              = EXPOBLENDING_PREPROCESSING;
                    ad2.inUrls              = t->urls;
                    ad2.preProcessedUrlsMap = d->preProcessedUrlsMap;
                    ad2.success             = result;
                    ad2.message             = errors;
                    emit finished(ad2);
                    break;
                }

                case EXPOBLENDING_LOAD:
                {
                    ExpoBlendingActionData ad1;
                    ad1.action   = EXPOBLENDING_LOAD;
                    ad1.inUrls   = t->urls;
                    ad1.starting = true;
                    emit starting(ad1);

                    QImage image;
                    bool result  = image.load(t->urls[0].toLocalFile());

                    // rotate image

                    if (result)
                    {
                        if (d->meta.load(t->urls[0].toLocalFile()))
                            d->meta.rotateExifQImage(image, d->meta.getImageOrientation());
                    }

                    ExpoBlendingActionData ad2;
                    ad2.action         = EXPOBLENDING_LOAD;
                    ad2.inUrls         = t->urls;
                    ad2.success        = result;
                    ad2.image          = image;
                    emit finished(ad2);
                    break;
                }

                case EXPOBLENDING_ENFUSEPREVIEW:
                {
                    ExpoBlendingActionData ad1;
                    ad1.action         = EXPOBLENDING_ENFUSEPREVIEW;
                    ad1.inUrls         = t->urls;
                    ad1.starting       = true;
                    ad1.enfuseSettings = t->enfuseSettings;
                    emit starting(ad1);

                    QString errors;
                    QUrl    destUrl         = t->outputUrl;
                    EnfuseSettings settings = t->enfuseSettings;
                    settings.outputFormat   = DSaveSettingsWidget::OUTPUT_JPEG;    // JPEG for preview: fast and small.
                    bool result             = startEnfuse(t->urls, destUrl, settings, t->binaryPath, errors);

                    qCDebug(DIGIKAM_GENERAL_LOG) << "Preview result was: " << result;

                    // preserve exif information for auto rotation

                    if (result)
                    {
                        if (d->meta.load(t->urls[0].toLocalFile()))
                        {
                            MetaEngine::ImageOrientation orientation = d->meta.getImageOrientation();

                            if (d->meta.load(destUrl.toLocalFile()))
                            {
                                d->meta.setImageOrientation(orientation);
                                d->meta.applyChanges();
                            }
                        }
                    }

                    // To be cleaned in destructor.
                    QMutexLocker(&d->enfuseTmpUrlsMutex);
                    d->enfuseTmpUrls << destUrl;

                    ExpoBlendingActionData ad2;
                    ad2.action         = EXPOBLENDING_ENFUSEPREVIEW;
                    ad2.inUrls         = t->urls;
                    ad2.outUrls        = QList<QUrl>() << destUrl;
                    ad2.success        = result;
                    ad2.message        = errors;
                    ad2.enfuseSettings = t->enfuseSettings;
                    emit finished(ad2);
                    break;
                }

                case EXPOBLENDING_ENFUSEFINAL:
                {
                    ExpoBlendingActionData ad1;
                    ad1.action         = EXPOBLENDING_ENFUSEFINAL;
                    ad1.inUrls         = t->urls;
                    ad1.starting       = true;
                    ad1.enfuseSettings = t->enfuseSettings;
                    emit starting(ad1);

                    QUrl    destUrl = t->outputUrl;
                    bool    result  = false;
                    QString errors;

                    result = startEnfuse(t->urls, destUrl, t->enfuseSettings, t->binaryPath, errors);

                    // We will take first image metadata from stack to restore Exif, Iptc, and Xmp.

                    if (d->meta.load(t->urls[0].toLocalFile()))
                    {
                        result = result & d->meta.setXmpTagString("Xmp.digiKam.EnfuseInputFiles",
                                                                  t->enfuseSettings.inputImagesList());

                        result = result & d->meta.setXmpTagString("Xmp.digiKam.EnfuseSettings",
                                                                  t->enfuseSettings.asCommentString().
                                                                  replace(QChar::fromLatin1('\n'),
                                                                  QLatin1String(" ; ")));

                        d->meta.setImageDateTime(QDateTime::currentDateTime());

                        if (t->enfuseSettings.outputFormat != DSaveSettingsWidget::OUTPUT_JPEG)
                        {
                            QImage img;

                            if (img.load(destUrl.toLocalFile()))
                                d->meta.setImagePreview(img.scaled(1280, 1024, Qt::KeepAspectRatio));
                        }

                        d->meta.save(destUrl.toLocalFile());
                    }

                    // To be cleaned in destructor.
                    QMutexLocker(&d->enfuseTmpUrlsMutex);
                    d->enfuseTmpUrls << destUrl;

                    ExpoBlendingActionData ad2;
                    ad2.action         = EXPOBLENDING_ENFUSEFINAL;
                    ad2.inUrls         = t->urls;
                    ad2.outUrls        = QList<QUrl>() << destUrl;
                    ad2.success        = result;
                    ad2.message        = errors;
                    ad2.enfuseSettings = t->enfuseSettings;
                    emit finished(ad2);
                    break;
                }

                default:
                {
                    qCritical() << "Unknown action specified" << endl;
                    break;
                }
            }
        }

        delete t;
    }
}

void ExpoBlendingThread::preProcessingMultithreaded(const QUrl& url, volatile bool& error)
{
    if (error)
    {
        return;
    }

    // check if we have to RAW file -> use preview image then

    if (DRawDecoder::isRawFile(url))
    {
        QUrl preprocessedUrl, previewUrl;

        if (!convertRaw(url, preprocessedUrl))
        {
            error = true;
            return;
        }

        if (!computePreview(preprocessedUrl, previewUrl))
        {
            error = true;
            return;
        }

        d->lock.lock();
        d->mixedUrls.append(preprocessedUrl);
        // In case of alignment is not performed.
        d->preProcessedUrlsMap.insert(url, ExpoBlendingItemPreprocessedUrls(preprocessedUrl, previewUrl));
        d->lock.unlock();
    }
    else
    {
        // NOTE: in this case, preprocessed Url is original file Url.
        QUrl previewUrl;

        if (!computePreview(url, previewUrl))
        {
            error = true;
            return;
        }

        d->lock.lock();
        d->mixedUrls.append(url);
        // In case of alignment is not performed.
        d->preProcessedUrlsMap.insert(url, ExpoBlendingItemPreprocessedUrls(url, previewUrl));
        d->lock.unlock();
    }
}

bool ExpoBlendingThread::startPreProcessing(const QList<QUrl>& inUrls,
                                            bool align,
                                            const QString& alignPath, QString& errors)
{
    QString prefix = QDir::tempPath() + QLatin1Char('/') +
                     QLatin1String("digiKam-expoblending-tmp-XXXXXX");

    d->preprocessingTmpDir = QSharedPointer<QTemporaryDir>(new QTemporaryDir(prefix));

    qCDebug(DIGIKAM_GENERAL_LOG) << "Temp dir : " << d->preprocessingTmpDir->path();

    // Parallelized pre-process RAW files if necessary.
    d->mixedUrls.clear();
    d->preProcessedUrlsMap.clear();

    volatile bool error = false;

    QList <QFuture<void> > tasks;

    for (int i = 0; i < inUrls.size(); ++i)
    {
        QUrl url = inUrls.at(i);

        tasks.append(QtConcurrent::run(this,
                                       &ExpoBlendingThread::preProcessingMultithreaded,
                                       url,
                                       error
                                      ));
    }

    for (QFuture<void>& t: tasks)
    {
        t.waitForFinished();
    }

    if (error)
    {
        return false;
    }

    if (align)
    {
        // Re-align images

        d->alignProcess.reset(new QProcess());
        d->alignProcess->setWorkingDirectory(d->preprocessingTmpDir->path());
        d->alignProcess->setProcessChannelMode(QProcess::MergedChannels);
        d->alignProcess->setProcessEnvironment(adjustedEnvironmentForAppImage());

        QStringList args;
        args << QLatin1String("-v");
        args << QLatin1String("-a");
        args << QLatin1String("aligned");
        foreach(const QUrl& url, d->mixedUrls)
        {
            args << url.toLocalFile();
        }

        d->alignProcess->setProgram(alignPath);
        d->alignProcess->setArguments(args);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Align command line: " << d->alignProcess->program();

        d->alignProcess->start();

        if (!d->alignProcess->waitForFinished(-1))
        {
            errors = getProcessError(*(d->alignProcess));
            qCDebug(DIGIKAM_GENERAL_LOG) << "align_image_stack error: " << errors;
            return false;
        }

        uint    i=0;
        QString temp;
        d->preProcessedUrlsMap.clear();

        foreach(const QUrl& url, inUrls)
        {
            QUrl previewUrl;
            QUrl alignedUrl = QUrl::fromLocalFile(
                d->preprocessingTmpDir->path()
                + QChar::fromLatin1('/')
                + QLatin1String("aligned")
                + QString::number(i).rightJustified(4, QChar::fromLatin1('0'))
                + QLatin1String(".tif"));

            if (!computePreview(alignedUrl, previewUrl))
            {
                return false;
            }

            d->preProcessedUrlsMap.insert(url, ExpoBlendingItemPreprocessedUrls(alignedUrl, previewUrl));
            i++;
        }

        foreach(const QUrl& inputUrl, d->preProcessedUrlsMap.keys())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Pre-processed output urls map: "
                                         << inputUrl << "=>"
                                         << d->preProcessedUrlsMap[inputUrl].preprocessedUrl << ","
                                         << d->preProcessedUrlsMap[inputUrl].previewUrl << ";";
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Align exit status    : " << d->alignProcess->exitStatus();
        qCDebug(DIGIKAM_GENERAL_LOG) << "Align exit code      : " << d->alignProcess->exitCode();

        if (d->alignProcess->exitStatus() != QProcess::NormalExit)
        {
            return false;
        }

        if (d->alignProcess->exitCode() == 0)
        {
            // Process finished successfully !
            return true;
        }

        errors = getProcessError(*(d->alignProcess));
        return false;
    }
    else
    {
        foreach(const QUrl& inputUrl, d->preProcessedUrlsMap.keys())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Pre-processed output urls map: "
                                         << inputUrl << "=>"
                                         << d->preProcessedUrlsMap[inputUrl].preprocessedUrl << ","
                                         << d->preProcessedUrlsMap[inputUrl].previewUrl << ";";
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Alignment not performed.";
        return true;
    }
}

bool ExpoBlendingThread::computePreview(const QUrl& inUrl, QUrl& outUrl)
{
    outUrl = QUrl::fromLocalFile(
        d->preprocessingTmpDir->path()
        + QChar::fromLatin1('/')
        + QChar::fromLatin1('.')
        + inUrl.fileName().replace(QChar::fromLatin1('.'), QChar::fromLatin1('_'))
        + QLatin1String("-preview.jpg"));

    DImg img;

    if (img.load(inUrl.toLocalFile()))
    {
        DImg preview = img.smoothScale(1280, 1024, Qt::KeepAspectRatio);
        bool saved   = preview.save(outUrl.toLocalFile(), QLatin1String("JPG"));

        // save exif information also to preview image for auto rotation

        if (saved)
        {
            if (d->meta.load(inUrl.toLocalFile()))
            {
                MetaEngine::ImageOrientation orientation = d->meta.getImageOrientation();

                if (d->meta.load(outUrl.toLocalFile()))
                {
                    d->meta.setImageOrientation(orientation);
                    d->meta.applyChanges();
                }
            }
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Preview Image url: " << outUrl << ", saved: " << saved;
        return saved;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Input image not loaded:" << inUrl;

    return false;
}

bool ExpoBlendingThread::convertRaw(const QUrl& inUrl, QUrl& outUrl)
{
    DImg img;

    DRawDecoding settings;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    DRawDecoderWidget::readSettings(settings.rawPrm, group);

    if (img.load(inUrl.toLocalFile(), d->rawObserver, settings))
    {
        if (d->meta.load(inUrl.toLocalFile()))
        {
            d->meta.setImageProgramId(QLatin1String("digiKam"), digiKamVersion());
            d->meta.setImageDimensions(img.size());
            d->meta.setExifTagString("Exif.Image.DocumentName", inUrl.fileName());
            d->meta.setXmpTagString("Xmp.tiff.Make",  d->meta.getExifTagString("Exif.Image.Make"));
            d->meta.setXmpTagString("Xmp.tiff.Model", d->meta.getExifTagString("Exif.Image.Model"));
            d->meta.setImageOrientation(MetaEngine::ORIENTATION_NORMAL);

            QFileInfo fi(inUrl.toLocalFile());
            outUrl = QUrl::fromLocalFile(
                d->preprocessingTmpDir->path()
                + QChar::fromLatin1('/')
                + QChar::fromLatin1('.')
                + fi.completeBaseName().replace(QChar::fromLatin1('.'), QChar::fromLatin1('_'))
                + QLatin1String(".tif"));

            if (!img.save(outUrl.toLocalFile(), QLatin1String("TIF")))
            {
                return false;
            }

            d->meta.save(outUrl.toLocalFile());
        }
    }
    else
    {
        return false;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Convert RAW output url: " << outUrl;

    return true;
}

bool ExpoBlendingThread::startEnfuse(const QList<QUrl>& inUrls, QUrl& outUrl,
                               const EnfuseSettings& settings,
                               const QString& enfusePath, QString& errors)
{
    QString comp;
    QString ext = DSaveSettingsWidget::extensionForFormat(settings.outputFormat);

    if (ext == QLatin1String(".tif"))
    {
        comp = QLatin1String("--compression=DEFLATE");
    }

    outUrl.setPath(outUrl.adjusted(QUrl::RemoveFilename).path() + QLatin1String(".digiKam-expoblending-tmp-") +
                                                                  QString::number(QDateTime::currentDateTime().toTime_t()) + ext);

    d->enfuseProcess.reset(new QProcess());
    d->enfuseProcess->setWorkingDirectory(d->preprocessingTmpDir->path());
    d->enfuseProcess->setProcessChannelMode(QProcess::MergedChannels);
    d->enfuseProcess->setProcessEnvironment(adjustedEnvironmentForAppImage());

    QStringList args;

    if (!settings.autoLevels)
    {
        args << QLatin1String("-l");
        args << QString::number(settings.levels);
    }

    if (settings.ciecam02)
    {
        args << QLatin1String("-c");
    }

    if (!comp.isEmpty())
    {
        args << comp;
    }

    if (settings.hardMask)
    {
        if (d->enfuseVersion4x)
            args << QLatin1String("--hard-mask");
        else
            args << QLatin1String("--HardMask");
    }

    if (d->enfuseVersion4x)
    {
        args << QString::fromUtf8("--exposure-weight=%1").arg(settings.exposure);
        args << QString::fromUtf8("--saturation-weight=%1").arg(settings.saturation);
        args << QString::fromUtf8("--contrast-weight=%1").arg(settings.contrast);
    }
    else
    {
        args << QString::fromUtf8("--wExposure=%1").arg(settings.exposure);
        args << QString::fromUtf8("--wSaturation=%1").arg(settings.saturation);
        args << QString::fromUtf8("--wContrast=%1").arg(settings.contrast);
    }

    args << QLatin1String("-v");
    args << QLatin1String("-o");
    args << outUrl.toLocalFile();

    foreach(const QUrl& url, inUrls)
    {
        args << url.toLocalFile();
    }

    d->enfuseProcess->setProgram(enfusePath);
    d->enfuseProcess->setArguments(args);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Enfuse command line: " << d->enfuseProcess->program();

    d->enfuseProcess->start();

    if (!d->enfuseProcess->waitForFinished(-1))
    {
        errors = getProcessError(*(d->enfuseProcess));
        return false;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Enfuse output url: "  << outUrl;
    qCDebug(DIGIKAM_GENERAL_LOG) << "Enfuse exit status: " << d->enfuseProcess->exitStatus();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Enfuse exit code:   " << d->enfuseProcess->exitCode();

    if (d->enfuseProcess->exitStatus() != QProcess::NormalExit)
    {
        return false;
    }

    if (d->enfuseProcess->exitCode() == 0)
    {
        // Process finished successfully !
        return true;
    }

    errors = getProcessError(*(d->enfuseProcess));
    return false;
}

QString ExpoBlendingThread::getProcessError(QProcess& proc) const
{
    QString std = QString::fromLocal8Bit(proc.readAll());
    return (i18n("Cannot run %1:\n\n %2", proc.program(), std));
}

/**
 * This function obtains the "average scene luminance" (cd/m^2) from an image file.
 * "average scene luminance" is the L (aka B) value mentioned in [1]
 * You have to take a log2f of the returned value to get an EV value.
 *
 * We are using K=12.07488f and the exif-implied value of N=1/3.125 (see [1]).
 * K=12.07488f is the 1.0592f * 11.4f value in pfscalibration's pfshdrcalibrate.cpp file.
 * Based on [3] we can say that the value can also be 12.5 or even 14.
 * Another reference for APEX is [4] where N is 0.3, closer to the APEX specification of 2^(-7/4)=0.2973.
 *
 * [1] http://en.wikipedia.org/wiki/APEX_system
 * [2] http://en.wikipedia.org/wiki/Exposure_value
 * [3] http://en.wikipedia.org/wiki/Light_meter
 * [4] http://doug.kerr.home.att.net/pumpkin/#APEX
 *
 * This function tries first to obtain the shutter speed from either of
 * two exif tags (there is no standard between camera manifacturers):
 * ExposureTime or ShutterSpeedValue.
 * Same thing for f-number: it can be found in FNumber or in ApertureValue.
 *
 * F-number and shutter speed are mandatory in exif data for EV calculation, iso is not.
 */
float ExpoBlendingThread::getAverageSceneLuminance(const QUrl& url)
{
    if (!d->meta.load(url.toLocalFile()))
        return -1;

    if (!d->meta.hasExif())
        return -1;

    long num = 1, den = 1;

    // default not valid values

    float    expo = -1.0;
    float    iso  = -1.0;
    float    fnum = -1.0;
    QVariant rationals;

    if (d->meta.getExifTagRational("Exif.Photo.ExposureTime", num, den))
    {
        if (den)
            expo = (float)(num) / (float)(den);
    }
    else if (getXmpRational("Xmp.exif.ExposureTime", num, den, &d->meta))
    {
        if (den)
            expo = (float)(num) / (float)(den);
    }
    else if (d->meta.getExifTagRational("Exif.Photo.ShutterSpeedValue", num, den))
    {
        long   nmr = 1, div = 1;
        double tmp = 0.0;

        if (den)
            tmp = exp(log(2.0) * (float)(num) / (float)(den));

        if (tmp > 1.0)
        {
            div = (long)(tmp + 0.5);
        }
        else
        {
            nmr = (long)(1 / tmp + 0.5);
        }

        if (div)
            expo = (float)(nmr) / (float)(div);
    }
    else if (getXmpRational("Xmp.exif.ShutterSpeedValue", num, den, &d->meta))
    {
        long   nmr = 1, div = 1;
        double tmp = 0.0;

        if (den)
            tmp = exp(log(2.0) * (float)(num) / (float)(den));

        if (tmp > 1.0)
        {
            div = (long)(tmp + 0.5);
        }
        else
        {
            nmr = (long)(1 / tmp + 0.5);
        }

        if (div)
            expo = (float)(nmr) / (float)(div);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << url.fileName() << " : expo = " << expo;

    if (d->meta.getExifTagRational("Exif.Photo.FNumber", num, den))
    {
        if (den)
            fnum = (float)(num) / (float)(den);
    }
    else if (getXmpRational("Xmp.exif.FNumber", num, den, &d->meta))
    {
        if (den)
            fnum = (float)(num) / (float)(den);
    }
    else if (d->meta.getExifTagRational("Exif.Photo.ApertureValue", num, den))
    {
        if (den)
            fnum = (float)(exp(log(2.0) * (float)(num) / (float)(den) / 2.0));
    }
    else if (getXmpRational("Xmp.exif.ApertureValue", num, den, &d->meta))
    {
        if (den)
            fnum = (float)(exp(log(2.0) * (float)(num) / (float)(den) / 2.0));
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << url.fileName() << " : fnum = " << fnum;

    // Some cameras/lens DO print the fnum but with value 0, and this is not allowed for ev computation purposes.

    if (fnum == 0.0)
        return -1.0;

    // If iso is found use that value, otherwise assume a value of iso=100. (again, some cameras do not print iso in exif).

    if (d->meta.getExifTagRational("Exif.Photo.ISOSpeedRatings", num, den))
    {
        if (den)
            iso = (float)(num) / (float)(den);
    }
    else if (getXmpRational("Xmp.exif.ISOSpeedRatings", num, den, &d->meta))
    {
        if (den)
            iso = (float)(num) / (float)(den);
    }
    else
    {
        iso = 100.0;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << url.fileName() << " : iso = " << iso;

    // At this point the three variables have to be != -1

    if (expo != -1.0 && iso != -1.0 && fnum != -1.0)
    {
        float asl = (expo * iso) / (fnum * fnum * 12.07488f);
        qCDebug(DIGIKAM_GENERAL_LOG) << url.fileName() << " : ASL ==> " << asl;

        return asl;
    }

    return -1.0;
}

bool ExpoBlendingThread::getXmpRational(const char* xmpTagName, long& num, long& den, MetaEngine* const meta)
{
    QVariant rationals = meta->getXmpTagVariant(xmpTagName);

    if (!rationals.isNull())
    {
        QVariantList list = rationals.toList();

        if (list.size() == 2)
        {
            num = list[0].toInt();
            den = list[1].toInt();

            return true;
        }
    }

    return false;
}

} // namespace Digikam
