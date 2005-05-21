/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-21
 * Copyright 2005 by Renchi Raju
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
 * ============================================================ */

#ifndef SEARCHFOLDERVIEW_H
#define SEARCHFOLDERVIEW_H

#include <klistview.h>

class SAlbum;
class SearchFolderItem;

class SearchFolderView : public KListView
{
    Q_OBJECT

public:

    SearchFolderView(QWidget* parent);
    ~SearchFolderView();

    void quickSearchNew();
    void extendedSearchNew();

    void quickSearchEdit(SAlbum* album);
    void extendedSearchEdit(SAlbum* album);

    void searchDelete(SAlbum* album);
    
    void setActive(bool val);

private slots:

    void slotAlbumAdded(Album* album);
    void slotAlbumDeleted(Album* album);
    void slotSelectionChanged();
    void slotContextMenu(QListViewItem*, const QPoint&, int);
    
private:

    bool              m_active;
    SearchFolderItem* m_lastAddedItem;
};
    

#endif /* SEARCHFOLDERVIEW_H */
