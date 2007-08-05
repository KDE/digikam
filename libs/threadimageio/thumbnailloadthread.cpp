/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading
 *
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QPainter>

// KDE includes

#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "thumbnailtask.h"
#include "thumbnailcreator.h"
#include "thumbnailloadthread.h"
#include "thumbnailloadthread.moc"

namespace Digikam
{

class ThumbnailLoadThreadPriv
{
public:

    ThumbnailLoadThreadPriv()
    {
        highlight     = true;
        sendSurrogate = true;
        creator       = 0;
    }

    ThumbnailCreator *creator;
    bool highlight;
    bool sendSurrogate;
};

ThumbnailLoadThread::ThumbnailLoadThread()
{
    d = new ThumbnailLoadThreadPriv;

    d->creator = new ThumbnailCreator();
    //d->creator->setOnlyLargeThumbnails(true);
    d->creator->setRemoveAlphaChannel(true);
}

ThumbnailLoadThread::~ThumbnailLoadThread()
{
    delete d;
}

void ThumbnailLoadThread::setPixmapReqested(bool wantPixmap)
{
    if (wantPixmap)
        connect(this, SIGNAL(thumbnailLoaded(const LoadingDescription &, const QImage&)),
                this, SLOT(slotThumbnailLoaded(const LoadingDescription &, const QImage&)));
    else
        disconnect(this, SIGNAL(thumbnailLoaded(const LoadingDescription &, const QImage&)),
                   this, SLOT(slotThumbnailLoaded(const LoadingDescription &, const QImage&)));
}

void ThumbnailLoadThread::setHighlightPixmap(bool highlight)
{
    d->highlight = highlight;
}

ThumbnailCreator *ThumbnailLoadThread::thumbnailCreator() const
{
    return d->creator;
}

void ThumbnailLoadThread::load(LoadingDescription description)
{
    if (description.previewParameters.size <= 0)
    {
        DError() << "ThumbnailLoadThread::load: No thumbnail size specified. Refusing to load thumbnail." << endl;
        return;
    }
    else if (description.previewParameters.size > 256)
    {
        DError() << "ThumbnailLoadThread::load: Thumbnail size " << description.previewParameters.size
                 << " is larger than 256. Refusing to load." << endl;
        return;
    }

    description.previewParameters.type = LoadingDescription::PreviewParameters::Thumbnail;
    ManagedLoadSaveThread::loadPreview(description);
}

void ThumbnailLoadThread::slotThumbnailLoaded(const LoadingDescription &loadingDescription, const QImage& thumb)
{
    if (thumb.isNull())
    {
        if (d->sendSurrogate)
            sendSurrogatePixmap(loadingDescription);
        else
            emit thumbnailLoaded(loadingDescription, QPixmap());
    }

    QPixmap pix = QPixmap::fromImage(thumb);

    int w = pix.width();
    int h = pix.height();

    // highlight only when requested and when thumbnail
    // width and height are greater than 10
    if (d->highlight && (w >= 10 && h >= 10))
    {
        QPainter p(&pix);
        p.setPen(QPen(Qt::black, 1));
        p.drawRect(0, 0, w - 1, h - 1);
    }

    emit thumbnailLoaded(loadingDescription, pix);
}

void ThumbnailLoadThread::sendSurrogatePixmap(const LoadingDescription &description)
{
    QPixmap pix;

    KMimeType::Ptr mimeType = KMimeType::findByPath(description.filePath);
    if (mimeType)
    {
        pix = DesktopIcon(mimeType->iconName(), K3Icon::SizeEnormous);
    }

    /*
    No dependency on AlbumSettings here please...
    QString ext = QFileInfo(url.path()).suffix();

    AlbumSettings* settings = AlbumSettings::componentData();
    if (settings)
    {
        if (settings->getImageFileFilter().toUpper().contains(ext.toUpper()) ||
            settings->getRawFileFilter().toUpper().contains(ext.toUpper()))
        {
            pix = DesktopIcon("image", K3Icon::SizeEnormous);
        }
        else if (settings->getMovieFileFilter().toUpper().contains(ext.toUpper()))
        {
            pix = DesktopIcon("video", K3Icon::SizeEnormous);
        }
        else if (settings->getAudioFileFilter().toUpper().contains(ext.toUpper()))
        {
            pix = DesktopIcon("sound", K3Icon::SizeEnormous);
        }
    }
    */

    if (pix.isNull())
        pix = DesktopIcon("file-broken", K3Icon::SizeEnormous);

    // Resize icon to the right size depending of current settings.

    QSize size(pix.size());
    size.scale(description.previewParameters.size, description.previewParameters.size, Qt::KeepAspectRatio);
    if (!pix.isNull() && size.width() < pix.width() && size.height() < pix.height())
    {
        // only scale down
        // do not scale up, looks bad
        pix = pix.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    //Note/TODO: Currently not cached
    emit thumbnailLoaded(description, pix);
}



}   // namespace Digikam
