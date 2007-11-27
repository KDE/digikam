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

#ifndef SEARCHRESULTSVIEW_H
#define SEARCHRESULTSVIEW_H

// Qt includes.

#include <qiconview.h>
#include <qcstring.h>
#include <qdict.h>
#include <qguardedptr.h>

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

class SearchResultsView : public QIconView
{
    Q_OBJECT
    
public:

    SearchResultsView(QWidget* parent);
    ~SearchResultsView();

    void openURL(const KURL& url);
    void clear();

signals:

    void signalSearchResultsMatch(bool);
    
private slots:

    void slotData(KIO::Job *job, const QByteArray &data);
    void slotResult(KIO::Job *job);
    void slotGotThumbnail(const KURL& url, const QPixmap& pix);
    void slotFailedThumbnail(const KURL& url);

private:
    
    QString                   m_libraryPath;
    QString                   m_filter;

    QDict<QIconViewItem>      m_itemDict;

    QGuardedPtr<ThumbnailJob> m_thumbJob;

    KIO::TransferJob*         m_listJob;
};

}  // namespace Digikam

#endif /* SEARCHRESULTSVIEW_H */
