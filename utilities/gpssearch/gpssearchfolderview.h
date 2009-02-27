/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-30
 * Description : GPS search folder view
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GPSSEARCHFOLDERVIEW_H
#define GPSSEARCHFOLDERVIEW_H

// Local includes.

#include "searchtextbar.h"
#include "folderview.h"

namespace Digikam
{

class Album;
class SAlbum;
class GPSSearchFolderItem;

class GPSSearchFolderView : public FolderView
{
    Q_OBJECT

public:

    GPSSearchFolderView(QWidget* parent);
    ~GPSSearchFolderView();

    void searchDelete(SAlbum* album);
    static QString currentGPSSearchName();

Q_SIGNALS:

    void signalTextSearchFilterMatch(bool);
    void signalAlbumSelected(SAlbum*);
    void signalRenameAlbum(SAlbum*);

public Q_SLOTS:

    void slotTextSearchFilterChanged(const SearchTextSettings&);

private Q_SLOTS:

    void slotAlbumAdded(Album*);
    void slotAlbumDeleted(Album*);
    void slotAlbumRenamed(Album*);
    void slotAlbumCurrentChanged(Album*);
    void slotSelectionChanged();
    void slotContextMenu(Q3ListViewItem*, const QPoint&, int);

protected:

    void selectItem(int id);
};

}  // namespace Digikam

#endif /* GPSSEARCHFOLDERVIEW_H */
