/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CONTEXTMENUHELPER_H
#define CONTEXTMENUHELPER_H

// Qt includes

#include <QObject>
#include <QList>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "digikam_config.h"
#include "coredbalbuminfo.h"

class QAction;
class QMenu;
class QPoint;
class QString;

class KActionCollection;

namespace Digikam
{

class AbstractCheckableAlbumModel;
class Album;
class AlbumIconItem;
class AlbumModificationHelper;
class ImageInfo;
class ImageFilterModel;
class PAlbum;
class TagModificationHelper;
class TAlbum;

/**
 * @brief A helper class to add actions and special menus to the context menu.
 *
 * The %ContextMenuHelper class helps adding commonly used actions and menus.
 * Use this class to add
 *  - actions from the actionCollection
 *  - standard actions (copy, paste, delete)
 *  - temporary actions
 *  - predefined special actions
 *  - predefined submenus
 *  to the menu.
 *
 * All addAction() methods take a special parameter 'addDisabled'. This
 * parameter controls if disabled actions are added to the menu. Normally
 * adding disabled actions is turned off, to clean up the menu and make it
 * more readable.
 *
 * If the %ContextMenuHelper class is used, you need to call its own exec() method,
 * instead the one from the parent menu. This way signals from
 * special menus can be emitted and connected to the appropriate slots.
 */
class DIGIKAM_EXPORT ContextMenuHelper : public QObject
{
    Q_OBJECT

public:

    typedef const QList<qlonglong> imageIds;

Q_SIGNALS:

    void signalSetThumbnail(const ImageInfo&);
    void signalGotoAlbum(const ImageInfo&);
    void signalGotoDate(const ImageInfo&);
    void signalGotoTag(int);
    void signalAssignTag(int);
    void signalRemoveTag(int);
    void signalAssignPickLabel(int);
    void signalAssignColorLabel(int);
    void signalAssignRating(int);
    void signalAddToExistingQueue(int);
    void signalAddNewTagFromABCMenu(const QString&);
    void signalPopupTagsView();
    void signalCreateGroup();
    void signalCreateGroupByTime();
    void signalCreateGroupByFilename();
    void signalUngroup();
    void signalRemoveFromGroup();

public:

    /**
     * Constructs the helper class.
     *
     * @param parent the menu the helper class is linked to
     * @param actionCollection the actionCollection that should be used. If not set, the standard
     * action from DigikamApp is used
     */
    explicit ContextMenuHelper(QMenu* parent, KActionCollection* actionCollection = 0);
    virtual ~ContextMenuHelper();

    /**
     * Add an action from the actionCollection.
     *
     * This method adds actions from the actionCollection. The actionCollection can
     * be set in the constructor of the ContextMenuHelper class.
     *
     * @param name the name of the action in the actionCollection
     * @param addDisabled if set, disabled actions are added to the menu
     */
    void addAction(const QString& name, bool addDisabled = false);

    /**
     * Add a temporary action.
     *
     * Sometimes it is necessary to define actions that only exist in the current context menu content.
     * Use this method to add such an action.
     *
     * @param action the action to add
     * @param addDisabled if set, disabled actions are added to the menu
     */
    void addAction(QAction* action, bool addDisabled = false);

    /**
     * Add a temporary action and assign it to a custom slot.
     *
     * Use this method if you want to add a temporary action and immediately connect it to the
     * receiving slot.
     *
     * @param action the action to add
     * @param recv the receiver of the triggered action
     * @param slot the slot to connect the triggered action to
     * @param addDisabled if set, disabled actions are added to the menu
     */
    void addAction(QAction* action, QObject* recv, const char* slot, bool addDisabled = false);

    /**
     * Add the standard cut action and connect it to the appropriate slot
     *
     * @param recv the receiver of the triggered action
     * @param slot the slot to connect the triggered action to
     */
    void addStandardActionCut(QObject* recv, const char* slot);

    /**
     * Add the standard copy action and connect it to the appropriate slot
     *
     * @param recv the receiver of the triggered action
     * @param slot the slot to connect the triggered action to
     */
    void addStandardActionCopy(QObject* recv, const char* slot);

    /**
     * Add the standard paste action and connect it to the appropriate slot
     *
     * @param recv the receiver of the triggered action
     * @param slot the slot to connect the triggered action to
     */
    void addStandardActionPaste(QObject* recv, const char* slot);

    /**
     * Add the standard delete action and connect it to the appropriate slot
     *
     * @param recv the receiver of the triggered action
     * @param slot the slot to connect the triggered action to
     * @param quantity the number of the files that should be deleted. This parameter is used for
     * the action name and is normally used when deleting more then one item.
     */
    void addStandardActionItemDelete(QObject* recv, const char* slot, int quantity = 1);

    /**
     * Add the lighttable action to the menu.
     *
     * Do not use addAction() to add the lighttable action, because we need
     * to handle special cases here. Depending on whether the lighttable window
     * has already been created and filled with items, we set different actions.
     */
    void addStandardActionLightTable();

    /**
     * Add the thumbnail action to the menu.
     *
     * Do not use addAction() to add the thumbnail action, because we need
     * to handle special cases here. Depending on whether the current view is
     * album or icon view, we set different actions.
     *
     * @param ids the selected items in the current view
     * @param album the current album the AlbumIconView is displaying
     */
    void addStandardActionThumbnail(const imageIds& ids, Album* album);

    /**
     * Add section for main views for opening and moving/going to albums.
     *
     * This is a convenience function to ensure consistent menues and reduce
     * code duplication.
     *
     * @param imageIds the list of selected items
     */
    void addOpenAndNavigateActions(const imageIds &ids);

    /**
     * Add the services menu to the menu.
     *
     * The services menu is used to open the selected items in a different application.
     * It will query the item for registered services and provide them in a submenu.
     * The menu will be titled "Open With...".
     *
     * @param selectedItems the list of selected items
     */
    void addServicesMenu(const QList<QUrl>& selectedItems);

    /**
     * Add the Goto menu.
     *
     * This menu will provide the following actions for the given item:
     * - Goto Album
     * - Goto Date
     * - Goto Tag
     * To make this menu work, you need to run exec() from this class, otherwise the signals
     * are not emitted and you will not be able to react on triggered actions from this menu.
     * Make sure to connect the signals to the appropriate slots in the context menu handling method.
     *
     * @param imageIds the list of selected items
     * @see exec()
     * @see signalGotoAlbum() signalGotoDate() signalGotoTag()
     */
    void addGotoMenu(const imageIds& ids);

    /**
     * Add Queue Manager actions menu.
     */
    void addQueueManagerMenu();

    /**
     * Add actions to add, remove or edit a tag.
     * The tag modification helper is used to execute the action.
     * You must set the parent tag to use on modification helper.
     */
    void addActionNewTag(TagModificationHelper* helper, TAlbum* parentTag = 0);
    void addActionDeleteTag(TagModificationHelper* helper, TAlbum* tag);
    void addActionDeleteTags(TagModificationHelper* helper, QList< TAlbum* > tags);
    void addActionEditTag(TagModificationHelper* helper, TAlbum* tag);

    /**
     * Add action to delete tags from people sidebar.
     */
    void addActionDeleteFaceTag(TagModificationHelper* helper, TAlbum* tag);
    void addActionDeleteFaceTags(TagModificationHelper* helper, QList< TAlbum* > tags);

    /**
     * Add action to set tags as face tags.
     */
    void addActionTagToFaceTag(TagModificationHelper* helper, TAlbum* tag);
    void addActionTagsToFaceTags(TagModificationHelper* helper, QList< TAlbum* > tags);

    /**
     * Add actions to add, remove or edit a tag.
     * The tag modification helper is used to execute the action.
     * You must set the parent tag to use on modification helper.
     */
    void addActionNewAlbum(AlbumModificationHelper* helper, PAlbum* parentAlbum = 0);
    void addActionDeleteAlbum(AlbumModificationHelper* helper, PAlbum* album);
    void addActionEditAlbum(AlbumModificationHelper* helper, PAlbum* album);
    void addActionRenameAlbum(AlbumModificationHelper* helper, PAlbum* album);
    void addActionResetAlbumIcon(AlbumModificationHelper* helper, PAlbum* album);

    /**
     * Add "Assign Tags" menu.
     *
     * This menu will provide a list of all tags available so that they can be assigned to the current
     * selected items.
     *
     * To make this menu work, you need to run exec() from this class, otherwise the signals
     * are not emitted and you will not be able to react on triggered actions from this menu.
     * Make sure to connect the signals to the appropriate slots in the context menu handling method.
     *
     * @param ids the selected items
     * @see exec()
     * @see signalAssignTag()
     */
    void addAssignTagsMenu(const imageIds& ids);

    /**
     * Add "Remove Tags" menu.
     *
     * This menu will provide a list of all tags assigned to the current items. Actions triggered in here
     * will remove the selected tag from the items.
     *
     * To make this menu work, you need to run exec() from this class, otherwise the signals
     * are not emitted and you will not be able to react on triggered actions from this menu.
     * Make sure to connect the signals to the appropriate slots in the context menu handling method.
     *
     * @param ids the selected items
     * @see exec()
     * @see signalRemoveTag()
     */
    void addRemoveTagsMenu(const imageIds& ids);

    /**
     * Add a menu to create new tags from adressbook entries.
     */
    void addCreateTagFromAddressbookMenu();

    /**
    * Add "Pick/Color/Rating Labels" action.
    *
    * This action will provide methods to assign pick/color/rating labels to the currently selected items.
    *
    * To make this menu work, you need to run exec() from this class, otherwise the signals
    * are not emitted and you will not be able to react on triggered actions from this menu.
    * Make sure to connect the signals to the appropriate slots in the context menu handling method.
    *
    * @see exec()
    * @see signalAssignPickLabel()
    * @see signalAssignColorLabel()
    * @see signalAssignRating()
    */
    void addLabelsAction();

    /**
     * Add a "Group" menu.
     * This menu will provide actions open, close, add to, remove from, or split a group.
     *
     * addGroupActions will add the actions as a flat list, not in a submenu.
     * Note: Call setImageFilterModel before to have Open/Close group actions.
     */
    void addGroupMenu(const imageIds& ids, const QList<QAction*>& extraMenuItems = QList<QAction*>());
    void addGroupActions(const imageIds& ids);

    /**
     * Set a filter model.
     * Some of the group actions will operate directly on the model.
     */
    void setImageFilterModel(ImageFilterModel* model);

    /**
     * Add a Select and Deselect menu to check and uncheck albums.
     * Note: Call setAlbumModel before, or this will have no effect.
     */
    void addAlbumCheckUncheckActions(Album* album);

    /**
     * Set an album model.
     * The check/uncheck actions will operate directly on the model.
     */
    void setAlbumModel(AbstractCheckableAlbumModel* model);

    /**
     * Add Import Webservices actions menu.
     */
    void addImportMenu();

    /**
     * Add Export Webservices actions menu.
     */
    void addExportMenu();

    void addAlbumActions();

    /**
     * Add a submenu to the parent context menu.
     *
     * @param subMenu   the submenu to be added
     */
    void addSubMenu(QMenu* subMenu);

    /**
     * Add a separator to the context menu
     */
    void addSeparator();

    /**
     * Execute the registered parent menu and evaluate the triggered actions.
     *
     * Always use this method instead the one from the parent menu.
     * It will ensure that the signals are emitted and special cases are handled.
     *
     * @param pos position of the triggered action in the registered menu
     * @param at the action that should be at the position pos
     * @return the triggered action
     */
    QAction* exec(const QPoint& pos, QAction* at = 0);

private Q_SLOTS:

    void slotOpenWith();
    void slotOpenWith(QAction* action);
    void slotDeselectAllAlbumItems();
    void slotOpenGroups();
    void slotCloseGroups();
    void slotOpenAllGroups();
    void slotCloseAllGroups();
    void slotSelectChildren();
    void slotDeselectChildren();
    void slotSelectParents();
    void slotDeselectParents();

private:

    void setGroupsOpen(bool open);
    void setSelectedIds(const imageIds& ids);
    void setSelectedItems(const QList<QUrl>& urls);

    bool imageIdsHaveSameCategory(const imageIds& ids, DatabaseItem::Category category);
    QList<QAction*> groupMenuActions(const imageIds& ids);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* CONTEXTMENUHELPER_H */
