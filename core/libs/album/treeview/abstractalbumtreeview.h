/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ABSTRACT_ALBUM_TREE_VIEW_H
#define DIGIKAM_ABSTRACT_ALBUM_TREE_VIEW_H

// Qt includes

#include <QTreeView>

// Local includes

#include "albummanager.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "albumpointer.h"
#include "statesavingobject.h"

namespace Digikam
{

class ContextMenuHelper;
class TagModificationHelper;

// NOTE: This structure name can be in conflict with QAbstractItemView::State.
struct State;

class AbstractAlbumTreeView;

/**
 * Base class for all tree views that display Album-based content provided by an
 * AbstractSpecificAlbumModel. This class enables various utility functions like
 * selecting albums on mouse actions or providing an infrastructure for
 * displaying a context menu for albums.
 *
 * Context menu handling is implemented as template methods with hook methods
 * that can be implemented by subclasses to provide a custom behavior. In
 * default mode no context menu is shown at all. It must be enabled via a call
 * to setEnableContextMenu.
 */
class AbstractAlbumTreeView : public QTreeView, public StateSavingObject
{
    Q_OBJECT

public:

    enum Flag
    {
        /** Create a default model. Not supported by abstract classes. Not part of default flags! */
        CreateDefaultModel,
        /** Create a default filter model. */
        CreateDefaultFilterModel,
        /** Create a delegate which paints according to settings.
         *  If not set, the Qt default delegate of the view is used. */
        CreateDefaultDelegate,
        /** Show the count according to the settings. If not set, call
         *  setShowCount() on the model yourself. */
        ShowCountAccordingToSettings,

        DefaultFlags = CreateDefaultFilterModel | CreateDefaultDelegate | ShowCountAccordingToSettings
    };
    Q_DECLARE_FLAGS(Flags, Flag)

public:

    /**
     * Constructs an album tree view.
     * If you give 0 for model, call setAlbumModel afterwards.
     * If you supply 0 for filterModel, call setAlbumFilterModel afterwards.
     */
    AbstractAlbumTreeView(QWidget* const parent, Flags flags);
    ~AbstractAlbumTreeView();

    AbstractSpecificAlbumModel* albumModel() const;
    AlbumFilterModel* albumFilterModel()     const;

    /// Enable expanding of tree items on single click on the item (default: off)
    void setExpandOnSingleClick(const bool doThat);

    /// Expand an item when making it the new current item
    void setExpandNewCurrentItem(const bool doThat);

    /**
     * Sets whether to select an album on click via the album manager or not.
     *
     * @param selectOnClick if true, a click on an item automatically sets this
     *                      item as the current album in the album manager
     */
    void setSelectAlbumOnClick(const bool selectOnClick);

    /**
     * Determines the global decision to show a popup menu or not. More detailed
     * decision at which position a menu can be shown and where not can be made
     * by implementing showContextMenuAt.
     *
     * @param enable if true, a context menu can be shown
     */
    void setEnableContextMenu(const bool enable);

    /**
     * Sets whether to select the album under the mouse cursor on a context menu
     * request (so that the album is shown using the album manager) or not
     *
     * Defaults to true.
     *
     * @param select true if a context menu request shall select the album
     */
    void setSelectOnContextMenu(const bool select);

    /**
     * Set the context menu title and icon.
     * This is used by the default implementation of contextMenuIcon()
     * and contextMenuTitle(). You can alternatively reimplement these methods.
     */
    void setContextMenuIcon(const QPixmap& pixmap);
    void setContextMenuTitle(const QString& title);

    /**
     * This is a combination of indexAt() checked with visualRect().
     * p must be in the viewport currently. Decoration will not be included.
     * Suitable for mouse click positions.
     */
    QModelIndex indexVisuallyAt(const QPoint& p);

    /**
     * Ensures that every current match is visible by expanding all parent
     * entries.
     *
     * @param index index to start ensuring expansion state
     * @return <code>true</code> if there was a match under <code>index</code>.
     *         This return value can normally be ignored by the caller because
     *         it is only used for an internal recursion.
     */
    bool expandMatches(const QModelIndex& index);

    /**
     * Implements state loading for the album tree view in a somewhat clumsy
     * procedure because the model may not be fully loaded when this method is
     * called. Therefore the config is first parsed into d->statesByAlbumId
     * which holds the state of all tree view entries indexed by album id.
     * Afterwards an initial sync run is done restoring the state of all model
     * entries that are already present at this time. Every processed entry
     * is removed from d->statesByAlbumId. If there are still entries left in
     * this map we assume that the model is not fully loaded at the moment.
     * Therefore the rowsInserted signal is connected to a slot that restores
     * the state of new rows based on the remaining entries in
     * d->statesByAlbumId.
     */
    virtual void doLoadState();
    virtual void doSaveState();

    /**
     * Some treeviews shall control the global current album kept by AlbumManager.
     * Other treeview are self-contained and shall not change the current album.
     * Default: false
     */
    void setAlbumManagerCurrentAlbum(const bool setCurrentAlbum);

    /**
     * Add a context menu element which can add actions to the context
     * menu when the menu is generated.
     * First, addCustomContextMenuActions is called, then
     * all elements' addActions method is called in order of addition.
     */
    class ContextMenuElement
    {
    public:

        virtual ~ContextMenuElement()
        {
        }

        /**
        * Add actions to the context menu being generated
        *
        * @param view The AbstractAlbumTreeView which generates the menu
        * @param cmh helper object to create the context menu
        * @param album album on which the context menu will be created. May be null if
        *              it is requested on no tag entry
        */
        virtual void addActions(AbstractAlbumTreeView* view, ContextMenuHelper& cmh, Album* album) = 0;
    };

    void addContextMenuElement(ContextMenuElement* const element);
    void removeContextMenuElement(ContextMenuElement* const element);
    QList<ContextMenuElement*> contextMenuElements() const;

    template<class A>
    QList<A*> currentAlbums();

    // for internal use: public viewportEvent
    virtual bool viewportEvent(QEvent* event);

    /**
     * @brief selectedItems() -
     */
    QList<Album*> selectedItems();

public Q_SLOTS:

    void setSearchTextSettings(const SearchTextSettings& settings);

    /**
     * Selects the given album.
     *
     * @param albums albums to select
     * @param selectInAlbumManager the album will be set as current album, if both
     * this parameter is true and setAlbumManagerCurrentAlbum() was set to true.
     */
    void setCurrentAlbums(const QList<Album*>& albums, bool selectInAlbumManager = true);

    /**
     * Adapt the column sizes to the contents of the tree view.
     */
    void adaptColumnsToContent();

    /**
     * Scrolls to the first selected album if there is one.
     */
    void scrollToSelectedAlbum();

    /**
     * Expands the complete tree under the given index.
     *
     * @param index index to start expanding everything
     */
    void expandEverything(const QModelIndex& index);

Q_SIGNALS:

    /// Emitted when the currently selected album changes
    void currentAlbumChanged(Album* currentAlbum);

    /// Emitted when the current selection changes. Use currentChanged unless in multi-selection mode.
    void selectedAlbumsChanged(const QList<Album*>& selectedAlbums);

protected Q_SLOTS:

    // override if implemented behavior is not as intended
    virtual void slotRootAlbumAvailable();

    void slotSearchTextSettingsChanged(bool wasSearching, bool searching);
    void slotSearchTextSettingsAboutToChange(bool searched, bool willSearch);
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
    virtual bool showContextMenuAt(QContextMenuEvent* event, Album* albumForEvent);

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
    virtual void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album);

    /**
     * Hook method to handle the custom context menu actions that were added
     * with addCustomContextMenuActions.
     *
     * @param action the action that was chosen by the user, may be null if none
     *               of the custom actions were selected
     * @param album the tag on which the context menu was requested. May be null
     *              if there was no
     */
    virtual void handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album);

    // other stuff

    void mousePressEvent(QMouseEvent* e);

    void rowsInserted(const QModelIndex& index, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void startDrag(Qt::DropActions supportedActions);
    void dragEnterEvent(QDragEnterEvent* e);
    void dragMoveEvent(QDragMoveEvent* e);
    void dragLeaveEvent(QDragLeaveEvent* e);
    void dropEvent(QDropEvent* e);

    virtual void middleButtonPressed(Album* a);
    virtual QPixmap pixmapForDrag(const QStyleOptionViewItem& option, QList<QModelIndex> indexes);

    void setAlbumFilterModel(AlbumFilterModel* const filterModel);
    void setAlbumModel(AbstractSpecificAlbumModel* const model);

protected:

    AbstractSpecificAlbumModel* m_albumModel;
    AlbumFilterModel*           m_albumFilterModel;
    AlbumModelDragDropHandler*  m_dragDropHandler;

    bool                        m_checkOnMiddleClick;
    bool                        m_restoreCheckState;
    Flags                       m_flags;

private:

    void saveStateRecursive(const QModelIndex& index, QList<int>& selection, QList<int>& expansion);

    /**
     * Restores the state of the index and all sub-indexes if there is an entry
     * for this index in stateStore. Every album that is restored is removed
     * from the stateStore.
     *
     * @param index index to start restoring
     * @param stateStore states indexed by album id
     */
    void restoreStateForHierarchy(const QModelIndex& index, const QMap<int, Digikam::State>& stateStore);
    /**
     * Restore the state for this index.
     */
    void restoreState(const QModelIndex& index, const QMap<int, Digikam::State>& stateStore);

    /**
     * Creates the context menu.
     *
     * @param event event that requested the menu
     */
    void contextMenuEvent(QContextMenuEvent* event);

private Q_SLOTS:

    /**
     * Adapts the columns in between the given model indices to the content
     * size. This can be connected to dataChanged.
     *
     * @param topLeft top left index of changed data
     * @param bottomRight index of changed data
     */
    void adaptColumnsOnDataChange(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    /**
     * Adapt the column sizes to new contents. This can be connected to all
     * signals indicating row changes.
     *
     * @param parent parent index of changed rows
     * @param start start row changed under the parent
     * @param end end row changed under the parent
     */
    void adaptColumnsOnRowChange(const QModelIndex& parent, int start, int end);

    /**
     * Adapts the column sizes if the layout changes.
     */
    void adaptColumnsOnLayoutChange();

    /**
     * This slot is used to ensure that after searching for entries the correct
     * album is selected again. Therefore it tracks new selections.
     */
    void currentAlbumChangedForBackupSelection(Album* currentAlbum);

private:

    class Private;
    Private* d;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::AbstractAlbumTreeView::Flags)

#endif // DIGIKAM_ABSTRACT_ALBUM_TREE_VIEW_H
