/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Find Duplicates View.
 *
 * Copyright (C) 2008-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef FINDDUPLICATESVIEW_H
#define FINDDUPLICATESVIEW_H

// Qt includes

#include <QWidget>

class QTreeWidgetItem;

namespace Digikam
{
class Album;
class SAlbum;

class FindDuplicatesView : public QWidget
{
    Q_OBJECT

public:

    explicit FindDuplicatesView(QWidget* const parent = 0);
    ~FindDuplicatesView();

    SAlbum* currentFindDuplicatesAlbum() const;

public Q_SLOTS:

    void slotSetSelectedAlbum(Album*);
    void slotSetSelectedTag(Album*);

private Q_SLOTS:

    void populateTreeView();
    void slotAlbumAdded(Album* a);
    void slotAlbumDeleted(Album* a);
    void slotSearchUpdated(SAlbum* a);
    void slotClear();
    void slotFindDuplicates();
    void slotDuplicatesAlbumActived(QTreeWidgetItem*, int);
    void slotComplete();
    void slotUpdateFingerPrints();
    void slotCheckForValidSettings();

private:

    void enableControlWidgets(bool);

    void updateAlbumsBox();
    void updateTagsBox();

    void resetAlbumsAndTags();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* FINDDUPLICATESVIEW_H */
