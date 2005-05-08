/* ============================================================
 * File  : albumfolderview.h
 * Author: Jörn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-06
 * Copyright 2005 by Jörn Ahrens
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

/** @file albumfoldeview.h */

#ifndef _ALBUMFOLDERVIEW_H_
#define _ALBUMFOLDERVIEW_H_

#include <qlistview.h>

class KURL;
class QPixmap;
class KFileMetaInfo;

class Album;
class PAlbum;
class AlbumFolderViewPriv;

class AlbumFolderView : public QListView
{
    Q_OBJECT

public:

    AlbumFolderView(QWidget *parent);
    ~AlbumFolderView();

    void setActive(bool val);
    
private slots:

    void    slotAlbumAdded(Album *);
    void    slotGotThumbnailFromIcon(const KURL& url, const QPixmap& thumbnail,
                                     const KFileMetaInfo*);
    void    slotSelectionChanged();
    
private:

    void    setAlbumThumbnail(PAlbum *album);
    
    AlbumFolderViewPriv   *d;
};


#endif // _ALBUMFOLDEVIEW_H_
