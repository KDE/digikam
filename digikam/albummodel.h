/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-22
 * Description : Qt Model for Albums
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ALBUMMODEL_H
#define ALBUMMODEL_H

// Qt includes.

#include <QAbstractItemModel>
#include <QHash>

// Local includes.

#include "album.h"

namespace Digikam
{

class Album;
class AlbumManager;
class AlbumModelPriv;

class AbstractAlbumModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    /**
     *  AbstractAlbumModel is the abstract base class for all models that
     *  present Album objects as managed by AlbumManager.
     *  You will want to create an instance of the base classes.
     */

    enum RootAlbumBehavior
    {
        /** The root album will be included as a single parent item
            with all top-level album as children */
        IncludeRootAlbum,
        /** The root album will not be included, but all top-level album
            are represented as top-level items in this view */
        IgnoreRootAlbum
    };

    enum AlbumDataRole
    {
        /// Returns the Album::Type of the associated album
        AlbumTypeRole = Qt::UserRole,
        /// Returns a pointer to the associated Album object
        AlbumPointerRole = Qt::UserRole + 1
    };

    /**
     * Create an AbstractAlbumModel object for albums with the given type.
     * Pass the root album if it is already available.
     * Do not use this class directly, but one of the subclasses.
     */
    AbstractAlbumModel(Album::Type albumType, Album *rootAlbum, RootAlbumBehavior rootBehavior = IncludeRootAlbum,
                       QObject *parent = 0);
    ~AbstractAlbumModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QMimeData * mimeData(const QModelIndexList &indexes) const;

protected:

    // these can be reimplemented in a subclass
    virtual QVariant decorationRole(Album *a) const;
    virtual QString columnHeader() const;
    virtual Qt::ItemFlags itemFlags(Album *album) const;
    virtual bool filterAlbum(Album *album) const;

    QModelIndex indexForAlbum(Album *album) const;
    Album *albumForIndex(const QModelIndex &index) const;
    Album *rootAlbum() const;

protected Q_SLOTS:

    void slotAlbumAboutToBeAdded(Album *album, Album *parent, Album *prev);
    void slotAlbumAdded(Album *);
    void slotAlbumAboutToBeDeleted(Album *album);
    void slotAlbumHasBeenDeleted(void *);
    void slotAlbumsCleared();
    void slotAlbumIconChanged(Album* album);
    void slotAlbumRenamed(Album *album);

private:

    AlbumModelPriv* const d;
};

// ------------------------------------------------------------------

class AbstractSpecificAlbumModel : public AbstractAlbumModel
{
    Q_OBJECT

public:

    /// Abstract base class, do not instantiate.
    AbstractSpecificAlbumModel(Album::Type albumType, Album *rootAlbum,
                               RootAlbumBehavior rootBehavior = IncludeRootAlbum,
                               QObject *parent = 0);

protected:

    virtual QVariant decorationRole(Album *a) const = 0;
    virtual QString  columnHeader() const;

protected Q_SLOTS:

    void slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail);
    void slotThumbnailLost(Album *album);
    void slotReloadThumbnails();

protected:

    void emitDataChangedForChildren(Album *album);

    QString m_columnHeader;
};

// ------------------------------------------------------------------

class AbstractCheckableAlbumModel : public AbstractSpecificAlbumModel
{
    Q_OBJECT

public:

    /// Abstract base class that manages the check state of Albums.
    /// Call setCheckable(true) to enable checkable albums.

    AbstractCheckableAlbumModel(Album::Type albumType, Album *rootAlbum,
                                RootAlbumBehavior rootBehavior = IncludeRootAlbum,
                                QObject *parent = 0);

    /// Triggers if the albums in this model are checkable
    void setCheckable(bool isCheckable);
    bool isCheckable() const;
    /** Triggers if the albums in this model are tristate.
     *  Note that you want to set setCheckable(true) before. */
    void setTristate(bool isTristate);
    bool isTristate() const;

    /// Returns if the given album has the check state Checked
    bool isChecked(Album *album) const;
    /// Returns the check state of the album
    Qt::CheckState checkState(Album *album) const;

    /// Sets the check state of album to Checked or Unchecked
    void setChecked(Album *album, bool isChecked);
    /// Sets the check state of the album
    void setCheckState(Album *album, Qt::CheckState state);

    /// Returns a list of album with check state Checked
    QList<Album *> checkedAlbums() const;

    /// Resets the checked state of all albums to Qt::Unchecked
    void resetCheckedAlbums();

Q_SIGNALS:

    /** Emitted when the check state of an album changes.
     *  checkState contains the new Qt::CheckState of album */
    void checkStateChanged(Album *album, int checkState);

protected:

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

private:

    Qt::ItemFlags                 m_extraFlags;
    QHash<Album*, Qt::CheckState> m_checkedAlbums;
};

// ------------------------------------------------------------------

class AlbumModel : public AbstractCheckableAlbumModel
{
public:

    /// Create a model containing all physical albums
    AlbumModel(RootAlbumBehavior rootBehavior = IncludeRootAlbum, QObject *parent = 0);

protected:

    virtual QVariant decorationRole(Album *a) const;
};

// ------------------------------------------------------------------

class TagModel : public AbstractCheckableAlbumModel
{
public:

    /// Create a model containing all tags
    TagModel(RootAlbumBehavior rootBehavior = IncludeRootAlbum, QObject *parent = 0);

protected:

    virtual QVariant decorationRole(Album *a) const;
};

} // namespace Digikam

#endif // ALBUMMODEL_H
