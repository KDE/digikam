/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-20
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <qdatastream.h>

#include <kio/job.h>
#include <kio/previewjob.h>
#include <kfileitem.h>
#include <kurl.h>

#include "albummanager.h"
#include "albumsettings.h"
#include "searchresultsitem.h"
#include "searchresultsview.h"

SearchResultsView::SearchResultsView(QWidget* parent)
    : QIconView(parent)
{
    m_listJob   = 0;
    m_previewJob = 0;

    m_libraryPath = AlbumManager::instance()->getLibraryPath();
    m_filter      = AlbumSettings::instance()->getAllFileFilter();

    setAutoArrange(true);
    setResizeMode(QIconView::Adjust);
}

SearchResultsView::~SearchResultsView()
{
    if (m_previewJob)
        m_previewJob->kill();
    if (m_listJob)
        m_listJob->kill();
}

void SearchResultsView::openURL(const KURL& url)
{
    if (m_listJob)
        m_listJob->kill();
    m_listJob = 0;

    if (m_previewJob)
        m_previewJob->kill();
    m_previewJob = 0;
    
    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << m_libraryPath;
    ds << url;
    ds << m_filter;
    ds << 0; // getting dimensions (not needed here)
    ds << 0; // recursive tags (not needed here)
    ds << 2; // miniListing (Use 1 for full listing)

    m_listJob = new KIO::TransferJob(url, KIO::CMD_SPECIAL,
                                     ba, QByteArray(), false);
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

    if (m_previewJob)
        m_previewJob->kill();
    m_previewJob = 0;

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
        ds >> path;

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
        m_previewJob = KIO::filePreview(ulist, 128);
        connect(m_previewJob, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)),
                SLOT(slotPreview(const KFileItem*, const QPixmap&)));
        connect(m_previewJob, SIGNAL(failed (const KFileItem*)),
                SLOT(slotFailed(const KFileItem*)));
    }
}

void SearchResultsView::slotResult(KIO::Job *job)
{
    if (job->error())
        job->showErrorDialog(this);
    m_listJob = 0;
}

void SearchResultsView::slotPreview(const KFileItem* item, const QPixmap& pix)
{
    QIconViewItem* i = m_itemDict.find(item->url().path());
    if (i)
        i->setPixmap(pix);
    
    m_previewJob = 0;
}

void SearchResultsView::slotFailed(const KFileItem*)
{
    m_previewJob = 0;    
}

#include "searchresultsview.moc"

