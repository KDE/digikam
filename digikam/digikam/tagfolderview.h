/* ============================================================
 * File  : tagfolderview.h
 * Author: Jörn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-03-22
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

/** @file tagfoldeview.h */

#ifndef _TAGFOLDERVIEW_H_
#define _TAGFOLDERVIEW_H_

#include <qlistview.h>

class Album;
class TagFolderViewPriv;

class TagFolderView : public QListView
{
    Q_OBJECT
public:
    TagFolderView(QWidget *parent);
    
private slots:
    void    slotAlbumAdded(Album *);

private:
    TagFolderViewPriv   *d;
};


#endif // _TAGFOLDEVIEW_H_
