/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "preprocesstask.h"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dbinarysearch.h"
#include "digikam_debug.h"
#include "drawdecoder.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "drawdecoderwidget.h"
#include "drawdecoding.h"



namespace Digikam
{

class PanoObserver;

class PreProcessTask::Private
{
public:

    Private(PanoramaPreprocessedUrls& urls, const QUrl& url)
        : fileUrl(url),
          preProcessedUrl(urls),
          observer(0)
    {
    }

    const QUrl                fileUrl;
    PanoramaPreprocessedUrls& preProcessedUrl;
    DMetadata                 meta;
    PanoObserver*             observer;
};

class PanoObserver : public DImgLoaderObserver
{
public:

    explicit PanoObserver(PreProcessTask* const p)
        : DImgLoaderObserver(),
          parent(p)
    {
    }

    ~PanoObserver()
    {
    }

    bool continueQuery(const DImg* const)
    {
        return (!parent->isAbortedFlag);
    }

private:

    PreProcessTask* const parent;
};

PreProcessTask::PreProcessTask(const QString& workDirPath, int id, PanoramaPreprocessedUrls& targetUrls,
                               const QUrl& sourceUrl)
    : PanoTask(PANO_PREPROCESS_INPUT, workDirPath),
      id(id),
      d(new Private(targetUrls, sourceUrl))
{
    d->observer        = new PanoObserver(this);
}

PreProcessTask::~PreProcessTask()
{
}

void PreProcessTask::requestAbort()
{
    PanoTask::requestAbort();
}

void PreProcessTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    // check if its a RAW file.
    if (DRawDecoder::isRawFile(d->fileUrl))
    {
        d->preProcessedUrl.preprocessedUrl = tmpDir;

        if (!convertRaw())
        {
            successFlag = false;
            return;
        }
    }
    else
    {
        // NOTE: in this case, preprocessed Url is the original file Url.
        d->preProcessedUrl.preprocessedUrl = d->fileUrl;
    }

    d->preProcessedUrl.previewUrl = tmpDir;

    if (!computePreview(d->preProcessedUrl.preprocessedUrl))
    {
        successFlag = false;
        return;
    }

    successFlag = true;
    return;
}

bool PreProcessTask::computePreview(const QUrl& inUrl)
{
    QUrl& outUrl = d->preProcessedUrl.previewUrl;

    QFileInfo fi(inUrl.toLocalFile());
    outUrl.setPath(outUrl.path() + fi.completeBaseName().replace(QLatin1String("."), QLatin1String("_"))
                                 + QLatin1String("-preview.jpg"));

    DImg img;

    if (img.load(inUrl.toLocalFile()))
    {
        DImg preview = img.smoothScale(1280, 1024, Qt::KeepAspectRatio);
        bool saved   = preview.save(outUrl.toLocalFile(), DImg::JPEG);

        // save exif information also to preview image for auto rotation
        if (saved)
        {
            d->meta.load(inUrl.toLocalFile());
            MetaEngine::ImageOrientation orientation = d->meta.getImageOrientation();

            d->meta.load(outUrl.toLocalFile());
            d->meta.setImageOrientation(orientation);
            d->meta.setImageDimensions(QSize(preview.width(), preview.height()));
            d->meta.applyChanges();
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Preview Image url: " << outUrl << ", saved: " << saved;
        return saved;
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Error during preview generation of: " << inUrl;
        errString = i18n("Input image cannot be loaded for preview generation.");
    }

    return false;
}

bool PreProcessTask::convertRaw()
{
    const QUrl& inUrl = d->fileUrl;
    QUrl& outUrl      = d->preProcessedUrl.preprocessedUrl;
    DImg img;

    DRawDecoding settings;
    KSharedConfig::Ptr config   = KSharedConfig::openConfig();
    KConfigGroup group          = config->group(QLatin1String("ImageViewer Settings"));
    DRawDecoderWidget::readSettings(settings.rawPrm, group);

    if (img.load(inUrl.toLocalFile(), d->observer, settings))
    {
        d->meta.load(inUrl.toLocalFile());

        DMetadata::MetaDataMap m = d->meta.getExifTagsDataList(QStringList() << QLatin1String("Photo"));

        if (!m.isEmpty())
        {
            for (DMetadata::MetaDataMap::iterator it = m.begin(); it != m.end(); ++it)
            {
                d->meta.removeExifTag(it.key().toLatin1().constData());
            }
        }

        QByteArray exif = d->meta.getExifEncoded();
        QByteArray iptc = d->meta.getIptc();
        QByteArray xmp  = d->meta.getXmp();
        QString make    = d->meta.getExifTagString("Exif.Image.Make");
        QString model   = d->meta.getExifTagString("Exif.Image.Model");

        QFileInfo fi(inUrl.toLocalFile());
        outUrl.setPath(outUrl.path() + fi.completeBaseName().replace(QLatin1String("."), QLatin1String("_"))
                                     + QLatin1String(".tif"));

        if (!img.save(outUrl.toLocalFile(), QLatin1String("TIF")))
        {
            errString = i18n("Tiff image creation failed.");
            return false;
        }

        d->meta.load(outUrl.toLocalFile());
        d->meta.setExif(exif);
        d->meta.setIptc(iptc);
        d->meta.setXmp(xmp);
        d->meta.setImageDimensions(QSize(img.width(), img.height()));
        d->meta.setExifTagString("Exif.Image.DocumentName", inUrl.fileName());
        d->meta.setXmpTagString("Xmp.tiff.Make",  make);
        d->meta.setXmpTagString("Xmp.tiff.Model", model);
        d->meta.setImageOrientation(DMetadata::ORIENTATION_NORMAL);
        d->meta.applyChanges();
    }
    else
    {
        errString = i18n("Raw file conversion failed.");
        return false;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Convert RAW output url: " << outUrl;

    return true;
}

}  // namespace Digikam
