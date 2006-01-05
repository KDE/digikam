/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-05
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
 *
 * ============================================================ */

#ifndef TAGFILTERVIEW_H
#define TAGFILTERVIEW_H

// Local includes.

#include "folderview.h"

namespace Digikam
{

class Album;
class TagFilterViewItem;
class TagFilterViewPriv;

class TagFilterView : public FolderView
{
    Q_OBJECT

public:

    TagFilterView(QWidget* parent);
    ~TagFilterView();

    void triggerChange();

protected:

    QDragObject* dragObject();
    bool acceptDrop(const QDropEvent *e) const;
    void contentsDropEvent(QDropEvent *e);

signals:

    void signalTagsAssigned();
    
private slots:

    void slotTagAdded(Album* album);
    void slotTagMoved(TAlbum* tag, TAlbum* newParent);
    void slotTagRenamed(Album* album);
    void slotTagDeleted(Album* album);
    void slotClear();
    void slotTimeOut();
    void slotContextMenu(QListViewItem*, const QPoint&, int);

private:

    void tagNew(TagFilterViewItem* item);
    void tagEdit(TagFilterViewItem* item);    
    void tagDelete(TagFilterViewItem* item);
    
    TagFilterViewPriv *d;
};

}  // namespace Digikam

#endif /* TAGFILTERVIEW_H */
