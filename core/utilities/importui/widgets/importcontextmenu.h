/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-13
 * Description : Modified context menu helper for import tool
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMPORTCONTEXTMENU_H
#define IMPORTCONTEXTMENU_H

// Qt includes

#include <QMenu>

// Local includes

#include "digikam_config.h"
#include "camiteminfo.h"
#include "importfiltermodel.h"

class QAction;

class KActionCollection;

namespace Digikam
{

class ImportContextMenuHelper : public QObject
{
    Q_OBJECT

public:

    typedef const QList<qlonglong> itemIds;

Q_SIGNALS:

    //void signalAssignTag(int);
    //void signalRemoveTag(int);
    //void signalPopupTagsView();
    void signalAssignPickLabel(int);
    void signalAssignColorLabel(int);
    void signalAssignRating(int);
    void signalAddNewTagFromABCMenu(const QString&);
    //void signalCreateGroup();
    //TODO: void signalUngroup();
    //TODO: void signalRemoveFromGroup();

public:

    /**
     * Constructs the helper class.
     *
     * @param parent the menu the helper class is linked to
     * @param actionCollection the actionCollection that should be used. If not set, the standard
     * action from DigikamApp is used
     */
    explicit ImportContextMenuHelper(QMenu* const parent, KActionCollection* const actionCollection = 0);
    virtual ~ImportContextMenuHelper();

    /**
     * Add an action from the actionCollection.
     *
     * This method adds actions from the actionCollection. The actionCollection can
     * be set in the constructor of the ImportContextMenuHelper class.
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
     * Add actions to add, remove or edit a tag.
     * The tag modification helper is used to execute the action.
     * You must set the parent tag to use on modification helper.
     */
    //TODO: void addActionNewTag(TagModificationHelper* helper, TAlbum* parentTag = 0);
    //TODO: void addActionDeleteTag(TagModificationHelper* helper, TAlbum* tag);
    //TODO: void addActionEditTag(TagModificationHelper* helper, TAlbum* tag);

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
    void addAssignTagsMenu(itemIds& ids);

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
    void addRemoveTagsMenu(itemIds& ids);

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
     * Add a menu to rotate item.
     * @param ids the selected items
     */
    void addRotateMenu(itemIds& ids);

    /**
     * Add a "Group" menu.
     * This menu will provide actions open, close, add to, remove from, or split a group.
     *
     * addGroupActions will add the actions as a flat list, not in a submenu.
     * Note: Call setImageFilterModel before to have Open/Close group actions.
     */
    void addGroupMenu(itemIds& ids);
    void addGroupActions(itemIds& ids);

    /**
     * Set a filter model.
     * Some of the group actions will operate directly on the model.
     */
    void setImportFilterModel(ImportFilterModel* model);

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
    //    void slotABCImportContextMenu();
    void slotABCMenuTriggered(QAction*);
    //void slotOpenGroups();
    //void slotCloseGroups();
    //void slotOpenAllGroups();
    //void slotCloseAllGroups();
    void slotRotate();

private:

    void setSelectedIds(itemIds& ids);
    void setSelectedItems(const QList<QUrl>& urls);
    //QList<QAction*> groupMenuActions(itemIds& ids);
    void setGroupsOpen(bool open);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMPORTIMPORTCONTEXTMENU_H
