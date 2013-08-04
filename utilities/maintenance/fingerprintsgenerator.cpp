/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : fingerprints generator
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
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

// KDE includes

#include <kcodecs.h>
#include <klocale.h>
#include <kconfig.h>
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
#include "metadatasettings.h"

namespace Digikam
{

class FingerPrintsGenerator::Private
{
public:

    Private() :
        rebuildAll(true),
        previewLoadThread(0)
    {
    }

    bool               rebuildAll;

    QStringList        allPicturesPath;

    PreviewLoadThread* previewLoadThread;

    HaarIface          haarIface;
};

FingerPrintsGenerator::FingerPrintsGenerator(const bool rebuildAll, ProgressItem* const parent)
    : MaintenanceTool("FingerPrintsGenerator", parent),
      d(new Private)
{
    setLabel(i18n("Finger-prints"));
    ProgressManager::addProgressItem(this);

    d->rebuildAll        = rebuildAll;
    d->previewLoadThread = new PreviewLoadThread();

    connect(d->previewLoadThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription,DImg)));
}

FingerPrintsGenerator::~FingerPrintsGenerator()
{
    delete d;
}

void FingerPrintsGenerator::slotStart()
{
    MaintenanceTool::slotStart();

    const AlbumList palbumList = AlbumManager::instance()->allPAlbums();

    // Get all digiKam albums collection pictures path, depending of d->rebuildAll flag.

    if (d->rebuildAll)
    {
        for (AlbumList::ConstIterator it = palbumList.constBegin();
             !canceled() && (it != palbumList.constEnd()); ++it)
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
        slotDone();
        return;
    }

    setTotalItems(d->allPicturesPath.count());
    processOne();
}

void FingerPrintsGenerator::processOne()
{
    if (canceled())
    {
        slotCancel();
        return;
    }

    if (d->allPicturesPath.isEmpty())
    {
        slotDone();
        return;
    }

    QString path                                            = d->allPicturesPath.first();
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
        slotDone();
    }
    else
    {
        processOne();
    }
}

void FingerPrintsGenerator::slotDone()
{
    // Switch on scanned for finger-prints flag on digiKam config file.
    KGlobal::config()->group("General Settings").writeEntry("Finger Prints Generator First Run", true);

    MaintenanceTool::slotDone();
}

}  // namespace Digikam
