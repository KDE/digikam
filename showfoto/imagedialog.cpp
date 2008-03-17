/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : image files selector dialog.
 * 
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

// Qt includes.

#include <QLabel>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kimageio.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawbinary.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "imagedialog.h"
#include "imagedialog.moc"

namespace ShowFoto
{

class ImageDialogPreviewPrivate 
{

public:

    ImageDialogPreviewPrivate()
    {
        imageLabel      = 0;
        infoLabel       = 0;
        thumbLoadThread = 0;
    }

    QLabel               *imageLabel;
    QLabel               *infoLabel;

    KUrl                  currentURL;

    DMetadata             metaIface;

    ThumbnailLoadThread *thumbLoadThread;
};

ImageDialogPreview::ImageDialogPreview(QWidget *parent)
                  : KPreviewWidgetBase(parent)
{
    d = new ImageDialogPreviewPrivate;

    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    QVBoxLayout *vlay = new QVBoxLayout(this);
    d->imageLabel     = new QLabel(this);
    d->imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->imageLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    d->infoLabel = new QLabel(this);

    vlay->setMargin(0);
    vlay->setSpacing(KDialog::spacingHint());
    vlay->addWidget(d->imageLabel);
    vlay->addWidget(d->infoLabel);

    setSupportedMimeTypes(KImageIO::mimeTypes());
    setMinimumWidth(128);

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnail(const LoadingDescription&, const QPixmap&)));
}

ImageDialogPreview::~ImageDialogPreview() 
{
    delete d;
}

QSize ImageDialogPreview::sizeHint() const
{
    return QSize(100, 200);
}

void ImageDialogPreview::resizeEvent(QResizeEvent *)
{
    QMetaObject::invokeMethod(this, "showPreview", Qt::QueuedConnection);
}

void ImageDialogPreview::showPreview()
{
    KUrl url(d->currentURL);
    clearPreview();
    showPreview(url);
}

void ImageDialogPreview::showPreview(const KUrl& url)
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
        d->thumbLoadThread->find(d->currentURL.path());

        d->metaIface.load(d->currentURL.path());
        PhotoInfoContainer info = d->metaIface.getPhotographInformations();
        if (!info.isEmpty())
        {
            QString identify = i18n("Make: %1\n", info.make); 
            identify.append(i18n("Model: %1\n", info.model));

            if (info.dateTime.isValid())
                identify.append(i18n("Created: %1\n", KGlobal::locale()->formatDateTime(info.dateTime,
                                                                         KLocale::ShortDate, true)));

            identify.append(i18n("Aperture: f/%1\n", info.aperture));
            identify.append(i18n("Focal: %1 mm\n", info.focalLength));
            identify.append(i18n("Exposure: 1/%1 s\n", info.exposureTime));
            identify.append(i18n("Sensitivity: %1 ISO", info.sensitivity));

            d->infoLabel->setText(identify);
        }
    }
}

void ImageDialogPreview::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (KUrl(desc.filePath) == d->currentURL)
    {
        QPixmap pixmap;
        QSize s = d->imageLabel->contentsRect().size();

        if (s.width() < pix.width() || s.height() < pix.height())
            pixmap = pix.scaled(s, Qt::KeepAspectRatio);
        else 
            pixmap = pix;

        d->imageLabel->setPixmap(pixmap);
    }
}

void ImageDialogPreview::clearPreview()
{
    d->imageLabel->clear();
    d->currentURL = KUrl();
}

// ------------------------------------------------------------------------

class ImageDialogPrivate 
{

public:

    ImageDialogPrivate(){}

    QString    fileformats;

    KUrl::List urls;
};

ImageDialog::ImageDialog(QWidget* parent, const KUrl url)
{
    d = new ImageDialogPrivate;

    QStringList patternList = KImageIO::pattern(KImageIO::Reading).split('\n', QString::SkipEmptyParts);

    // All Images from list must been always the first entry given by KDE API
    QString allPictures = patternList[0];

    // Add other files format witch are missing to All Images" type mime provided by KDE and remplace current.
    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
    {
        allPictures.insert(allPictures.indexOf("|"), QString(KDcrawIface::DcrawBinary::instance()->rawFiles()) + QString(" *.JPE *.TIF"));
        patternList.removeAll(patternList[0]);
        patternList.prepend(allPictures);
    }

    // Added RAW file formats supported by dcraw program like a type mime. 
    // Nota: we cannot use here "image/x-raw" type mime from KDE because it uncomplete 
    // or unavailable(see file #121242 in B.K.O).
    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
        patternList.append(i18n("\n%1|Camera RAW files",QString(KDcrawIface::DcrawBinary::instance()->rawFiles())));

    d->fileformats = patternList.join("\n");

    DDebug() << "fileformats=" << d->fileformats << endl;

    KFileDialog dlg(url, d->fileformats, parent);
    ImageDialogPreview *preview = new ImageDialogPreview(&dlg);
    dlg.setPreviewWidget(preview);
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setMode(KFile::Files);
    dlg.setWindowTitle(i18n("Open Images"));
    dlg.exec();
    d->urls = dlg.selectedUrls();
}

ImageDialog::~ImageDialog() 
{
    delete d;
}

QString ImageDialog::fileformats() const 
{
    return d->fileformats;
}

KUrl::List ImageDialog::urls() const
{
    return d->urls;
}

KUrl::List ImageDialog::getImageURLs(QWidget* parent, const KUrl url)
{
    ImageDialog dlg(parent, url);
    if (!dlg.urls().isEmpty())
        return dlg.urls();
    else
        return KUrl::List();
}

} // namespace ShowFoto
