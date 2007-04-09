/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-20
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

// KDE includes.

#include <kio/job.h>
#include <kfileitem.h>
#include <kurl.h>

// Local includes.

#include "albummanager.h"
#include "albumsettings.h"
#include "databaseurl.h"
#include "imagelister.h"
#include "searchresultsitem.h"
#include "searchresultsview.h"

namespace Digikam
{

SearchResultsView::SearchResultsView(QWidget* parent)
    : QIconView(parent)
{
    m_listJob  = 0;
    m_thumbJob = 0;

    m_filter      = AlbumSettings::instance()->getAllFileFilter();

    setAutoArrange(true);
    setResizeMode(QIconView::Adjust);
}

SearchResultsView::~SearchResultsView()
{
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
    if (m_listJob)
        m_listJob->kill();
}

void SearchResultsView::openURL(const KURL& url)
{
    if (m_listJob)
        m_listJob->kill();
    m_listJob = 0;

    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
    m_thumbJob = 0;

    m_listJob = ImageLister::startListJob(DatabaseUrl::fromSearchUrl(url),
                                          m_filter,
                                          false, // getting dimensions (not needed here)
                                          1);    // miniListing (Use 0 for full listing)

    connect(m_listJob, SIGNAL(result(KIO::Job*)),
            SLOT(slotResult(KIO::Job*)));
    connect(m_listJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
}

void SearchResultsView::clear()
{
    if (m_listJob)
        m_listJob->kill();
    m_listJob = 0;

    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
    m_thumbJob = 0;

    m_itemDict.clear();
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
        ImageListerRecord record;
        ds >> record;

        // the record is not completely filled, we chose miniListing above. We only need the path.
        DatabaseUrl url = DatabaseUrl::fromAlbumAndName(record.name, record.albumName, record.albumRoot);
        path = url.path();

        SearchResultsItem* existingItem = (SearchResultsItem*) m_itemDict.find(path);
        if (existingItem)
        {
            existingItem->m_marked = true;
            continue;
        }

        SearchResultsItem* item = new SearchResultsItem(this, path);
        m_itemDict.insert(path, item);

        ulist.append(KURL(path));
    }

    SearchResultsItem* item = (SearchResultsItem*)firstItem();
    QIconViewItem* nextItem;
    while (item)
    {
        nextItem = item->nextItem();
        if (!item->m_marked)
        {
            m_itemDict.remove(item->m_path);
            delete item;
        }
        item = (SearchResultsItem*)nextItem;
    }
    arrangeItemsInGrid();


    if (!ulist.isEmpty())
    {
        m_thumbJob = new ThumbnailJob(ulist, 128, true, true);

        connect(m_thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                this, SLOT(slotGotThumbnail(const KURL&, const QPixmap&)));

        connect(m_thumbJob, SIGNAL(signalFailed(const KURL&)),
                this, SLOT(slotFailedThumbnail(const KURL&)));     
    }
}

void SearchResultsView::slotResult(KIO::Job *job)
{
    if (job->error())
        job->showErrorDialog(this);
    m_listJob = 0;
}


void SearchResultsView::slotGotThumbnail(const KURL& url, const QPixmap& pix)
{
    QIconViewItem* i = m_itemDict.find(url.path());
    if (i)
        i->setPixmap(pix);
    
    m_thumbJob = 0;
}

void SearchResultsView::slotFailedThumbnail(const KURL&)
{
    m_thumbJob = 0;    
}

}  // namespace Digikam

#include "searchresultsview.moc"

