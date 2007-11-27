/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-20
 * Description : search results view.
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qdatastream.h>
#include <qdict.h>
#include <qguardedptr.h>

// KDE includes.

#include <kio/job.h>
#include <kfileitem.h>
#include <kurl.h>

// Local includes.

#include "thumbnailjob.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "searchresultsitem.h"
#include "searchresultsview.h"
#include "searchresultsview.moc"

namespace Digikam
{

class SearchResultsViewPriv
{
public:

    SearchResultsViewPriv()
    {
        listJob  = 0;
        thumbJob = 0;
    }

    QString                   libraryPath;
    QString                   filter;

    QDict<QIconViewItem>      itemDict;

    QGuardedPtr<ThumbnailJob> thumbJob;

    KIO::TransferJob*         listJob;
};

SearchResultsView::SearchResultsView(QWidget* parent)
                 : QIconView(parent)
{
    d = new SearchResultsViewPriv;
    d->libraryPath = AlbumManager::instance()->getLibraryPath();
    d->filter      = AlbumSettings::instance()->getAllFileFilter();

    setAutoArrange(true);
    setResizeMode(QIconView::Adjust);
}

SearchResultsView::~SearchResultsView()
{
    if (!d->thumbJob.isNull())
        d->thumbJob->kill();
    if (d->listJob)
        d->listJob->kill();

    delete d;
}

void SearchResultsView::openURL(const KURL& url)
{
    if (d->listJob)
        d->listJob->kill();
    d->listJob = 0;

    if (!d->thumbJob.isNull())
        d->thumbJob->kill();
    d->thumbJob = 0;
    
    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << d->libraryPath;
    ds << url;
    ds << d->filter;
    ds << 0; // getting dimensions (not needed here)
    ds << 0; // recursive sub-album (not needed here)
    ds << 0; // recursive sub-tags (not needed here)
    ds << 2; // miniListing (Use 1 for full listing)

    d->listJob = new KIO::TransferJob(url, KIO::CMD_SPECIAL,
                                      ba, QByteArray(), false);

    connect(d->listJob, SIGNAL(result(KIO::Job*)),
            this, SLOT(slotResult(KIO::Job*)));

    connect(d->listJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void SearchResultsView::clear()
{
    if (d->listJob)
        d->listJob->kill();
    d->listJob = 0;

    if (!d->thumbJob.isNull())
        d->thumbJob->kill();
    d->thumbJob = 0;

    d->itemDict.clear();
    QIconView::clear();
}

void SearchResultsView::slotData(KIO::Job*, const QByteArray &data)
{
    for (QIconViewItem* item = firstItem(); item; item = item->nextItem())
        ((SearchResultsItem*)item)->m_marked = false;

    KURL::List ulist;

    QString path;
    QDataStream ds(data, IO_ReadOnly);
    while (!ds.atEnd())
    {
        ds >> path;

        SearchResultsItem* existingItem = (SearchResultsItem*) d->itemDict.find(path);
        if (existingItem)
        {
            existingItem->m_marked = true;
            continue;
        }
            
        SearchResultsItem* item = new SearchResultsItem(this, path);
        d->itemDict.insert(path, item);

        ulist.append(KURL(path));
    }

    SearchResultsItem* item = (SearchResultsItem*)firstItem();
    QIconViewItem* nextItem;
    while (item)
    {
        nextItem = item->nextItem();
        if (!item->m_marked)
        {
            d->itemDict.remove(item->m_path);
            delete item;
        }
        item = (SearchResultsItem*)nextItem;
    }
    arrangeItemsInGrid();
   
    bool match = !ulist.isEmpty();

    emit signalSearchResultsMatch(match);
    
    if (match)
    {
        d->thumbJob = new ThumbnailJob(ulist, 128, true, true);
    
        connect(d->thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                this, SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));
   
        connect(d->thumbJob, SIGNAL(signalFailed(const KURL&)),
                this, SLOT(slotFailedThumbnail(const KURL&)));     
    }
}

void SearchResultsView::slotResult(KIO::Job *job)
{
    if (job->error())
        job->showErrorDialog(this);
    d->listJob = 0;
}

void SearchResultsView::slotGotThumbnail(const KURL& url, const QPixmap& pix)
{
    QIconViewItem* i = d->itemDict.find(url.path());
    if (i)
        i->setPixmap(pix);
    
    d->thumbJob = 0;
}

void SearchResultsView::slotFailedThumbnail(const KURL&)
{
    d->thumbJob = 0;    
}

}  // namespace Digikam
