/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-26
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju
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

#ifndef ALBUMLISTER_H
#define ALBUMLISTER_H

/** @file albumlister.h */

// Qt includes.

#include <qobject.h>
#include <qcstring.h>

// Local includes.

#include "imageinfo.h"

namespace KIO
{
class Job;
}

namespace Digikam
{

class Album;
class AlbumListerPriv;

/**
 * Manages imageinfo
 *
 * does listing of imageinfo for the current album and controls the lifetime 
 * of the imageinfo. kioslaves are used for listing the imageinfo
 * corresponding to an album. Similar to the albummanager, frontend entities need
 * to connect to the AlbumLister for notifications of new Images, deletion of
 * Images or refreshing of currently listed Image.
 */
class AlbumLister : public QObject
{
    Q_OBJECT

public:

    /** @enum MatchingCondition
     * Possible logical matching condition used to sort tags id.
     */
    enum MatchingCondition
    {
        OrCondition = 0,
        AndCondition
    };

public:

    static AlbumLister* instance();
    
    ~AlbumLister();

    /**
     * Opens an album to lists its items
     */
    void openAlbum(Album *album);
    void stop();
    
    /**
     * Reread an albums item list
     */    
    void refresh();

    void setNameFilter(const QString& nameFilter);

    void setDayFilter(const QValueList<int>& days);
    void setTagFilter(const QValueList<int>& tags, const MatchingCondition& matchingCond, 
                      bool showUnTagged=false);
    
signals:

    void signalNewItems(const ImageInfoList& items);
    void signalDeleteItem(ImageInfo* item);
    void signalNewFilteredItems(const ImageInfoList& items);
    void signalDeleteFilteredItem(ImageInfo* item);
    void signalClear();
    void signalCompleted();

private slots:

    void slotClear();
    void slotFilterItems();

    void slotResult(KIO::Job* job);
    void slotData(KIO::Job* job, const QByteArray& data);

private:

    AlbumLister();
    bool matchesFilter(const ImageInfo* info) const;
    
private:

    AlbumListerPriv    *d;

    static AlbumLister *m_instance; 

};

}  // namespace Digikam

#endif /* ALBUMLISTER_H */
