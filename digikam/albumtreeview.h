/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ALBUMTREEVIEW_H
#define ALBUMTREEVIEW_H

// Qt includes

#include <QTreeView>

// KDE includes
#include <kconfiggroup.h>

// Local includes

#include "albummodel.h"
#include "albumfiltermodel.h"
#include "statesavingobject.h"

namespace Digikam
{

class ContextMenuHelper;
class TagModificationHelper;

class AbstractAlbumTreeViewPriv;

/**
 * Base class for all tree views that display Album-based content provided by an
 * AbstractSpecificAlbumModel. This class enables various utility functions like
 * selecting albums on mouse actions or providing an infrastructure for
 * displaying a context menu for albums.
 *
 * Context menu handling is implemented as template methods with hook methods
 * that can be implemented by subclasses to provide a custom behaviour. In
 * default mode no context menu is shown at all. It must be enabled via a call
 * to setEnableContextMenu.
 */
class AbstractAlbumTreeView : public QTreeView, public StateSavingObject
{
    Q_OBJECT

public:

    /// Constructs an album model. If you supply 0 for filterModel, call setAlbumFilterModel afterwards.
    explicit AbstractAlbumTreeView(AbstractSpecificAlbumModel *model,
                                   AlbumFilterModel *filterModel,
                                   QWidget *parent = 0);
    ~AbstractAlbumTreeView();

    AbstractSpecificAlbumModel *albumModel() const;
    AlbumFilterModel *albumFilterModel() const;

    /// Enable expanding of tree items on single click on the item (default: on)
    void setExpandOnSingleClick(bool doThat);

    /**
     * Sets whether to select an album on click via the album manager or not.
     *
     * @param selectOnClick if true, a click on an item automatically sets this
     *                      item as the current album in the album manager
     */
    void setSelectAlbumOnClick(bool selectOnClick);

    /**
     * Determines the global decision to show a popup menu or not. More detailed
     * decision at which position a menu can be shown and where not can be made
     * by implementing showContextMenuAt.
     *
     * @param enable if true, a context menu can be shown
     */
    void setEnableContextMenu(bool enable);

    /**
     * Sets whether to select the album under the mouse cursor on a context menu
     * request (so that the album is shown using the album manager) or not
     *
     * Defaults to true.
     *
     * @param select true if a context menu request shall select the album
     */
    void setSelectOnContextMenu(bool select);

    /** This is a combination of indexAt() checked with visualRect().
     *  p must be in the viewport currently. Decoration will not be included.
     *  Suitable for mouse click positions.
     */
    QModelIndex indexVisuallyAt(const QPoint& p);

    virtual void doLoadState();
    virtual void doSaveState();

public Q_SLOTS:

    void setSearchTextSettings(const SearchTextSettings& settings);

    /**
     * Selects the given album.
     *
     * @param album album to select
     * @param selectInAlbumManager if <code>true</code>, the album will be
     *                             selected in the album manager
     */
    void slotSelectAlbum(Album *album, bool selectInAlbumManager = true);

    /**
     * Adapt the column sizes to the contents of the tree view.
     */
    void adaptColumnsToContent();

Q_SIGNALS:

    /// Emitted when the currently selected album changes
    void currentAlbumChanged(Album *currentAlbum);
    /// Emitted when the current selection changes. Use currentChanged unless in multi-selection mode.
    void selectedAlbumsChanged(QList<Album*> selectedAlbums);

protected Q_SLOTS:

    // override if implemented behavior is not as intended
    virtual void slotRootAlbumAvailable();

    void slotFilterChanged();
    void slotCurrentChanged();
    void slotSelectionChanged();

    void albumSettingsChanged();

protected:

    // context menu handling

    /**
     * Hook method to implement that determines if a context menu shall be
     * displayed for the given event at the position coded in the event.
     *
     * @param event context menu event to react on
     * @param albumForEvent the album at the mouse position or null if there is
     *                      no album at that position
     * @return true if a context menu shall be displayed at the event
     *         coordinates, else false
     */
    virtual bool showContextMenuAt(QContextMenuEvent *event, Album *albumForEvent);

    /**
     * Hook method that can be implemented to return a special icon used for the
     * context menu.
     *
     * @return the icon for the context menu
     */
    virtual QPixmap contextMenuIcon() const;

    /**
     * Hook method to implement that returns the title for the context menu.
     *
     * @return title for the context menu
     */
    virtual QString contextMenuTitle() const;

    /**
     * Hook method to add custom actions to the generated context menu.
     *
     * @param cmh helper object to create the context menu
     * @param album tag on which the context menu will be created. May be null if
     *              it is requested on no tag entry
     */
    virtual void addCustomContextMenuActions(ContextMenuHelper &cmh, Album *album);

    /**
     * Hook method to handle the custom context menu actions that were added
     * with addCustomContextMenuActions.
     *
     * @param action the action that was chosen by the user, may be null if none
     *               of the custom actions were selected
     * @param album the tag on which the context menu was requested. May be null
     *              if there was no
     */
    virtual void handleCustomContextMenuAction(QAction *action, Album *album);

    // other stuff

    bool checkExpandedState(const QModelIndex& index);
    void mousePressEvent(QMouseEvent *e);

    void startDrag(Qt::DropActions supportedActions);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dragLeaveEvent(QDragLeaveEvent * e);
    void dropEvent(QDropEvent *e);

    virtual void middleButtonPressed(Album *a);
    virtual QPixmap pixmapForDrag(const QStyleOptionViewItem& option, QList<QModelIndex> indexes);

    void setAlbumFilterModel(AlbumFilterModel *filterModel);

    AbstractSpecificAlbumModel *m_albumModel;
    AlbumFilterModel           *m_albumFilterModel;

    bool     m_checkOnMiddleClick;
    bool     m_restoreCheckState;

private:

    void saveStateRecursive(const QModelIndex &index, QStringList &selection,
                    QStringList &expansion);
    void restoreState(const QModelIndex &index);

    /**
     * Creates the context menu.
     *
     * @param event event that requested the menu
     */
    void contextMenuEvent(QContextMenuEvent *event);

private Q_SLOTS:

    /**
     * Used for asynchronous restoring of the tree view state if the contents of
     * the model are received after the view has been created.
     *
     * @param index parent index of the inserted data
     * @param start start row of new data under the parent index
     * @param end end row of new data under the parent index
     */
    void slotFixRowsInserted(const QModelIndex &index, int start, int end);

    /**
     * Adapts the columns in between the given model indices to the content
     * size. This can be connected to dataChanged.
     *
     * @param topLeft top left index of changed data
     * @param bottomRight index of changed data
     */
    void adaptColumnsOnDataChange(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    /**
     * Adapt the column sizes to new contents. This can be connected to all
     * signals indicating row changes.
     *
     * @param parent parent index of changed rows
     * @param start start row changed under the parent
     * @param end end row changed under the parent
     */
    void adaptColumnsOnRowChange(const QModelIndex &parent, int start, int end);

    /**
     * Adapts the column sizes if the layout changes.
     */
    void adaptColumnsOnLayoutChange();

private:

    AbstractAlbumTreeViewPriv *d;
};

class AbstractCountingAlbumTreeView : public AbstractAlbumTreeView
{
    Q_OBJECT

public:

    explicit AbstractCountingAlbumTreeView(AbstractCountingAlbumModel *model,
                                           AlbumFilterModel *filterModel,
                                           QWidget *parent = 0);
    explicit AbstractCountingAlbumTreeView(AbstractCountingAlbumModel *model, QWidget *parent = 0);

protected:

    void setAlbumFilterModel(AlbumFilterModel *filterModel);

private Q_SLOTS:

    void slotCollapsed(const QModelIndex& index);
    void slotExpanded(const QModelIndex& index);
    void slotSetShowCount();
    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void updateShowCountState(const QModelIndex& index, bool recurse);

private:

    void setupConnections();
};

class AbstractCheckableAlbumTreeViewPriv;
class AbstractCheckableAlbumTreeView : public AbstractCountingAlbumTreeView
{
Q_OBJECT
public:

    /// Models of these view _can_ be checkable, they need _not_. You need to enable it on the model.

    explicit AbstractCheckableAlbumTreeView(AbstractCheckableAlbumModel *model,
                                            CheckableAlbumFilterModel *filterModel,
                                            QWidget *parent = 0);
    explicit AbstractCheckableAlbumTreeView(AbstractCheckableAlbumModel *model, QWidget *parent = 0);

    virtual ~AbstractCheckableAlbumTreeView();

    /// Manage check state through the model directly
    AbstractCheckableAlbumModel *checkableModel() const;
    CheckableAlbumFilterModel *checkableAlbumFilterModel() const;

    /// Enable checking on middle mouse button click (default: on)
    void setCheckOnMiddleClick(bool doThat);

    /**
     * Tells if the check state is restored while loading / saving state.
     *
     * TODO tristate not supported right now
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

    virtual void middleButtonPressed(Album *a);

private Q_SLOTS:

    void slotRowsAddedCheckState(QModelIndex index, int start, int end);

private:

    void restoreCheckState(const QModelIndex &index);

    AbstractCheckableAlbumTreeViewPriv *d;

};

class AlbumTreeViewPriv;
class AlbumTreeView : public AbstractCheckableAlbumTreeView
{
    Q_OBJECT
public:

    AlbumTreeView(AlbumModel *model, QWidget *parent = 0);
    virtual ~AlbumTreeView();
    AlbumModel *albumModel() const;
    PAlbum *currentAlbum() const;
    PAlbum *albumForIndex(const QModelIndex &index) const;

};

class TagTreeView : public AbstractCheckableAlbumTreeView
{
    Q_OBJECT
public:

    TagTreeView(TagModel *model, QWidget *parent = 0);
    TagModel *albumModel() const;
    TAlbum *currentAlbum() const;
    TAlbum *albumForIndex(const QModelIndex &index) const;

    TagModificationHelper *tagModificationHelper() const;

Q_SIGNALS:

    void assignTags(int tagId, const QList<int>& imageIDs);

protected:

    TagModificationHelper *m_modificationHelper;
};

class SearchTreeView : public AbstractCheckableAlbumTreeView
{
    Q_OBJECT
public:

    SearchTreeView(QWidget *parent, SearchModel *searchModel);
    /// Note: not filtered by search type
    SearchModel *albumModel() const;
    /// Contains only the searches with appropriate type - prefer to albumModel()
    SearchFilterModel *filteredModel() const;
    SAlbum *currentAlbum() const;

public Q_SLOTS:

    void slotSelectSAlbum(SAlbum *salbum);

protected:

    SearchFilterModel *m_filteredModel;
};

class DateAlbumTreeView : public AbstractCountingAlbumTreeView
{
public:

    DateAlbumTreeView(QWidget *parent, DateAlbumModel *dateAlbumModel);
    DateAlbumModel *albumModel() const;
    DAlbum *currentAlbum() const;
    DAlbum *albumForIndex(const QModelIndex &index) const;
};



}

#endif

