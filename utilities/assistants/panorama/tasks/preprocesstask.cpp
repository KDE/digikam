/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a plugin to create panorama by fusion of several images.
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

using namespace RawEngine;

namespace Digikam
{

class RawObserver : public DImgLoaderObserver
{
public:

    explicit RawObserver(PreProcessTask* const p)
        : DImgLoaderObserver(),
          parent(p)
    {
    }

    ~RawObserver()
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
    : PanoTask(PANO_PREPROCESS_INPUT,
      workDirPath),
      id(id),
      fileUrl(sourceUrl),
      preProcessedUrl(targetUrls)
{
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
    if (DRawDecoder::isRawFile(fileUrl))
    {
        preProcessedUrl.preprocessedUrl = tmpDir;

        if (!convertRaw())
        {
            successFlag = false;
            return;
        }
    }
    else
    {
        // NOTE: in this case, preprocessed Url is the original file Url.
        preProcessedUrl.preprocessedUrl = fileUrl;
    }

    preProcessedUrl.previewUrl = tmpDir;

    if (!computePreview(preProcessedUrl.preprocessedUrl))
    {
        successFlag = false;
        return;
    }

    successFlag = true;
    return;
}

bool PreProcessTask::computePreview(const QUrl& inUrl)
{
    QUrl& outUrl = preProcessedUrl.previewUrl;

    QFileInfo fi(inUrl.toLocalFile());
    outUrl = tmpDir.resolved(QUrl::fromLocalFile(fi.completeBaseName().replace(QLatin1String("."), QLatin1String("_")) + QStringLiteral("-preview.jpg")));

    DImg img;

    if (img.load(inUrl.toLocalFile()))
    {
        DImg preview = img.smoothScale(1280, 1024, Qt::KeepAspectRatio);
        bool saved   = preview.save(outUrl.toLocalFile(), "JPG");

        // save exif information also to preview image for auto rotation
        if (saved)
        {
            m_meta.load(inUrl.toLocalFile());
            MetaEngine::ImageOrientation orientation = m_meta.getImageOrientation();

            m_meta.load(outUrl.toLocalFile());
            m_meta.setImageOrientation(orientation);
            m_meta.setImageDimensions(QSize(preview.width(), preview.height()));
            m_meta.applyChanges();
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
    const QUrl& inUrl = fileUrl;
    QUrl& outUrl      = preProcessedUrl.preprocessedUrl;
    DImg img;

    DRawDecoding settings;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    DRawDecoderWidget::readSettings(settings.rawPrm, group);
    RawObserver observer(this);

    if (img.load(inUrl.toLocalFile(), &observer, settings))
    {
        m_meta.load(inUrl.toLocalFile());

        DMetadata::MetaDataMap m = m_meta.getExifTagsDataList(QStringList() << QLatin1String("Photo"));

        if (!m.isEmpty())
        {
            for (DMetadata::MetaDataMap::iterator it = m.begin(); it != m.end(); ++it)
            {
                m_meta.removeExifTag(it.key().toLatin1().constData());
            }
        }

        QByteArray exif = m_meta.getExifEncoded();
        QByteArray iptc = m_meta.getIptc();
        QByteArray xmp  = m_meta.getXmp();
        QString make    = m_meta.getExifTagString("Exif.Image.Make");
        QString model   = m_meta.getExifTagString("Exif.Image.Model");

        QFileInfo fi(inUrl.toLocalFile());
        outUrl = tmpDir.resolved(QUrl::fromLocalFile(fi.completeBaseName().replace(QLatin1String("."), QLatin1String("_")) + QStringLiteral(".tif")));

        if (!img.save(outUrl.toLocalFile(), QLatin1String("TIF")))
        {
            errString = i18n("Tiff image creation failed.");
            return false;
        }

        m_meta.load(outUrl.toLocalFile());
        m_meta.setExif(exif);
        m_meta.setIptc(iptc);
        m_meta.setXmp(xmp);
        m_meta.setImageDimensions(QSize(img.width(), img.height()));
        m_meta.setExifTagString("Exif.Image.DocumentName", inUrl.fileName());
        m_meta.setXmpTagString("Xmp.tiff.Make",  make);
        m_meta.setXmpTagString("Xmp.tiff.Model", model);
        m_meta.setImageOrientation(DMetadata::ORIENTATION_NORMAL);
        m_meta.applyChanges();
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
