/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : image files selector dialog.
 *
 * Copyright (C) 2008-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagedialog.h"

// Qt includes

#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocale.h>
#include <klocalizedstring.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kimageio.h>

// Libkdcraw includes

#include <libkdcraw_version.h>
#include <kdcraw.h>

// Local includes

#include "digikam_debug.h"
#include "ditemtooltip.h"
#include "dmetadata.h"
#include "loadingdescription.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class ImageDialogPreview::Private
{
public:

    Private() :
        imageLabel(0),
        infoLabel(0),
        thumbLoadThread(0)
    {
    }

    QLabel*              imageLabel;
    QLabel*              infoLabel;

    QUrl                 currentURL;

    DMetadata            metaIface;

    ThumbnailLoadThread* thumbLoadThread;
};

ImageDialogPreview::ImageDialogPreview(QWidget* const parent)
    : KPreviewWidgetBase(parent),
      d(new Private)
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    QVBoxLayout* vlay  = new QVBoxLayout(this);
    d->imageLabel      = new QLabel(this);
    d->imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->imageLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    d->infoLabel = new QLabel(this);
    d->infoLabel->setAlignment(Qt::AlignCenter);

    vlay->setMargin(0);
    vlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vlay->addWidget(d->imageLabel);
    vlay->addWidget(d->infoLabel);
    vlay->addStretch();

    setSupportedMimeTypes(KImageIO::mimeTypes());

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));
}

ImageDialogPreview::~ImageDialogPreview()
{
    delete d;
}

QSize ImageDialogPreview::sizeHint() const
{
    return QSize(256, 256);
}

void ImageDialogPreview::resizeEvent(QResizeEvent*)
{
    QMetaObject::invokeMethod(this, "showPreview", Qt::QueuedConnection);
}

void ImageDialogPreview::showPreview()
{
    QUrl url(d->currentURL);
    clearPreview();
    showPreview(url);
}

void ImageDialogPreview::showPreview(const QUrl& url)
{
    if (!url.isValid())
    {
        clearPreview();
        return;
    }

    if (url != d->currentURL)
    {
        clearPreview();
        d->currentURL = url;
        d->thumbLoadThread->find(ThumbnailIdentifier(d->currentURL.toLocalFile()));

        d->metaIface.load(d->currentURL.toLocalFile());
        PhotoInfoContainer info      = d->metaIface.getPhotographInformation();
        VideoInfoContainer videoInfo = d->metaIface.getVideoInformation();

        if (!info.isEmpty())
        {
            DToolTipStyleSheet cnt;
            QString identify("<qt><center>");
            QString make, model, dateTime, aperture, focalLength, exposureTime, sensitivity;
            QString aspectRatio, audioBitRate, audioChannelType, audioCompressor, duration, frameRate, videoCodec;

            if (info.make.isEmpty())
            {
                make = cnt.unavailable;
            }
            else
            {
                make = info.make;
            }

            if (info.model.isEmpty())
            {
                model = cnt.unavailable;
            }
            else
            {
                model = info.model;
            }

            if (!info.dateTime.isValid())
            {
                dateTime = cnt.unavailable;
            }
            else
            {
                dateTime = KLocale::global()->formatDateTime(info.dateTime, KLocale::ShortDate, true);
            }

            if (info.aperture.isEmpty())
            {
                aperture = cnt.unavailable;
            }
            else
            {
                aperture = info.aperture;
            }

            if (info.focalLength.isEmpty())
            {
                focalLength = cnt.unavailable;
            }
            else
            {
                focalLength = info.focalLength;
            }

            if (info.exposureTime.isEmpty())
            {
                exposureTime = cnt.unavailable;
            }
            else
            {
                exposureTime = info.exposureTime;
            }

            if (info.sensitivity.isEmpty())
            {
                sensitivity = cnt.unavailable;
            }
            else
            {
                sensitivity = i18n("%1 ISO", info.sensitivity);
            }

            if (videoInfo.aspectRatio.isEmpty())
            {
                aspectRatio = cnt.unavailable;
            }
            else
            {
                aspectRatio = videoInfo.aspectRatio;
            }

            if (videoInfo.audioBitRate.isEmpty())
            {
                audioBitRate = cnt.unavailable;
            }
            else
            {
                audioBitRate = videoInfo.audioBitRate;
            }

            if (videoInfo.audioChannelType.isEmpty())
            {
                audioChannelType = cnt.unavailable;
            }
            else
            {
                audioChannelType = videoInfo.audioChannelType;
            }

            if (videoInfo.audioCompressor.isEmpty())
            {
                audioCompressor = cnt.unavailable;
            }
            else
            {
                audioCompressor = videoInfo.audioCompressor;
            }

            if (videoInfo.duration.isEmpty())
            {
                duration = cnt.unavailable;
            }
            else
            {
                duration = videoInfo.duration;
            }

            if (videoInfo.frameRate.isEmpty())
            {
                frameRate = cnt.unavailable;
            }
            else
            {
                frameRate = videoInfo.frameRate;
            }

            if (videoInfo.videoCodec.isEmpty())
            {
                videoCodec = cnt.unavailable;
            }
            else
            {
                videoCodec = videoInfo.videoCodec;
            }

            identify += "<table cellspacing=0 cellpadding=0>";
            identify += cnt.cellBeg + i18n("<i>Make:</i>")              + cnt.cellMid + make                + cnt.cellEnd;
            identify += cnt.cellBeg + i18n("<i>Model:</i>")             + cnt.cellMid + model               + cnt.cellEnd;
            identify += cnt.cellBeg + i18n("<i>Created:</i>")           + cnt.cellMid + dateTime            + cnt.cellEnd;
            identify += cnt.cellBeg + i18n("<i>Aperture:</i>")          + cnt.cellMid + aperture            + cnt.cellEnd;
            identify += cnt.cellBeg + i18n("<i>Focal:</i>")             + cnt.cellMid + focalLength         + cnt.cellEnd;
            identify += cnt.cellBeg + i18n("<i>Exposure:</i>")          + cnt.cellMid + exposureTime        + cnt.cellEnd;
            identify += cnt.cellBeg + i18n("<i>Sensitivity:</i>")       + cnt.cellMid + sensitivity         + cnt.cellEnd;

            if (!videoInfo.isEmpty())
            {
                identify += cnt.cellBeg + i18n("<i>AspectRatio:</i>")       + cnt.cellMid + aspectRatio         + cnt.cellEnd;
                identify += cnt.cellBeg + i18n("<i>AudioBitRate:</i>")      + cnt.cellMid + audioBitRate        + cnt.cellEnd;
                identify += cnt.cellBeg + i18n("<i>AudioChannelType:</i>")  + cnt.cellMid + audioChannelType    + cnt.cellEnd;
                identify += cnt.cellBeg + i18n("<i>AudioCompressor:</i>")   + cnt.cellMid + audioCompressor     + cnt.cellEnd;
                identify += cnt.cellBeg + i18n("<i>Duration:</i>")          + cnt.cellMid + duration            + cnt.cellEnd;
                identify += cnt.cellBeg + i18n("<i>FrameRate:</i>")         + cnt.cellMid + frameRate           + cnt.cellEnd;
                identify += cnt.cellBeg + i18n("<i>VideoCodec:</i>")        + cnt.cellMid + videoCodec          + cnt.cellEnd;
            }

            identify += "</table></center></qt>";

            d->infoLabel->setText(identify);
        }
        else
        {
            d->infoLabel->clear();
        }
    }
}

void ImageDialogPreview::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (QUrl(desc.filePath) == d->currentURL)
    {
        QPixmap pixmap;
        QSize   s = d->imageLabel->contentsRect().size();

        if (s.width() < pix.width() || s.height() < pix.height())
        {
            pixmap = pix.scaled(s, Qt::KeepAspectRatio);
        }
        else
        {
            pixmap = pix;
        }

        d->imageLabel->setPixmap(pixmap);
    }
}

void ImageDialogPreview::clearPreview()
{
    d->imageLabel->clear();
    d->infoLabel->clear();
    d->currentURL = QUrl();
}

// ------------------------------------------------------------------------

class ImageDialog::Private
{

public:

    Private() :
        singleSelect(false)
    {
    }

    bool       singleSelect;

    QString    fileFormats;

    QUrl       url;
    QList<QUrl> urls;
};

ImageDialog::ImageDialog(QWidget* const parent, const QUrl& url, bool singleSelect, const QString& caption)
    : d(new Private)
{
    d->singleSelect         = singleSelect;
    QStringList patternList = KImageIO::pattern(KImageIO::Reading).split('\n', QString::SkipEmptyParts);

    // All Images from list must been always the first entry given by KDE API
    QString allPictures     = patternList[0];

    allPictures.insert(allPictures.indexOf("|"), QString(KDcrawIface::KDcraw::rawFiles()) +
                                                 QString(" *.JPE *.TIF *.PGF"));
    patternList.removeAll(patternList[0]);
    // Added RAW file formats supported by dcraw program like a type mime.
    // Note: we cannot use here "image/x-raw" type mime from KDE because it is incomplete
    // or unavailable(see file #121242 in bug).
    patternList.prepend(i18n("%1|Camera RAW files", QString(KDcrawIface::KDcraw::rawFiles())));
    patternList.prepend(allPictures);
    patternList.append(i18n("*.pgf|Progressive Graphics file"));

    d->fileFormats = patternList.join("\n");

    qCDebug(DIGIKAM_GENERAL_LOG) << "file formats=" << d->fileFormats;

    QPointer<KFileDialog> dlg   = new KFileDialog(url, d->fileFormats, parent);
    ImageDialogPreview* preview = new ImageDialogPreview(dlg);
    dlg->setPreviewWidget(preview);
    dlg->setOperationMode(KFileDialog::Opening);

    if (d->singleSelect)
    {
        dlg->setMode(KFile::File);

        if (caption.isEmpty())
        {
            dlg->setWindowTitle(i18n("Select an Image"));
        }
        else
        {
            dlg->setWindowTitle(caption);
        }

        dlg->exec();
        d->url = dlg->selectedUrl();
    }
    else
    {
        dlg->setMode(KFile::Files);

        if (caption.isEmpty())
        {
            dlg->setWindowTitle(i18n("Select Images"));
        }
        else
        {
            dlg->setWindowTitle(caption);
        }

        dlg->exec();
        d->urls = dlg->selectedUrls();
    }

    delete dlg;
}

ImageDialog::~ImageDialog()
{
    delete d;
}

bool ImageDialog::singleSelect() const
{
    return d->singleSelect;
}

QString ImageDialog::fileFormats() const
{
    return d->fileFormats;
}

QUrl ImageDialog::url() const
{
    return d->url;
}

QList<QUrl> ImageDialog::urls() const
{
    return d->urls;
}

QList<QUrl> ImageDialog::getImageURLs(QWidget* const parent, const QUrl& url, const QString& caption)
{
    ImageDialog dlg(parent, url, false, caption);

    if (!dlg.urls().isEmpty())
    {
        return dlg.urls();
    }
    else
    {
        return QList<QUrl>();
    }
}

QUrl ImageDialog::getImageURL(QWidget* const parent, const QUrl& url, const QString& caption)
{
    ImageDialog dlg(parent, url, true, caption);

    if (dlg.url() != QUrl())
    {
        return dlg.url();
    }
    else
    {
        return QUrl();
    }
}

} // namespace Digikam
