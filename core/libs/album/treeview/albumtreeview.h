/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ALBUM_TREE_VIEW_H
#define DIGIKAM_ALBUM_TREE_VIEW_H

// Qt includes

#include <QTreeView>

// Local includes

#include "abstractalbumtreeview.h"
#include "abstractcountingalbumtreeview.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "albumpointer.h"
#include "statesavingobject.h"

namespace Digikam
{

class AbstractCheckableAlbumTreeView : public AbstractCountingAlbumTreeView
{
    Q_OBJECT

public:

    /// Models of these view _can_ be checkable, they need _not_. You need to enable it on the model.

    explicit AbstractCheckableAlbumTreeView(QWidget* const parent, Flags flags);

    virtual ~AbstractCheckableAlbumTreeView();

    /// Manage check state through the model directly
    AbstractCheckableAlbumModel* albumModel()     const;
    CheckableAlbumFilterModel* albumFilterModel() const;

    AbstractCheckableAlbumModel* checkableModel() const
    {
        return albumModel();
    }

    CheckableAlbumFilterModel* checkableAlbumFilterModel() const
    {
        return albumFilterModel();
    }

    /// Enable checking on middle mouse button click (default: on)
    void setCheckOnMiddleClick(bool doThat);

    /**
     * Tells if the check state is restored while loading / saving state.
     *
     * @return true if restoring check state is active
     */
    bool isRestoreCheckState() const;

    /**
     * Set whether to restore check state or not.
     *
     * @param restore if true, restore check state
     */
    void setRestoreCheckState(bool restore);

    virtual void doLoadState();
    virtual void doSaveState();

protected:

    virtual void middleButtonPressed(Album* a);
    virtual void rowsInserted(const QModelIndex& parent, int start, int end);

private:

    void restoreCheckStateForHierarchy(const QModelIndex& index);
    void restoreCheckState(const QModelIndex& index);

private:

    class Private;
    Private* d;
};

// -------------------------------------------------------------------------------------

class AlbumTreeView : public AbstractCheckableAlbumTreeView
{
    Q_OBJECT

public:

    explicit AlbumTreeView(QWidget* const parent = 0, Flags flags = DefaultFlags);
    virtual ~AlbumTreeView();

    AlbumModel* albumModel()                        const;
    PAlbum* currentAlbum()                          const;
    PAlbum* albumForIndex(const QModelIndex& index) const;

    void setAlbumFilterModel(CheckableAlbumFilterModel* const filterModel);
    void setAlbumModel(AlbumModel* const model);

public Q_SLOTS:

    void setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager = true);
    void setCurrentAlbum(int albumId, bool selectInAlbumManager = true);
};

// -------------------------------------------------------------------------------------

class TagTreeView : public AbstractCheckableAlbumTreeView
{
    Q_OBJECT

public:

    explicit TagTreeView(QWidget* const parent = 0, Flags flags = DefaultFlags);
    ~TagTreeView();

    TagModel* albumModel() const;

    /// Contains only the tags filtered by properties - prefer to albumModel()
    TagPropertiesFilterModel* filteredModel() const;

    /**
     * @brief currentAlbum     - even if multiple selection is enabled current
     *                           Album can be only one, the last clicked item
     *                          if you need selected items, see selectedAlbums()
     *                          It's NOT the same as AlbumManager::currentAlbums()
     */
    TAlbum* currentAlbum() const;

    /**
     * @brief selectedTags - return a list of all selected items in tag model
     */
    QList<Album*> selectedTags();
    QList<TAlbum*> selectedTagAlbums();

    TAlbum* albumForIndex(const QModelIndex& index) const;
    TagModificationHelper* tagModificationHelper()  const;

    void setAlbumFilterModel(TagPropertiesFilterModel* const filteredModel, CheckableAlbumFilterModel* const filterModel);
    void setAlbumModel(TagModel* const model);

public Q_SLOTS:

    void setCurrentAlbums(const QList<Album*>& tags, bool selectInAlbumManager = true);
    void setCurrentAlbum(int tagId, bool selectInAlbumManager = true);

Q_SIGNALS:

    void assignTags(int tagId, const QList<int>& imageIDs);

protected:

    TagPropertiesFilterModel* m_filteredModel;
    TagModificationHelper*    m_modificationHelper;
};

// -------------------------------------------------------------------------------------

class SearchTreeView : public AbstractCheckableAlbumTreeView
{
    Q_OBJECT

public:

    explicit SearchTreeView(QWidget* const parent = 0, Flags flags = DefaultFlags);
    ~SearchTreeView();

    /// Note: not filtered by search type
    SearchModel* albumModel()          const;

    /// Contains only the searches with appropriate type - prefer to albumModel()
    SearchFilterModel* filteredModel() const;
    SAlbum* currentAlbum()             const;

    void setAlbumModel(SearchModel* const model);
    void setAlbumFilterModel(SearchFilterModel* const filteredModel, CheckableAlbumFilterModel* const model);

public Q_SLOTS:

    void setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager = true);
    void setCurrentAlbum(int searchId, bool selectInAlbumManager = true);

protected:

    SearchFilterModel* m_filteredModel;
};

// -------------------------------------------------------------------------------------

class DateAlbumTreeView : public AbstractCountingAlbumTreeView
{
    Q_OBJECT

public:

    explicit DateAlbumTreeView(QWidget* const parent = 0, Flags flags = DefaultFlags);

    DateAlbumModel* albumModel()                    const;
    DAlbum* currentAlbum()                          const;
    DAlbum* albumForIndex(const QModelIndex& index) const;

    void setAlbumModel(DateAlbumModel* const model);
    void setAlbumFilterModel(AlbumFilterModel* const filterModel);

public Q_SLOTS:

    void setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager = true);
    void setCurrentAlbum(int dateId, bool selectInAlbumManager = true);
};

} // namespace Digikam

#endif // DIGIKAM_ALBUM_TREE_VIEW_H
