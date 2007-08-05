/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Thumbnail loading
 *
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef THUMBNAIL_LOAD_THREAD_H
#define THUMBNAIL_LOAD_THREAD_H

// Qt includes

#include <QPixmap>

// Local includes.

#include "managedloadsavethread.h"

namespace Digikam
{

class ThumbnailCreator;
class ThumbnailLoadThreadPriv;

class DIGIKAM_EXPORT ThumbnailLoadThread : public ManagedLoadSaveThread
{
    Q_OBJECT

public:

    ThumbnailLoadThread();
    ~ThumbnailLoadThread();

    /**
     * Load a thumbnail.
     * The LoadingDescription shall be constructed with the constructor for preview/thumbnail jobs.
     */
    void load(LoadingDescription description);

    /**
     * If you enable this, the signal thumbnailLoaded(LoadingDescription, QPixmap) will be emitted.
     * If you do not enable this, only the QImage-based signal (see LoadSaveThread) will be emitted.
     * Default value: disabled.
     */
    void setPixmapReqested(bool wantPixmap);
    /**
     * If you enable this, a highlighting border will be drawn around the pixmap.
     * This option has only an effect if pixmapRequested is true.
     * Default value: Enabled.
     */
    void setHighlightPixmap(bool highlight);
    /**
     * If you enable this, the thread will try hard to send a pixmap if thumbnail loading failed.
     * It will use standard system icons to replace the real thumbnail.
     * If you disable this, a null QPixmap will be sent.
     * This does not influence the QImage-based signal; this signal will be emitted with a null
     * QImage regardless of this setting here, if the loading failed.
     * Default value: Enabled.
     */
    void setSendSurrogatePixmap(bool send);

signals:

    // See LoadSaveThread for a QImage-based thumbnailLoaded() signal.

    void thumbnailLoaded(const LoadingDescription &loadingDescription, const QPixmap& pix);

public:

    // For internal use - may only be used from the thread
    ThumbnailCreator *thumbnailCreator() const;

private:

    void sendSurrogatePixmap(const LoadingDescription &loadingDescription);

private slots:

    void slotThumbnailLoaded(const LoadingDescription &loadingDescription, const QImage& thumb);

private:

    ThumbnailLoadThreadPriv *d;
};

}   // namespace Digikam

#endif // SHARED_LOAD_SAVE_THREAD_H
