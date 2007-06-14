/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-20
 * Description : search results view.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef SEARCHRESULTSVIEW_H
#define SEARCHRESULTSVIEW_H

// Qt includes.

#include <q3iconview.h>
#include <q3cstring.h>
#include <q3dict.h>
#include <qpointer.h>
//Added by qt3to4:
#include <QPixmap>

// Local includes.

#include "thumbnailjob.h"

class QPixmap;
class KFileItem;

namespace KIO
{
class TransferJob;
class Job;
}

namespace Digikam
{

class SearchResultsView : public Q3IconView
{
    Q_OBJECT

public:

    SearchResultsView(QWidget* parent);
    ~SearchResultsView();

    void openURL(const KURL& url);
    void clear();

private:

    KIO::TransferJob*         m_listJob;

    QPointer<ThumbnailJob> m_thumbJob;

    Q3Dict<Q3IconViewItem>      m_itemDict;

    QString                   m_filter;

private slots:

    void slotData(KIO::Job *job, const QByteArray &data);
    void slotResult(KIO::Job *job);
    void slotGotThumbnail(const KURL& url, const QPixmap& pix);
    void slotFailedThumbnail(const KURL& url);
};

}  // namespace Digikam

#endif /* SEARCHRESULTSVIEW_H */
