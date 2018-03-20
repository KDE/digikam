/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thumbsgenerator.h"

// Qt includes

#include <QString>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "coredb.h"
#include "coredbalbuminfo.h"
#include "albummanager.h"
#include "applicationsettings.h"
#include "coredbaccess.h"
#include "imageinfo.h"
#include "thumbsdbaccess.h"
#include "thumbsdb.h"
#include "maintenancethread.h"
#include "digikam_config.h"

namespace Digikam
{

class ThumbsGenerator::Private
{
public:

    Private() :
        rebuildAll(true),
        thread(0)
    {
    }

    bool               rebuildAll;

    AlbumList          albumList;

    QStringList        allPicturesPath;

    MaintenanceThread* thread;
};

ThumbsGenerator::ThumbsGenerator(const bool rebuildAll, const AlbumList& list, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("ThumbsGenerator"), parent),
      d(new Private)
{
    d->albumList = list;
    init(rebuildAll);
}

ThumbsGenerator::ThumbsGenerator(const bool rebuildAll, int albumId, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("ThumbsGenerator"), parent),
      d(new Private)
{
    d->albumList.append(AlbumManager::instance()->findPAlbum(albumId));
    init(rebuildAll);
}

ThumbsGenerator::~ThumbsGenerator()
{
    delete d;
}

void ThumbsGenerator::init(const bool rebuildAll)
{
    setLabel(i18n("Thumbs"));
    ProgressManager::addProgressItem(this);

    d->rebuildAll = rebuildAll;
    d->thread     = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    connect(d->thread, SIGNAL(signalAdvance(QImage)),
            this, SLOT(slotAdvance(QImage)));
}

void ThumbsGenerator::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void ThumbsGenerator::slotCancel()
{
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

void ThumbsGenerator::slotStart()
{
    MaintenanceTool::slotStart();

    if (d->albumList.isEmpty())
    {
        d->albumList = AlbumManager::instance()->allPAlbums();
    }

    for (AlbumList::const_iterator it = d->albumList.constBegin();
         !canceled() && (it != d->albumList.constEnd()); ++it)
    {
        if (!(*it))
        {
            continue;
        }

        if ((*it)->type() == Album::PHYSICAL)
        {
            d->allPicturesPath += CoreDbAccess().db()->getItemURLsInAlbum((*it)->id());
        }
        else if ((*it)->type() == Album::TAG)
        {
            d->allPicturesPath += CoreDbAccess().db()->getItemURLsInTag((*it)->id());
        }
    }

    if (!d->rebuildAll)
    {
        QHash<QString, int> filePaths = ThumbsDbAccess().db()->getFilePathsWithThumbnail();
        QStringList::iterator it      = d->allPicturesPath.begin();

        while (it != d->allPicturesPath.end())
        {
            if (filePaths.contains(*it))
            {
                it = d->allPicturesPath.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // remove non-image or video files from the list
    QStringList::iterator it = d->allPicturesPath.begin();

    while (it != d->allPicturesPath.end())
    {
        ImageInfo info = ImageInfo::fromLocalFile(*it);

        if (info.category() != DatabaseItem::Image &&
            info.category() != DatabaseItem::Video &&
            info.category() != DatabaseItem::Audio)
        {
            it = d->allPicturesPath.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (d->allPicturesPath.isEmpty())
    {
        slotDone();
        return;
    }

    setTotalItems(d->allPicturesPath.count());

    d->thread->generateThumbs(d->allPicturesPath);
    d->thread->start();
}

void ThumbsGenerator::slotAdvance(const QImage& img)
{
    setThumbnail(QPixmap::fromImage(img));
    advance(1);
}

}  // namespace Digikam
