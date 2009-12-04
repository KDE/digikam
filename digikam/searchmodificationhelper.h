/*
 * SearchModificationHelper.h
 *
 *  Created on: 04.12.2009
 *      Author: languitar
 */

#ifndef SEARCHMODIFICATIONHELPER_H
#define SEARCHMODIFICATIONHELPER_H

// Qt includes

#include <qobject.h>

// Local inclues

#include "album.h"

namespace Digikam
{

typedef QPair<QDateTime, QDateTime> DateRange;    // Range of a contiguous dates selection <start date, end date>.
typedef QList<DateRange> DateRangeList;           // List of dates range selected.

class SearchModificationHelperPriv;

class SearchModificationHelper: public QObject
{
Q_OBJECT
public:
    SearchModificationHelper(QObject *parent, QWidget *dialogParent);
    virtual ~SearchModificationHelper();

public Q_SLOTS:

    void slotSearchDelete(SAlbum *searchAlbum);
    void slotSearchRename(SAlbum *searchAlbum);
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
