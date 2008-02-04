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

#include <Q3Dict>
#include <QDataStream>
#include <QPixmap>

// KDE includes.

#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kfileitem.h>

// Local includes.

#include "thumbnailjob.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "databaseurl.h"
#include "imagelister.h"
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

    QString                filter;

    Q3Dict<Q3IconViewItem> itemDict;

    QPointer<ThumbnailJob> thumbJob;

    KIO::TransferJob*      listJob;
};

SearchResultsView::SearchResultsView(QWidget* parent)
                 : Q3IconView(parent)
{
    d = new SearchResultsViewPriv;
    d->filter = AlbumSettings::instance()->getAllFileFilter();

    setAutoArrange(true);
    setResizeMode(Q3IconView::Adjust);
}

SearchResultsView::~SearchResultsView()
{
    if (!d->thumbJob.isNull())
        d->thumbJob->kill();
    if (d->listJob)
        d->listJob->kill();

    delete d;
}

void SearchResultsView::openURL(const KUrl& url)
{
    /*
    if (d->listJob)
        d->listJob->kill();
    d->listJob = 0;

    if (!d->thumbJob.isNull())
        d->thumbJob->kill();
    d->thumbJob = 0;

    d->listJob = ImageLister::startListJob(DatabaseUrl::fromSearchUrl(url),
                    //d->filter,
                    //false, // getting dimensions (not needed here)
                    1);    // miniListing (Use 0 for full listing)

    connect(d->listJob, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    connect(d->listJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
    */
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
    Q3IconView::clear();
}

void SearchResultsView::slotData(KIO::Job*, const QByteArray &data)
{
    for (Q3IconViewItem* item = firstItem(); item; item = item->nextItem())
        ((SearchResultsItem*)item)->m_marked = false;

    KUrl::List ulist;

    QString path;
    QDataStream ds(data);
    while (!ds.atEnd())
    {
        ImageListerRecord record;
        ds >> record;

        ImageInfo info(record);
        path = info.filePath();

        SearchResultsItem* existingItem = (SearchResultsItem*) d->itemDict.find(path);
        if (existingItem)
        {
            existingItem->m_marked = true;
            continue;
        }

        SearchResultsItem* item = new SearchResultsItem(this, path);
        d->itemDict.insert(path, item);

        ulist.append(KUrl(path));
    }

    SearchResultsItem* item = (SearchResultsItem*)firstItem();
    Q3IconViewItem* nextItem;
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

        connect(d->thumbJob, SIGNAL(signalThumbnail(const KUrl&, const QPixmap&)),
                this, SLOT(slotGotThumbnail(const KUrl&, const QPixmap&)));

        connect(d->thumbJob, SIGNAL(signalFailed(const KUrl&)),
                this, SLOT(slotFailedThumbnail(const KUrl&)));     
    }
}

void SearchResultsView::slotResult(KJob *kjob)
{
    KIO::Job *job = static_cast<KIO::Job*>(kjob);
    if (job->error())
    {
        job->ui()->setWindow(this);
        job->ui()->showErrorMessage();
    }
    d->listJob = 0;
}

void SearchResultsView::slotGotThumbnail(const KUrl& url, const QPixmap& pix)
{
    Q3IconViewItem* i = d->itemDict.find(url.path());
    if (i)
        i->setPixmap(pix);
    
    d->thumbJob = 0;
}

void SearchResultsView::slotFailedThumbnail(const KUrl&)
{
    d->thumbJob = 0;    
}

}  // namespace Digikam
