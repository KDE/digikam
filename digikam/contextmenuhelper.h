/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

// Qt includes.

#include <QMap>
#include <QObject>

// KDE includes.

#include <kservice.h>

class QAction;
class QMenu;
class QPoint;
class QString;
class Q3ListViewItem;

class KActionCollection;

namespace Digikam
{

class Album;
class AlbumIconItem;
class ContextMenuHelperPriv;
class ImageInfo;

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
 * If the %ContextMenuHelper class is used, it is usually the best to use its
 * own exec() method, instead the one from the assigned menu. This way signals from
 * special menus can be emitted and connected to the appropriate slots.
 */
class ContextMenuHelper : public QObject
{
    Q_OBJECT

public:

    typedef const QList<qlonglong> imageIds;

Q_SIGNALS:

    void signalSetThumbnail(ImageInfo&);
    void signalGotoAlbum(ImageInfo&);
    void signalGotoDate(ImageInfo&);
    void signalGotoTag(int);
    void signalAssignTag(int);
    void signalRemoveTag(int);
    void signalAssignRating(int);
    void signalAddToExistingQueue(int);

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
     * This method will help you adding actions from the actionCollection. The actionCollection can
     * be set in the constructor of the ContextMenuHelper class.
     *
     * @param name the name of the action in the actionCollection
     * @param addDisabled if set, disabled actions are added to the menu
     * @see ContextMenuHelper()
     */
    void addAction(const char* name, bool addDisabled = false);

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
     * Add the standard copy action and connect it to the appropriate slot
     *
     * @param recv the receiver of the triggered action
     * @param slot the slot to connect the triggered action to
     */
    void addActionCopy(QObject* recv, const char* slot);

    /**
     * Add the standard paste action and connect it to the appropriate slot
     *
     * @param recv the receiver of the triggered action
     * @param slot the slot to connect the triggered action to
     */
    void addActionPaste(QObject* recv, const char* slot);

    /**
     * Add the standard delete action and connect it to the appropriate slot
     *
     * @param recv the receiver of the triggered action
     * @param slot the slot to connect the triggered action to
     * @param quantity the number of the files that should be deleted. This parameter is used for
     * the action name and is normally used when deleting more then one item.
     */
    void addActionItemDelete(QObject* recv, const char* slot, int quantity = 1);

    /**
     * Add the lighttable action to the menu.
     *
     * Do not use addAction() to add the lighttable action, because we need
     * to handle special cases here. Depending on whether the lighttable window
     * has already been created and filled with items, we set different actions.
     */
    void addActionLightTable();

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
    void addActionThumbnail(imageIds& ids, Album* album);

    /**
     * Add the services menut to the menu.
     *
     * The services menu is used to open the selected items in a different application.
     * It will query the item for registered services and provide them in a submenu.
     * The menu will be titled "Open With...".
     *
     * @param item the selected item
     * @param servicesMap a reference to a map that will be filled with the detected services
     */
    void addServicesMenu(const ImageInfo &item, QMap<QAction*, KService::Ptr> &servicesMap);

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
     * @param ids the selected items
     * @see exec()
     * @see signalGotoAlbum() signalGotoDate() signalGotoTag()
     */
    void addGotoMenu(imageIds& ids);

    /**
     * Add Queue Manager actions menu.
     */
    void addQueueManagerMenu();

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
    void addAssignTagsMenu(imageIds& ids);

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
    void addRemoveTagsMenu(imageIds& ids);

    /**
     * Add a menu to create new tags from adressbook entries.
     */
    void addCreateTagFromAddressbookMenu();

    /**
     * Add a menu to select tags in a tag item based view.
     *
     * This method will add a menu that allows selecting tags by the following criteria:
     * - All - select all tags
     * - Children - select the highlighted item and its children
     * - Parents - select the highlighted item and its parents
     *
     * @param item the highlighted item
     */
    void addSelectTagsMenu(Q3ListViewItem *item);

    /**
     * Add "Rating" menu.
     *
     * This menu will provide methods to assign ratings to the currently selected items.
     *
     * To make this menu work, you need to run exec() from this class, otherwise the signals
     * are not emitted and you will not be able to react on triggered actions from this menu.
     * Make sure to connect the signals to the appropriate slots in the context menu handling method.
     *
     * @see exec()
     * @see signalAssignRating()
     */
    void addRatingMenu();

    /**
     * Add some of the KIPI actions to the menu.
     *
     * This method will add some of the KIPI actions into the context menu, right now only the
     * rotation actions are added.
     */
    void addKipiActions();

    /**
     * Add Import KIPI actions menu.
     */
    void addImportMenu();

    /**
     * Add Export KIPI actions menu.
     */
    void addExportMenu();

    /**
     * Add Batch KIPI actions menu.
     */
    void addBatchMenu();

    void addAlbumActions();

    /**
     * Execute the registered menu and evaluate the triggered actions.
     *
     * Always use this method instead the one from QMenu.
     * It will ensure that the signals are emitted or other special cases are handled.
     *
     * @param pos position of the triggered action in the registered menu
     * @param at the action that should be at the position pos
     * @return the triggered action
     */
    QAction* exec(const QPoint& pos, QAction* at = 0);

private:

    /**
     * Assign the selected image ids.
     *
     * @param ids the selected image ids
     */
    void setSelectedIds(imageIds& ids);

private:

    ContextMenuHelperPriv* const d;
};

} // namespace Digikam

#endif /* CONTEXTMENUHELPER_H */
