/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : fingerprints generator
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "fingerprintsgenerator.moc"

// Qt includes

#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

// KDE includes

#include <kapplication.h>
#include <kcodecs.h>
#include <klocale.h>
#include <kstandardguiitem.h>

// Local includes

#include "dimg.h"
#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "haar.h"
#include "haariface.h"
#include "previewloadthread.h"
#include "knotificationwrapper.h"
#include "metadatasettings.h"

namespace Digikam
{

class FingerPrintsGenerator::FingerPrintsGeneratorPriv
{
public:

    FingerPrintsGeneratorPriv() :
        cancel(false),
        rebuildAll(true),
        previewLoadThread(0)
    {
        duration.start();
    }

    bool               cancel;
    bool               rebuildAll;

    QTime              duration;

    QStringList        allPicturesPath;

    PreviewLoadThread* previewLoadThread;

    HaarIface          haarIface;
};

FingerPrintsGenerator::FingerPrintsGenerator(bool rebuildAll)
    : ProgressItem(0, "FingerPrintsGenerator", QString(), QString(), true, true),
      d(new FingerPrintsGeneratorPriv)
{
    ProgressManager::addProgressItem(this);

    d->rebuildAll        = rebuildAll;
    d->previewLoadThread = new PreviewLoadThread();

    connect(d->previewLoadThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription,DImg)));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

    setLabel(i18n("Finger-prints"));

    QTimer::singleShot(500, this, SLOT(slotRebuildFingerPrints()));
}

FingerPrintsGenerator::~FingerPrintsGenerator()
{
    delete d;
}

void FingerPrintsGenerator::slotRebuildFingerPrints()
{
    const AlbumList palbumList = AlbumManager::instance()->allPAlbums();

    // Get all digiKam albums collection pictures path, depending of d->rebuildAll flag.

    if (d->rebuildAll)
    {
        for (AlbumList::ConstIterator it = palbumList.constBegin();
             !d->cancel && (it != palbumList.constEnd()); ++it)
        {
            d->allPicturesPath += DatabaseAccess().db()->getItemURLsInAlbum((*it)->id());
        }
    }
    else
    {
        d->allPicturesPath = DatabaseAccess().db()->getDirtyOrMissingFingerprintURLs();
    }

    if (d->allPicturesPath.isEmpty())
    {
        slotCancel();
        return;
    }

    setTotalItems(d->allPicturesPath.count());
    processOne();
}

void FingerPrintsGenerator::processOne()
{
    if (d->cancel)
    {
        return;
    }

    QString path = d->allPicturesPath.first();
    LoadingDescription description(path, HaarIface::preferredSize(), LoadingDescription::ConvertToSRGB);
    description.rawDecodingSettings.rawPrm.sixteenBitsImage = false;
    d->previewLoadThread->load(description);
}

void FingerPrintsGenerator::slotGotImagePreview(const LoadingDescription& desc, const DImg& img)
{
    if (d->allPicturesPath.isEmpty())
    {
        return;
    }

    if (d->allPicturesPath.first() != desc.filePath)
    {
        return;
    }

    if (!img.isNull())
    {
        // compute Haar fingerprint
        d->haarIface.indexImage(desc.filePath, img);
    }

    QPixmap pix = DImg(img).smoothScale(22, 22, Qt::KeepAspectRatio).convertToPixmap();
    setThumbnail(pix);
    advance(1);

    if (!d->allPicturesPath.isEmpty())
    {
        d->allPicturesPath.removeFirst();
    }

    if (d->allPicturesPath.isEmpty())
    {
        complete();
    }
    else
    {
        processOne();
    }
}

void FingerPrintsGenerator::complete()
{
    QTime now, t = now.addMSecs(d->duration.elapsed());
    // Pop-up a message to bring user when all is done.
    KNotificationWrapper(id(),
                         i18n("Finger-prints generation is done.\nDuration: %1", t.toString()),
                         kapp->activeWindow(), label());

    emit signalComplete();

    setComplete();
}

void FingerPrintsGenerator::slotCancel()
{
    d->cancel = true;
    emit signalComplete();
    setComplete();
}

}  // namespace Digikam
