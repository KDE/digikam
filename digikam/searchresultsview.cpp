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

#include <QDataStream>
#include <QPixmap>

// KDE includes.

#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kfileitem.h>
#include <kurl.h>

// Local includes.

#include "albummanager.h"
#include "albumsettings.h"
#include "databaseurl.h"
#include "imagelister.h"
#include "searchresultsitem.h"
#include "searchresultsview.h"
#include "searchresultsview.moc"

namespace Digikam
{

SearchResultsView::SearchResultsView(QWidget* parent)
                 : Q3IconView(parent)
{
    m_listJob  = 0;
    m_thumbJob = 0;

    m_filter   = AlbumSettings::instance()->getAllFileFilter();

    setAutoArrange(true);
    setResizeMode(Q3IconView::Adjust);
}

SearchResultsView::~SearchResultsView()
{
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
    if (m_listJob)
        m_listJob->kill();
}

void SearchResultsView::openURL(const KUrl& url)
{
    if (m_listJob)
        m_listJob->kill();
    m_listJob = 0;

    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
    m_thumbJob = 0;

    m_listJob = ImageLister::startListJob(DatabaseUrl::fromSearchUrl(url),
                    //m_filter,
                    //false, // getting dimensions (not needed here)
                    1);    // miniListing (Use 0 for full listing)

    connect(m_listJob, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    connect(m_listJob, SIGNAL(data(KIO::Job*, const QByteArray&)),
            this, SLOT(slotData(KIO::Job*, const QByteArray&)));
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

        SearchResultsItem* existingItem = (SearchResultsItem*) m_itemDict.find(path);
        if (existingItem)
        {
            existingItem->m_marked = true;
            continue;
        }

        SearchResultsItem* item = new SearchResultsItem(this, path);
        m_itemDict.insert(path, item);

        ulist.append(KUrl(path));
    }

    SearchResultsItem* item = (SearchResultsItem*)firstItem();
    Q3IconViewItem* nextItem;
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

        connect(m_thumbJob, SIGNAL(signalThumbnail(const KUrl&, const QPixmap&)),
                this, SLOT(slotGotThumbnail(const KUrl&, const QPixmap&)));

        connect(m_thumbJob, SIGNAL(signalFailed(const KUrl&)),
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
    m_listJob = 0;
}

void SearchResultsView::slotGotThumbnail(const KUrl& url, const QPixmap& pix)
{
    Q3IconViewItem* i = m_itemDict.find(url.path());
    if (i)
        i->setPixmap(pix);
    
    m_thumbJob = 0;
}

void SearchResultsView::slotFailedThumbnail(const KUrl&)
{
    m_thumbJob = 0;    
}

}  // namespace Digikam
