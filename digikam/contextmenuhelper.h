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

public:

    explicit ContextMenuHelper(QMenu* parent, KActionCollection* actionCollection = 0);
    virtual ~ContextMenuHelper();

    /*
     * The ContextMenuHelper class makes it easier to add actions to a menu.
     * Use this class to add
     *  - actions from the actionCollection
     *  - standard actions (copy, paste, delete)
     *  - temporary actions
     *  - predefined special actions
     *  - predefined submenus
     *  to the menu.
     *
     *  All addAction() methods take a special parameter 'addDisabled'. This
     *  parameter controls if disabled actions are added to the menu. Normally
     *  adding disabled actions is turned off, to clean up the menu and make it
     *  more readable.
     */

    // add action from actionCollection
    void addAction(const char* name, bool addDisabled = false);

    // add temporary action
    void addAction(QAction* action, bool addDisabled = false);
    void addAction(QAction* action, QObject* recv, const char* slot, bool addDisabled = false);

    // add standard actions
    void addActionCopy(QObject* recv, const char* slot);
    void addActionPaste(QObject* recv, const char* slot);
    void addActionItemDelete(QObject* recv, const char* slot, int quantity = 1);

    // add special actions
    void addActionLightTable();
    void addActionThumbnail(imageIds& ids, Album* album);

    // add special menus
    void addServicesMenu(const ImageInfo&, QMap<QAction*, KService::Ptr>&);
    void addGotoMenu(imageIds& ids);
    void addQueueManagerMenu();

    // tags & rating menus
    void addAssignTagsMenu(imageIds& ids);
    void addRemoveTagsMenu(imageIds& ids);
    void addCreateTagFromAddressbookMenu();
    void addSelectTagsMenu(Q3ListViewItem *item);
    void addRatingMenu();

    // KIPI menus
    void addKipiActions();
    void addImportMenu();
    void addExportMenu();
    void addBatchMenu();
    void addAlbumActions();

    // execute the registered menu
    QAction* exec(const QPoint& pos, QAction* at = 0);

private:

    void setSelectedIds(imageIds& ids);
    bool actionExists(QAction* action);

private:

    ContextMenuHelperPriv* const d;
};

} // namespace Digikam

#endif /* CONTEXTMENUHELPER_H */
