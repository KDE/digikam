/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-20
 * Description : search results view.
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QHasht>
#include <QDataStream>
#include <QPixmap>

// KDE includes.

#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kfileitem.h>

// Local includes.

#include "albummanager.h"
#include "albumsettings.h"
#include "databaseurl.h"
#include "imagelister.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
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
        listJob         = 0;
        thumbLoadThread = 0;
    }

    QString                       filter;

    QHash<KUrl, Q3IconViewItem*>  itemHash;

    ThumbnailLoadThread          *thumbLoadThread;

    KIO::TransferJob*             listJob;
};

SearchResultsView::SearchResultsView(QWidget* parent)
                 : Q3IconView(parent)
{
    d = new SearchResultsViewPriv;
    d->filter = AlbumSettings::instance()->getAllFileFilter();

    setAutoArrange(true);
    setResizeMode(Q3IconView::Adjust);

    d->thumbLoadThread = new ThumbnailLoadThread();
    d->thumbLoadThread->setThumbnailSize(128);
    d->thumbLoadThread->setSendSurrogatePixmap(true);
    d->thumbLoadThread->setExifRotate(true);

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotGotThumbnail(const LoadingDescription&, const QPixmap&)));
}

SearchResultsView::~SearchResultsView()
{
    delete d->thumbLoadThread;

    if (d->listJob)
        d->listJob->kill();

    // Delete all hash items 
    while (!d->itemHash.isEmpty()) 
    {
        Q3IconViewItem *value = *d->itemHash.begin();
        d->itemHash.erase(d->itemHash.begin());
        delete value;
    }

    delete d;
}

void SearchResultsView::openURL(const KUrl& url)
{
    /*
    if (d->listJob)
        d->listJob->kill();
    d->listJob = 0;

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

    // Delete all hash items 
    while (!d->itemHash.isEmpty()) 
    {
        Q3IconViewItem *value = *d->itemHash.begin();
        d->itemHash.erase(d->itemHash.begin());
        delete value;
    }

    Q3IconView::clear();
}

void SearchResultsView::slotData(KIO::Job*, const QByteArray &data)
{
    for (Q3IconViewItem* item = firstItem(); item; item = item->nextItem())
        (dynamic_cast<SearchResultsItem*>(item))->m_marked = false;

    KUrl::List ulist;

    KUrl url;
    QDataStream ds(data);
    while (!ds.atEnd())
    {
        ImageListerRecord record;
        ds >> record;

        ImageInfo info(record);
        url = info.fileUrl();

        SearchResultsItem* existingItem = dynamic_cast<SearchResultsItem*>(*d->itemHash.find(url));
        if (existingItem)
        {
            existingItem->m_marked = true;
            continue;
        }

        SearchResultsItem* item = new SearchResultsItem(this, url);
        d->itemHash.insert(url, item);

        ulist.append(url);
    }

    SearchResultsItem* item = dynamic_cast<SearchResultsItem*>(firstItem());
    Q3IconViewItem* nextItem;
    while (item)
    {
        nextItem = item->nextItem();
        if (!item->m_marked)
        {
            d->itemHash.remove(item->m_url);
            delete item;
        }
        item = dynamic_cast<SearchResultsItem*>(nextItem);
    }
    arrangeItemsInGrid();

    bool match = !ulist.isEmpty();

    emit signalSearchResultsMatch(match);

    if (match)
    {
        for (KUrl::List::const_iterator it = ulist.begin() ; it != ulist.end() ; ++it)
            d->thumbLoadThread->find((*it).path());
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

void SearchResultsView::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    QHash<KUrl, Q3IconViewItem*>::const_iterator it = d->itemHash.find(KUrl(desc.filePath));
    if (it == d->itemHash.end())
        return;

    (*it)->setPixmap(pix);
}

}  // namespace Digikam
