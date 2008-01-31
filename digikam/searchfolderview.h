/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : Searches folder view 
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SEARCHFOLDERVIEW_H
#define SEARCHFOLDERVIEW_H

// Local includes.

#include "folderview.h"

namespace Digikam
{

class SAlbum;
class SearchFolderItem;

class SearchFolderView : public FolderView
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
    
signals:

    void signalTextSearchFilterMatch(bool);

public slots:

    void slotTextSearchFilterChanged(const QString&);

private slots:

    void slotAlbumAdded(Album* album);
    void slotAlbumDeleted(Album* album);
    void slotSelectionChanged();
    void slotContextMenu(QListViewItem*, const QPoint&, int);
    void slotDoubleClicked(QListViewItem*, const QPoint&, int);
    
protected:
    
    void selectItem(int id);    
    
private:

    bool checkName( KURL& url );
    bool checkAlbum( const QString& name ) const;

private:

    SearchFolderItem* m_lastAddedItem;
};
    
}  // namespace Digikam

#endif /* SEARCHFOLDERVIEW_H */
