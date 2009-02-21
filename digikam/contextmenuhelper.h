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
class QString;

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

    enum StdActions
    {
        Unknown = 0,
        AssignTags,
        RemoveTags,
        AssignRating,
        GotoAlbum,
        GotoTag,
        GotoDate,
        SetThumbnail
    };

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
    virtual void addAction(const char* name, bool addDisabled = false);

    // add temporary action
    virtual void addAction(QAction* action, bool addDisabled = false);
    virtual void addAction(QAction* action, QObject* recv, const char* slot, bool addDisabled = false);

    // add standard actions
    virtual void addActionCopy(QObject* recv, const char* slot);
    virtual void addActionPaste(QObject* recv, const char* slot);
    virtual void addActionItemDelete(QObject* recv, const char* slot, int quantity = 1);

    // add special actions
    virtual void addActionLightTable();
    virtual void addActionThumbnail(imageIds& imageIds, Album* album);

    // add special menus
    virtual void addKipiActions();
    virtual void addServicesMenu(const ImageInfo&, QMap<QAction*, KService::Ptr>&);
    virtual void addGotoMenu(imageIds& imageIDs/*, QAction*, QAction* */);
    virtual void addQueueManagerMenu();

    virtual void addAssignTagsMenu(imageIds& imageIDs, QObject* recv, const char* slot);
    virtual void addRemoveTagsMenu(imageIds& imageIDs, QObject* recv, const char* slot);
    virtual void addRatingMenu(QObject* recv, const char* slot);

    virtual void addImportMenu();
    virtual void addExportMenu();
    virtual void addBatchMenu();
    virtual void addAlbumActions();

    // execute the registered menu
    virtual QAction* exec(int &id);

private:

    ContextMenuHelperPriv* const d;
};

} // namespace Digikam

#endif /* CONTEXTMENUHELPER_H */
