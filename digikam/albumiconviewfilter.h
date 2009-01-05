/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to filter album contents
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMICONVIEWFILTER_H
#define ALBUMICONVIEWFILTER_H

// Qt includes.

#include "qhbox.h"
#include "qstring.h"

// Local includes.

#include "albumlister.h"

class QEvent;
class QObject;

namespace Digikam
{

class AlbumIconViewFilterPriv;

class AlbumIconViewFilter : public QHBox
{
    Q_OBJECT

public:

    AlbumIconViewFilter(QWidget* parent);
    ~AlbumIconViewFilter();

    void readSettings();
    void saveSettings();

signals:

    void signalResetTagFilters();

private slots:

    void slotRatingFilterChanged(int, AlbumLister::RatingCondition);
    void slotMimeTypeFilterChanged(int);
    void slotTextFilterChanged(const QString&);
    void slotItemsFilterMatch(bool);

private:

    bool eventFilter(QObject *object, QEvent *e);

private:

    AlbumIconViewFilterPriv* d;
};

}  // namespace Digikam

#endif // ALBUMICONVIEWFILTER_H
