/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-06
 * Description : Albums folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef ALBUMFOLDERVIEW_H
#define ALBUMFOLDERVIEW_H

// QT includes
#include <qtreeview.h>

// Local includes
#include "albummodel.h"
#include "albumtreeview.h"
#include "albummodificationhelper.h"

namespace Digikam {

class AlbumFolderViewNewPriv;

/**
 * Basic album view based on a AlbumTreeView.
 *
 * @author jwienke
 */
class AlbumFolderViewNew: public AlbumTreeView
{
Q_OBJECT
public:
    AlbumFolderViewNew(QWidget *parent, AlbumModificationHelper *albumModificationHelper);
    ~AlbumFolderViewNew();

    /**
     * Returns the album on that the last context menu was triggered.
     *
     * @return album for which the last context menu was triggered or null if it
     *         wasn't triggered on a real album.
     */
    PAlbum *lastContextMenuAlbum() const;

Q_SIGNALS:

    /**
     * Emitted if a find duplicates search shall be invoked on the given album.
     *
     * @param album the album to find duplicates in
     */
    void signalFindDuplicatesInAlbum(Album *album);

public Q_SLOTS:

    /**
     * Selects the given album.
     *
     * @param album album to select
     */
    void slotSelectAlbum(Album *album);

private:

    /**
     * Creates the context menu.
     *
     * @param event event that requested the menu
     */
    void contextMenuEvent(QContextMenuEvent *event);

    /**
     * Re-implemented to handle custom tool tips.
     *
     * @param event event to process.
     */
    bool viewportEvent(QEvent *event);

private Q_SLOTS:
    void slotAlbumSelected(const QModelIndex &index);

private:
    AlbumFolderViewNewPriv *d;

};

}  // namespace Digikam

#endif // ALBUMFOLDERVIEW_H
