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

#include "contextmenuhelper.h"
#include "contextmenuhelper.moc"

// Qt includes.

#include <QAction>
#include <QClipboard>
#include <QMimeData>
#include <QString>

// KDE includes.

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <kstandardaction.h>

// LibKIPI includes.

#include <libkipi/plugin.h>
#include <libkipi/pluginloader.h>

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "imageinfo.h"
#include "lighttablewindow.h"
#include "ratingpopupmenu.h"
#include "tagspopupmenu.h"

namespace Digikam
{

class ContextMenuHelperPriv
{
public:

    ContextMenuHelperPriv()
    {
        ac = 0;
    }
    KActionCollection* ac;
};

ContextMenuHelper::ContextMenuHelper(QObject* parent, KActionCollection* actionCollection)
                 : QObject(parent), d(new ContextMenuHelperPriv)
{
    d->ac = actionCollection;
}

ContextMenuHelper::~ContextMenuHelper()
{
    delete d;
}

void ContextMenuHelper::addAction(KMenu& menu, const char* name, bool addDisabled)
{
    QAction* action = 0;
    action = d->ac->action(name);
    addAction(menu, action, addDisabled);
}

void ContextMenuHelper::addAction(KMenu& menu, QAction* action, bool addDisabled)
{
    if (!action)
        return;

    if (action->isEnabled() || addDisabled)
        menu.addAction(action);
}

void ContextMenuHelper::addAction(KMenu& menu, QAction* action, QObject* recv, const char* slot,
                                  bool addDisabled)
{
    connect(action, SIGNAL(triggered()),
            recv, slot);
    addAction(menu, action, addDisabled);
}

void ContextMenuHelper::addActionLightTable(KMenu& menu)
{
    QAction* action = 0;
    QStringList ltActionNames;
    ltActionNames << QString("image_add_to_lighttable")
                  << QString("image_lighttable");

    if (LightTableWindow::lightTableWindowCreated() && !LightTableWindow::lightTableWindow()->isEmpty())
        action = d->ac->action(ltActionNames.at(0));
    else
        action = d->ac->action(ltActionNames.at(1));

    addAction(menu, action);
}

void ContextMenuHelper::addActionThumbnail(KMenu& menu, const QList<qlonglong>& selectedIds, Album* album)
{
    QAction* thumbnailAction = 0;
    if (album && selectedIds.count() == 1)
    {
        if (album->type() == Album::PHYSICAL )
            thumbnailAction = new QAction(i18n("Set as Album Thumbnail"), this);
        else if (album->type() == Album::TAG )
            thumbnailAction = new QAction(i18n("Set as Tag Thumbnail"), this);

        addAction(menu, thumbnailAction);
        menu.addSeparator();
    }
}

void ContextMenuHelper::addServicesMenu(KMenu& menu, const ImageInfo& item,
                                        QMap<QAction*, KService::Ptr> &servicesMap)
{
    KMimeType::Ptr mimePtr = KMimeType::findByUrl(item.fileUrl(), 0, true, true);
    const KService::List offers = KMimeTypeTrader::self()->query(mimePtr->name());
    KMenu *servicesMenu = new KMenu(&menu);
    QAction* menuAction = servicesMenu->menuAction();
    menuAction->setText(i18n("Open With"));

    foreach (KService::Ptr ptr, offers)
    {
        QAction *serviceAction = servicesMenu->addAction(SmallIcon(ptr->icon()), ptr->name());
        servicesMap[serviceAction] = ptr;
    }

    if (servicesMenu->isEmpty())
        menuAction->setEnabled(false);

    addAction(menu, menuAction);
}

void ContextMenuHelper::addKipiActions(KMenu &menu)
{
    KIPI::PluginLoader* kipiPluginLoader      = KIPI::PluginLoader::instance();
    KIPI::PluginLoader::PluginList pluginList = kipiPluginLoader->pluginList();

    foreach (KIPI::PluginLoader::Info* info, pluginList)
    {
        KIPI::Plugin* plugin = info->plugin();

        if (plugin && info->name() == "JPEGLossless")
        {
            kDebug(50003) << "Found JPEGLossless plugin" << endl;

            QList<KAction*> actionList = plugin->actions();
            foreach (KAction* action, actionList)
            {
                if (action->objectName().toLatin1() == QString::fromLatin1("jpeglossless_rotate"))
                    menu.addAction(action);
            }
        }
    }
}

void ContextMenuHelper::addAssignTagsMenu(KMenu& menu, const QList<qlonglong>& selectedImageIDs,
                                          QObject* recv, const char* slot)
{
    KMenu* assignTagsPopup = new TagsPopupMenu(selectedImageIDs, TagsPopupMenu::ASSIGN, &menu);
    assignTagsPopup->menuAction()->setText(i18n("Assign Tag"));
    menu.addMenu(assignTagsPopup);

    connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
            recv, slot);
}

void ContextMenuHelper::addRemoveTagsMenu(KMenu& menu, const QList<qlonglong>& selectedImageIDs,
                                          QObject* recv, const char* slot)
{
    KMenu* removeTagsPopup = new TagsPopupMenu(selectedImageIDs, TagsPopupMenu::REMOVE, &menu);
    removeTagsPopup->menuAction()->setText(i18n("Remove Tag"));
    menu.addMenu(removeTagsPopup);

    // Performance: Only check for tags if there are <250 images selected
    // Also disable the remove Tag popup menu, if there are no tags at all.
    if (selectedImageIDs.count() > 250 ||
            !DatabaseAccess().db()->hasTags(selectedImageIDs))
        removeTagsPopup->menuAction()->setEnabled(false);

    connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
            recv, slot);
}

void ContextMenuHelper::addRatingMenu(KMenu& menu, QObject* recv, const char* slot)
{
    KMenu* ratingMenu = new RatingPopupMenu(&menu);
    ratingMenu->menuAction()->setText(i18n("Assign Rating"));
    menu.addMenu(ratingMenu);

    connect(ratingMenu, SIGNAL(signalRatingChanged(int)),
            recv, slot);
}

void ContextMenuHelper::addImportMenu(KMenu& menu)
{
    KMenu* menuImport = new KMenu(i18n("Import"), &menu);
    const QList<QAction*> importActions = DigikamApp::getinstance()->menuImportActions();

    if(!importActions.isEmpty())
        menuImport->addActions(importActions);

    menu.addMenu(menuImport);
}

void ContextMenuHelper::addExportMenu(KMenu& menu)
{
    KMenu* menuExport = new KMenu(i18n("Export"), &menu);
    const QList<QAction*> exportActions = DigikamApp::getinstance()->menuExportActions();

    if(!exportActions.isEmpty())
        menuExport->addActions(exportActions);

    menu.addMenu(menuExport);
}

void ContextMenuHelper::addBatchMenu(KMenu& menu)
{
    KMenu* menuKIPIBatch = new KMenu(i18n("Batch Process"), &menu);
    const QList<QAction*>& batchActions = DigikamApp::getinstance()->menuBatchActions();

    if(!batchActions.isEmpty())
        menuKIPIBatch->addActions(batchActions);

    menu.addMenu(menuKIPIBatch);
}

void ContextMenuHelper::addAlbumActions(KMenu& menu)
{
    const QList<QAction*>& albumActions = DigikamApp::getinstance()->menuAlbumActions();
    if(!albumActions.isEmpty())
        menu.addActions(albumActions);
}

void ContextMenuHelper::addGotoMenu(KMenu& menu, const ImageInfo& item, QObject* recv, const char* slot)
{
    KMenu *gotoMenu    = new KMenu(&menu);
    QAction *gotoAlbum = new QAction(SmallIcon("folder-image"),        i18n("Album"), gotoMenu);
    QAction *gotoDate  = new QAction(SmallIcon("view-calendar-month"), i18n("Date"),  gotoMenu);
    gotoMenu->addAction(gotoAlbum);
    gotoMenu->addAction(gotoDate);

    connect(gotoAlbum, SIGNAL(triggered()),
            recv, slot);

    connect(gotoDate, SIGNAL(triggered()),
            recv, slot);

    // We disable the goto menu when multiple images are selected.
    // Therefore selectedImageIDs contains only the currently selected image.
    QList<qlonglong> selectedImageIDs;
    selectedImageIDs << item.id();

    TagsPopupMenu *gotoTagsPopup = new TagsPopupMenu(selectedImageIDs, TagsPopupMenu::DISPLAY, gotoMenu);
    QAction *gotoTag = gotoMenu->addMenu(gotoTagsPopup);
    gotoTag->setIcon(SmallIcon("tag"));
    gotoTag->setText(i18n("Tag"));

    // Disable the goto Tag popup menu, if there are no tags at all.
    if (!DatabaseAccess().db()->hasTags(selectedImageIDs))
        gotoTag->setEnabled(false);

    connect(gotoTagsPopup, SIGNAL(signalTagActivated(int)),
            recv, slot);
//    connect(gotoTagsPopup, SIGNAL(signalTagActivated(int)),
//            this, SLOT(slotGotoTag(int)));

    Album* currentAlbum = AlbumManager::instance()->currentAlbum();
    if (currentAlbum->type() == Album::PHYSICAL )
    {
        // If the currently selected album is the same as album to
        // which the image belongs, then disable the "Go To" Album.
        // (Note that in recursive album view these can be different).
        if (item.albumId() == currentAlbum->id())
            gotoAlbum->setEnabled(false);
    }
    else if (currentAlbum->type() == Album::DATE )
    {
        gotoDate->setEnabled(false);
    }
    QAction *gotoMenuAction = gotoMenu->menuAction();
    gotoMenuAction->setIcon(SmallIcon("go-jump"));
    gotoMenuAction->setText(i18n("Go To"));

    addAction(menu, gotoMenuAction);
}

void ContextMenuHelper::addActionCopy(KMenu& menu, QObject* recv, const char* slot)
{
    KAction* copy = KStandardAction::copy(recv, slot, &menu);
    addAction(menu, copy);
}

void ContextMenuHelper::addActionPaste(KMenu& menu, QObject* recv, const char* slot)
{
    KAction* paste = KStandardAction::paste(recv, slot, &menu);

    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);
    if(!data || !KUrl::List::canDecode(data))
        paste->setEnabled(false);
    addAction(menu, paste);
}

void ContextMenuHelper::addActionDelete(KMenu& menu, QObject* recv, const char* slot, int quantity)
{
    QAction *trashAction = new QAction(SmallIcon("user-trash"), i18np("Move to Trash",
                                       "Move %1 Files to Trash", quantity), &menu);
    connect(trashAction, SIGNAL(triggered()),
            recv, slot);
    addAction(menu, trashAction);
}

} // namespace Digikam
