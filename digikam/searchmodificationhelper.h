/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify search albums in views
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef SEARCHMODIFICATIONHELPER_H
#define SEARCHMODIFICATIONHELPER_H

// Qt includes

#include <qobject.h>

// Local inclues

#include "album.h"

namespace Digikam
{

/**
 * Range of a contiguous dates selection <start date, end date>.
 */
typedef QPair<QDateTime, QDateTime> DateRange;

/**
 * List of dates range selected.
 */
typedef QList<DateRange> DateRangeList;

class SearchModificationHelperPriv;

/**
 * Utility class providing methods to modify search albums (SAlbum) in a way
 * useful to implement views.
 *
 * @author jwienke
 */
class SearchModificationHelper: public QObject
{
Q_OBJECT
public:

    /**
     * Constructor.
     *
     * @param parent parent for qt parent child mechanism
     * @param dialogParent paret widget for dialogs displayed by this object
     */
    SearchModificationHelper(QObject *parent, QWidget *dialogParent);

    /**
     * Destructor.
     */
    virtual ~SearchModificationHelper();

public Q_SLOTS:

    /**
     * Deletes the given search after prompting the user.
     *
     * @param searchAlbum search to delete
     */
    void slotSearchDelete(SAlbum *searchAlbum);

    /**
     * Renames the given search via a dialog.
     *
     * @param searchAlbum search to rename
     */
    void slotSearchRename(SAlbum *searchAlbum);

    /**
     * Creates a new timeline search.
     *
     * @param desiredName desired name for the search. If this name already
     *                    exists and overwriteIfExisting is false, then the user
     *                    will be prompted for a new name
     * @param dateRanges date ranges to contain in this timeline search. If this
     *                        is empty, no search will be created.
     * @param overwriteIfExisting if true, an existing search with the desired
     *                            name will be overwritten without prompting the
     *                            user for a new name
     */
    void slotCreateTimeLineSearch(const QString &desiredName,
                                  const DateRangeList &dateRanges,
                                  bool overwriteIfExisting = false);

private:

    bool checkAlbum(const QString &name) const;
    bool checkName(QString &name);

private:
    SearchModificationHelperPriv *d;

};

}

#endif /* SEARCHMODIFICATIONHELPER_H */
