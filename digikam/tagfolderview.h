/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : tags folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern.ahrens@kdemail.net>
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

#ifndef TAGFOLDERVIEW_H
#define TAGFOLDERVIEW_H

// Qt includes
#include <qtreeview.h>

// Local includes
#include "albumtreeview.h"
#include "tagmodificationhelper.h"

namespace Digikam
{

class TagFolderViewNewPriv;
class TagFolderViewNew: public TagTreeView
{
Q_OBJECT
public:
    TagFolderViewNew(QWidget *parent, TagModel *model,
                     TagModificationHelper *tagModificationHelper);
    virtual ~TagFolderViewNew();

Q_SIGNALS:
    void signalFindDuplicatesInAlbum(Album*);

public Q_SLOTS:
    void slotSelectAlbum(Album *album);

private Q_SLOTS:
    void slotTagSelected(const QModelIndex&);

private:

    /**
     * Creates the context menu.
     *
     * @param event event that requested the menu
     */
    void contextMenuEvent(QContextMenuEvent *event);

private:
    TagFolderViewNewPriv *d;

};

}

#endif // TAGFOLDERVIEW_H
