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

#include <qlabel.h>
#include <qlayout.h>
#include <qguardedptr.h>
#include <qtimer.h>

// KDE includes.

#include <kapplication.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kiconloader.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000106
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "thumbnailsize.h"
#include "thumbnailjob.h"
#include "imagedialog.h"
#include "imagedialog.moc"

namespace Digikam
{

class ImageDialogPreviewPrivate 
{

public:

    ImageDialogPreviewPrivate()
    {
        imageLabel = 0;
        infoLabel  = 0;
        thumbJob   = 0;
        timer      = 0;
    }

    QTimer                    *timer;

    QLabel                    *imageLabel;
    QLabel                    *infoLabel;

    KURL                       currentURL;

    DMetadata                  metaIface;

    QGuardedPtr<ThumbnailJob>  thumbJob;
};

ImageDialogPreview::ImageDialogPreview(QWidget *parent)
                  : KPreviewWidgetBase(parent)
{
    d = new ImageDialogPreviewPrivate;

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

    d->timer = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()), 
            this, SLOT(showPreview()) );
}

ImageDialogPreview::~ImageDialogPreview() 
{
    if (!d->thumbJob.isNull())
    {
        d->thumbJob->kill();
        d->thumbJob = 0;
    }
    delete d;
}

QSize ImageDialogPreview::sizeHint() const
{
    return QSize(256, 256);
}

void ImageDialogPreview::resizeEvent(QResizeEvent *)
{
    d->timer->start(100, true);
}

void ImageDialogPreview::showPreview()
{
    KURL url(d->currentURL);
    clearPreview();
    showPreview(url);
}

void ImageDialogPreview::showPreview(const KURL& url)
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

        if (!d->thumbJob.isNull())
        {
        d->thumbJob->kill();
        d->thumbJob = 0;
        }

        d->thumbJob = new ThumbnailJob(url, ThumbnailSize::Huge, true, true);

        connect(d->thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                this, SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));

        connect(d->thumbJob, SIGNAL(signalFailed(const KURL&)),
                this, SLOT(slotFailedThumbnail(const KURL&)));

        d->metaIface.load(d->currentURL.path());
        PhotoInfoContainer info = d->metaIface.getPhotographInformations();
        if (!info.isEmpty())
        {
            QString identify;
            QString make, model, dateTime, aperture, focalLength, exposureTime, sensitivity;
            QString unavailable(i18n("<i>unavailable</i>"));
            QString cellBeg("<tr><td><nobr><font size=-1>");
            QString cellMid("</font></nobr></td><td><nobr><font size=-1>");
            QString cellEnd("</font></nobr></td></tr>");

            if (info.make.isEmpty()) make = unavailable;
            else make = info.make;

            if (info.model.isEmpty()) model = unavailable;
            else model = info.model;

            if (!info.dateTime.isValid()) dateTime = unavailable;
            else dateTime = KGlobal::locale()->formatDateTime(info.dateTime, true, true);

            if (info.aperture.isEmpty()) aperture = unavailable; 
            else aperture = info.aperture;

            if (info.focalLength.isEmpty()) focalLength = unavailable; 
            else focalLength = info.focalLength;

            if (info.exposureTime.isEmpty()) exposureTime = unavailable; 
            else exposureTime = info.exposureTime;

            if (info.sensitivity.isEmpty()) sensitivity = unavailable; 
            else sensitivity = i18n("%1 ISO").arg(info.sensitivity);

            identify = "<table cellspacing=0 cellpadding=0>";
            identify += cellBeg + i18n("Make:")        + cellMid + make         + cellEnd;
            identify += cellBeg + i18n("Model:")       + cellMid + model        + cellEnd;
            identify += cellBeg + i18n("Created:")     + cellMid + dateTime     + cellEnd;
            identify += cellBeg + i18n("Aperture:")    + cellMid + aperture     + cellEnd;
            identify += cellBeg + i18n("Focal:")       + cellMid + focalLength  + cellEnd;
            identify += cellBeg + i18n("Exposure:")    + cellMid + exposureTime + cellEnd;
            identify += cellBeg + i18n("Sensitivity:") + cellMid + sensitivity  + cellEnd;
            identify += "</table>";

            d->infoLabel->setText(identify);
        }
        else
            d->infoLabel->clear();
    }
}

void ImageDialogPreview::slotGotThumbnail(const KURL& url, const QPixmap& pix)
{
    if (url == d->currentURL)
    {
        QPixmap pixmap;
        QSize s = d->imageLabel->contentsRect().size();

        if (s.width() < pix.width() || s.height() < pix.height())
            pixmap = pix.convertToImage().smoothScale(s, QImage::ScaleMin);
        else 
            pixmap = pix;

        d->imageLabel->setPixmap(pixmap);
    }
}

void ImageDialogPreview::slotFailedThumbnail(const KURL& /*url*/)
{
    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    d->imageLabel->setPixmap(iconLoader->loadIcon("image", KIcon::NoGroup, 128,
                             KIcon::DefaultState, 0, true));
}

void ImageDialogPreview::clearPreview()
{
    d->imageLabel->clear();
    d->infoLabel->clear();
    d->currentURL = KURL();
}

// ------------------------------------------------------------------------

class ImageDialogPrivate 
{

public:

    ImageDialogPrivate()
    {
        singleSelect = false;
    }

    bool       singleSelect;

    QString    fileformats;

    KURL       url;
    KURL::List urls;
};

ImageDialog::ImageDialog(QWidget* parent, const KURL &url, bool singleSelect, const QString& caption)
{
    d = new ImageDialogPrivate;
    d->singleSelect = singleSelect;

    QStringList patternList = QStringList::split('\n', KImageIO::pattern(KImageIO::Reading));

    // All Images from list must been always the first entry given by KDE API
    QString allPictures = patternList[0];

#if KDCRAW_VERSION < 0x000106
    // Add other files format witch are missing to All Images" type mime provided by KDE and remplace current.
    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
    {
        allPictures.insert(allPictures.find("|"), QString(KDcrawIface::DcrawBinary::instance()->rawFiles()) + QString(" *.JPE *.TIF"));
        patternList.remove(patternList[0]);
        patternList.prepend(allPictures);
        // Added RAW file formats supported by dcraw program like a type mime. 
        // Nota: we cannot use here "image/x-raw" type mime from KDE because it uncomplete 
        // or unavailable (see file #121242 in B.K.O).
        patternList.append(i18n("\n%1|Camera RAW files").arg(QString(KDcrawIface::DcrawBinary::instance()->rawFiles())));
    }
#else
    allPictures.insert(allPictures.find("|"), QString(KDcrawIface::KDcraw::rawFiles()) + QString(" *.JPE *.TIF"));
    patternList.remove(patternList[0]);
    patternList.prepend(allPictures);
    // Added RAW file formats supported by dcraw program like a type mime. 
    // Nota: we cannot use here "image/x-raw" type mime from KDE because it uncomplete 
    // or unavailable (see file #121242 in B.K.O).
    patternList.append(i18n("\n%1|Camera RAW files").arg(QString(KDcrawIface::KDcraw::rawFiles())));
#endif

    d->fileformats = patternList.join("\n");

    DDebug() << "fileformats=" << d->fileformats << endl;

    KFileDialog dlg(url.path(), d->fileformats, parent, "imageFileOpenDialog", false);
    ImageDialogPreview *preview = new ImageDialogPreview(&dlg);
    dlg.setPreviewWidget(preview);
    dlg.setOperationMode(KFileDialog::Opening);

    if (d->singleSelect)
    {
        dlg.setMode(KFile::File);
        if (caption.isEmpty()) dlg.setCaption(i18n("Select an Image"));
        else dlg.setCaption(caption);
        dlg.exec();
        d->url = dlg.selectedURL();
    }
    else
    {
        dlg.setMode(KFile::Files);
        if (caption.isEmpty()) dlg.setCaption(i18n("Select Images"));
        else dlg.setCaption(caption);
        dlg.exec();
        d->urls = dlg.selectedURLs();
    }
}

ImageDialog::~ImageDialog() 
{
    delete d;
}

bool ImageDialog::singleSelect() const 
{
    return d->singleSelect;
}

QString ImageDialog::fileformats() const 
{
    return d->fileformats;
}

KURL ImageDialog::url() const
{
    return d->url;
}

KURL::List ImageDialog::urls() const
{
    return d->urls;
}

KURL::List ImageDialog::getImageURLs(QWidget* parent, const KURL& url, const QString& caption)
{
    ImageDialog dlg(parent, url, false, caption);
    if (!dlg.urls().isEmpty())
        return dlg.urls();
    else
        return KURL::List();
}

KURL ImageDialog::getImageURL(QWidget* parent, const KURL& url, const QString& caption)
{
    ImageDialog dlg(parent, url, true, caption);
    if (dlg.url() != KURL())
        return dlg.url();
    else
        return KURL();
}

} // namespace Digikam
