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
#include "tagmodificationhelper.h"

namespace Digikam
{

class ContextMenuHelper;

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
class AbstractAlbumTreeView : public QTreeView
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

    virtual void loadViewState(KConfigGroup &group, QString prefix = QString());
    virtual void saveViewState(KConfigGroup &group, QString prefix = QString());

public Q_SLOTS:

    void setSearchTextSettings(const SearchTextSettings& settings);

    /**
     * Selects the given album.
     *
     * @param album album to select
     */
    void slotSelectAlbum(Album *album);

Q_SIGNALS:

    /// Emitted when the currently selected album changes
    void currentAlbumChanged(Album *currentAlbum);
    /// Emitted when the current selection changes. Use currentChanged unless in multi-selection mode.
    void selectedAlbumsChanged(QList<Album*> selectedAlbums);
    /// Emitted when search text settings have been changed
    void filteringDone(bool atLeastOneMatch);

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

private:

    void saveState(const QModelIndex &index, QStringList &selection,
                    QStringList &expansion);
    void restoreState(const QModelIndex &index);

    /**
     * Creates the context menu.
     *
     * @param event event that requested the menu
     */
    void contextMenuEvent(QContextMenuEvent *event);

private Q_SLOTS:

    void slotFixRowsInserted(const QModelIndex &index, int start, int end);

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

class AbstractCheckableAlbumTreeView : public AbstractCountingAlbumTreeView
{
Q_OBJECT
public:

    /// Models of these view _can_ be checkable, they need _not_. You need to enable it on the model.

    explicit AbstractCheckableAlbumTreeView(AbstractCheckableAlbumModel *model,
                                            CheckableAlbumFilterModel *filterModel,
                                            QWidget *parent = 0);
    explicit AbstractCheckableAlbumTreeView(AbstractCheckableAlbumModel *model, QWidget *parent = 0);

    /// Manage check state through the model directly
    AbstractCheckableAlbumModel *checkableModel() const;
    CheckableAlbumFilterModel *checkableAlbumFilterModel() const;

    /// Enable checking on middle mouse button click (default: on)
    void setCheckOnMiddleClick(bool doThat);

protected:

    virtual void middleButtonPressed(Album *a);
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

    TagTreeView(TagModel *model, TagModificationHelper *tagModificationHelper, QWidget *parent = 0);
    TagModel *albumModel() const;
    TAlbum *currentAlbum() const;
    TAlbum *albumForIndex(const QModelIndex &index) const;

Q_SIGNALS:
    void assignTags(int tagId, const QList<int>& imageIDs);

};

class SearchTreeView : public AbstractAlbumTreeView
{
    Q_OBJECT
public:

    SearchTreeView(QWidget *parent, SearchModel *searchModel);
    SearchModel *albumModel() const;
    SearchFilterModel *albumFilterModel() const;
    SAlbum *currentAlbum() const;

public Q_SLOTS:
    void slotSelectSAlbum(SAlbum *salbum);

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

