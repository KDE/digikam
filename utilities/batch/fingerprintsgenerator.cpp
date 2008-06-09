/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : finger-prints generator
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

// QT includes.

#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QPointer>
#include <QCloseEvent>

// KDE includes.

#include <kcodecs.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandardguiitem.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "haariface.h"
#include "previewloadthread.h"
#include "fingerprintsgenerator.h"
#include "fingerprintsgenerator.moc"

namespace Digikam
{

class FingerPrintsGeneratorPriv
{
public:

    FingerPrintsGeneratorPriv()
    {
        cancel            = false;
        previewLoadThread = 0;
        duration.start();
    }

    bool               cancel;

    QTime              duration;

    QStringList        allPicturesPath;

    PreviewLoadThread *previewLoadThread;

    HaarIface          haarIface;
};

FingerPrintsGenerator::FingerPrintsGenerator(QWidget* parent)
                     : DProgressDlg(parent)
{
    d = new FingerPrintsGeneratorPriv;
    d->previewLoadThread = new PreviewLoadThread();

    connect(d->previewLoadThread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
            this, SLOT(slotGotImagePreview(const LoadingDescription&, const DImg&)));

    setValue(0);
    setCaption(i18n("Rebuild All Finger-Prints"));
    setLabel(i18n("<b>Updating finger-prints database in progress. Please wait...</b>"));
    setButtonText(i18n("&Abort"));
    resize(600, 300);

    QTimer::singleShot(500, this, SLOT(slotRebuildFingerPrints()));
}

FingerPrintsGenerator::~FingerPrintsGenerator()
{
    delete d;
}

void FingerPrintsGenerator::slotRebuildFingerPrints()
{
    setTitle(i18n("Processing..."));
    AlbumList palbumList = AlbumManager::instance()->allPAlbums();

    // Get all digiKam albums collection pictures path.

    DatabaseAccess access;

    for (AlbumList::Iterator it = palbumList.begin();
         !d->cancel && (it != palbumList.end()); ++it)
    {
        QStringList albumItemsPath;
        {
            albumItemsPath = access.db()->getItemURLsInAlbum((*it)->id());
        }
        d->allPicturesPath += albumItemsPath;
    }

    setMaximum(d->allPicturesPath.count());

    if(d->allPicturesPath.isEmpty())
    {
       slotCancel();
       return;
    }

    processOne();
}

void FingerPrintsGenerator::processOne()
{
    if (d->cancel) return;
    QString path = d->allPicturesPath.first();
    d->previewLoadThread->load(LoadingDescription(path, HaarIface::preferredSize(),
                               AlbumSettings::instance()->getExifRotate()));
}

void FingerPrintsGenerator::complete()
{
    QTime t;
    t = t.addMSecs(d->duration.elapsed());
    setLabel(i18n("<b>Update of finger-prints database done</b>"));
    setTitle(i18n("Duration: %1", t.toString()));
    setButtonGuiItem(KStandardGuiItem::ok());
    setButtonText(i18n("&Close"));
    emit signalRebuildAllFingerPrintsDone();
}

void FingerPrintsGenerator::slotGotImagePreview(const LoadingDescription& desc, const DImg& img)
{
    if (!img.isNull())
    {
        // compute Haar fingerprint
        d->haarIface.indexImage(desc.filePath, img);
    }
    QPixmap pix = DImg(img).smoothScale(128, 128, Qt::KeepAspectRatio).convertToPixmap();
    addedAction(pix, desc.filePath);
    advance(1);
    d->allPicturesPath.removeFirst();
    if (d->allPicturesPath.isEmpty())
        complete();
    else
        processOne();
}

void FingerPrintsGenerator::slotCancel()
{
    abort();
    done(Cancel);
}

void FingerPrintsGenerator::closeEvent(QCloseEvent *e)
{
    abort();
    e->accept();
}

void FingerPrintsGenerator::abort()
{
    d->cancel = true;
    emit signalRebuildAllFingerPrintsDone();
}

}  // namespace Digikam
