/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify physical albums in views
 *
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef ALBUMMODIFICATIONHELPER_H
#define ALBUMMODIFICATIONHELPER_H

// Qt includes

#include <QObject>
#include <QWidget>

// KDE includes

#include <kjob.h>

// Local includes

#include "album.h"

namespace Digikam
{

/**
 * Utility class providing methods to modify physical albums (PAlbum) in a way
 * useful to implement views.
 *
 * @author jwienke
 */
class AlbumModificationHelper : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent parent for qt parent child mechanism
     * @param dialogParent parent widget for dialogs displayed by this object
     */
    AlbumModificationHelper(QObject* parent, QWidget* dialogParent);

    /**
     * Destructor.
     */
    virtual ~AlbumModificationHelper();

public Q_SLOTS:

    /**
     * Creates a new album under the given parent. The user will be prompted for
     * the settings of the new album.
     *
     * @param parentAlbum parent album for the new one
     * @return the new album or 0 if no album was created
     */
    PAlbum* slotAlbumNew(PAlbum* parentAlbum);

    /**
     * Deletes the given album after waiting for a graphical confirmation of the
     * user.
     *
     * @param album album to delete
     */
    void slotAlbumDelete(PAlbum* album);

    /**
     * Renames the given album. The user will be prompted for a new name.
     *
     * @param album album to rename
     */
    void slotAlbumRename(PAlbum* album);

    /**
     * Graphically edits the properties of the given album.
     *
     * @param album album to edit
     */
    void slotAlbumEdit(PAlbum* album);

private Q_SLOTS:

    void slotDIOResult(KJob* kjob);

private:

    void addAlbumChildrenToList(KUrl::List& list, Album* album);

private:

    class AlbumModificationHelperPriv;
    AlbumModificationHelperPriv* const d;

};

} // namespace Digikam

#endif /* ALBUMMODIFICATIONHELPER_H */
