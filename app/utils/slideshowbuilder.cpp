/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-24
 * Description : slideshow builder progress indicator
 *
 * Copyright (C) 2012-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slideshowbuilder.moc"

// Qt includes

#include <QTimer>

// KDE includes

#include <kapplication.h>
#include <klocale.h>
#include <kicon.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "imageinfoalbumsjob.h"
#include "applicationsettings.h"

namespace Digikam
{

class SlideShowBuilder::Private
{
public:

    Private() :
        cancel(false),
        album(0)
    {
    }

    bool          cancel;
    ImageInfoList infoList;
    Album*        album;
};

SlideShowBuilder::SlideShowBuilder(const ImageInfoList& infoList)
    : ProgressItem(0, "SlideShowBuilder", QString(), QString(), true, true),
      d(new Private)
{
    d->infoList = infoList;

    ProgressManager::addProgressItem(this);

    QTimer::singleShot(500, this, SLOT(slotRun()));
}

SlideShowBuilder::SlideShowBuilder(Album* const album)
    : ProgressItem(0, "SlideShowBuilder", QString(), QString(), true, true),
      d(new Private)
{
    d->album = album;

    ProgressManager::addProgressItem(this);

    QTimer::singleShot(500, this, SLOT(slotRun()));
}

SlideShowBuilder::~SlideShowBuilder()
{
    delete d;
}

void SlideShowBuilder::slotRun()
{
    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

    setLabel(i18n("Preparing slideshow"));
    setThumbnail(KIcon("digikam").pixmap(22));

    if (d->album)
    {
        AlbumList albumList;
        albumList.append(d->album);
        AlbumIterator it(d->album);

        while (it.current())
        {
            albumList.append(*it);
            ++it;
        }

        ImageInfoAlbumsJob* const job = new ImageInfoAlbumsJob;

        connect(job, SIGNAL(signalCompleted(ImageInfoList)),
                this, SLOT(slotParseImageInfoList(ImageInfoList)));

        job->allItemsFromAlbums(albumList);
    }
    else
    {
        slotParseImageInfoList(d->infoList);
    }
}

void SlideShowBuilder::slotParseImageInfoList(const ImageInfoList& list)
{
    setTotalItems(list.count());

    int               i = 0;
    SlideShowSettings settings;
    settings.readFromConfig();
    settings.previewSettings = ApplicationSettings::instance()->getPreviewSettings();

    for (ImageInfoList::const_iterator it = list.constBegin();
         !d->cancel && (it != list.constEnd()) ; ++it)
    {
        ImageInfo info       = *it;
        settings.fileList.append(info.fileUrl());
        SlidePictureInfo pictInfo;
        pictInfo.comment     = info.comment();
        pictInfo.title       = info.title();
        pictInfo.rating      = info.rating();
        pictInfo.colorLabel  = info.colorLabel();
        pictInfo.pickLabel   = info.pickLabel();
        pictInfo.photoInfo   = info.photoInfoContainer();
        pictInfo.tags        = AlbumManager::instance()->tagNames(info.tagIds());
        pictInfo.tags.sort();

        settings.pictInfoMap.insert(info.fileUrl(), pictInfo);

        advance(i++);
        kapp->processEvents();
    }

    if (!d->cancel)
        emit signalComplete(settings);

    setComplete();
}

void SlideShowBuilder::slotCancel()
{
    d->cancel = true;
}

}  // namespace Digikam
