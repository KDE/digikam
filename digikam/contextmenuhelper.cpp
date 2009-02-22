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
#include <QMenu>
#include <QMap>
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
        stdActionCollection = 0;
        menu                = 0;
    }
    KActionCollection*  stdActionCollection;
    QMenu*              menu;
    QMap<int, QAction*> actions;
};

ContextMenuHelper::ContextMenuHelper(QMenu* parent, KActionCollection* actionCollection)
                 : QObject(parent), d(new ContextMenuHelperPriv)
{
    d->menu = parent;

    if (!actionCollection)
        d->stdActionCollection = DigikamApp::getinstance()->actionCollection();
    else
        d->stdActionCollection = actionCollection;
}

ContextMenuHelper::~ContextMenuHelper()
{
    delete d;
}

void ContextMenuHelper::addAction(const char* name, bool addDisabled)
{
    QAction* action = 0;
    action = d->stdActionCollection->action(name);
    addAction(action, addDisabled);
}

void ContextMenuHelper::addAction(QAction* action, bool addDisabled)
{
    if (!action)
        return;

    if (action->isEnabled() || addDisabled)
        d->menu->addAction(action);
}

void ContextMenuHelper::addAction(QAction* action, QObject* recv, const char* slot,
                                  bool addDisabled)
{
    if (!action)
        return;

    connect(action, SIGNAL(triggered()), recv, slot);
    addAction(action, addDisabled);
}

void ContextMenuHelper::addActionLightTable()
{
    QAction* action = 0;
    QStringList ltActionNames;
    ltActionNames << QString("image_add_to_lighttable")
                  << QString("image_lighttable");

    if (LightTableWindow::lightTableWindowCreated() && !LightTableWindow::lightTableWindow()->isEmpty())
        action = d->stdActionCollection->action(ltActionNames.at(0));
    else
        action = d->stdActionCollection->action(ltActionNames.at(1));

    addAction(action);
}

void ContextMenuHelper::addActionThumbnail(imageIds& ids, Album* album)
{
    QAction* thumbnailAction = 0;
    if (album && ids.count() == 1)
    {
        if (album->type() == Album::PHYSICAL )
            thumbnailAction = new QAction(i18n("Set as Album Thumbnail"), this);
        else if (album->type() == Album::TAG )
            thumbnailAction = new QAction(i18n("Set as Tag Thumbnail"), this);

        addAction(thumbnailAction);
        d->menu->addSeparator();

        d->actions.insert(SetThumbnail, thumbnailAction);
    }
}

void ContextMenuHelper::addServicesMenu(const ImageInfo& item,
                                        QMap<QAction*, KService::Ptr> &servicesMap)
{
    KMimeType::Ptr mimePtr = KMimeType::findByUrl(item.fileUrl(), 0, true, true);
    const KService::List offers = KMimeTypeTrader::self()->query(mimePtr->name());
    KMenu *servicesMenu = new KMenu(d->menu);
    QAction* menuAction = servicesMenu->menuAction();
    menuAction->setText(i18n("Open With"));

    foreach (KService::Ptr ptr, offers)
    {
        QAction *serviceAction = servicesMenu->addAction(SmallIcon(ptr->icon()), ptr->name());
        servicesMap[serviceAction] = ptr;
    }

    if (servicesMenu->isEmpty())
        menuAction->setEnabled(false);

    addAction(menuAction);
}

void ContextMenuHelper::addKipiActions()
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
                    d->menu->addAction(action);
            }
        }
    }
}

void ContextMenuHelper::addAssignTagsMenu(imageIds& ids, QObject* recv, const char* slot)
{
    KMenu* assignTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::ASSIGN, d->menu);
    assignTagsPopup->menuAction()->setText(i18n("Assign Tag"));
    d->menu->addMenu(assignTagsPopup);

    connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
            recv, slot);
}

void ContextMenuHelper::addRemoveTagsMenu(imageIds& ids, QObject* recv, const char* slot)
{
    KMenu* removeTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::REMOVE, d->menu);
    removeTagsPopup->menuAction()->setText(i18n("Remove Tag"));
    d->menu->addMenu(removeTagsPopup);

    // Performance: Only check for tags if there are <250 images selected
    // Also disable the remove Tag popup menu, if there are no tags at all.
    if (ids.count() > 250 ||
            !DatabaseAccess().db()->hasTags(ids))
        removeTagsPopup->menuAction()->setEnabled(false);

    connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
            recv, slot);
}

void ContextMenuHelper::addRatingMenu(QObject* recv, const char* slot)
{
    KMenu* ratingMenu = new RatingPopupMenu(d->menu);
    ratingMenu->menuAction()->setText(i18n("Assign Rating"));
    d->menu->addMenu(ratingMenu);

    connect(ratingMenu, SIGNAL(signalRatingChanged(int)),
            recv, slot);
}

void ContextMenuHelper::addImportMenu()
{
    KMenu* menuImport = new KMenu(i18n("Import"), d->menu);
    const QList<QAction*> importActions = DigikamApp::getinstance()->menuImportActions();

    if(!importActions.isEmpty())
        menuImport->addActions(importActions);

    d->menu->addMenu(menuImport);
}

void ContextMenuHelper::addExportMenu()
{
    KMenu* menuExport = new KMenu(i18n("Export"), d->menu);
    const QList<QAction*> exportActions = DigikamApp::getinstance()->menuExportActions();

    if(!exportActions.isEmpty())
        menuExport->addActions(exportActions);

    d->menu->addMenu(menuExport);
}

void ContextMenuHelper::addBatchMenu()
{
    KMenu* menuKIPIBatch = new KMenu(i18n("Batch Process"), d->menu);
    const QList<QAction*>& batchActions = DigikamApp::getinstance()->menuBatchActions();

    if(!batchActions.isEmpty())
        menuKIPIBatch->addActions(batchActions);

    d->menu->addMenu(menuKIPIBatch);
}

void ContextMenuHelper::addAlbumActions()
{
    const QList<QAction*>& albumActions = DigikamApp::getinstance()->menuAlbumActions();
    if(!albumActions.isEmpty())
        d->menu->addActions(albumActions);
}

void ContextMenuHelper::addGotoMenu(imageIds& ids/*, QAction* gotoAlbum, QAction* gotoDate*/)
{
    // when more then one item is selected, don't add the menu
    if (ids.count() > 1) return;

    QAction  *viewAction, *gotoAlbum, *gotoDate = 0;
    viewAction = new QAction(SmallIcon("viewimage"),           i18n("View"),  this);
    gotoAlbum  = new QAction(SmallIcon("folder-image"),        i18n("Album"), this);
    gotoDate   = new QAction(SmallIcon("view-calendar-month"), i18n("Date"),  this);
    d->actions.insert(GotoAlbum, gotoAlbum);
    d->actions.insert(GotoDate,  gotoDate);

//    if (!gotoAlbum || !gotoDate) return;

    // the currently selected image is always the first item
    ImageInfo item(ids.first());

    KMenu *gotoMenu  = new KMenu(d->menu);
    gotoMenu->addAction(gotoAlbum);
    gotoMenu->addAction(gotoDate);

    TagsPopupMenu *gotoTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::DISPLAY, gotoMenu);
    QAction *gotoTag = gotoMenu->addMenu(gotoTagsPopup);
    gotoTag->setIcon(SmallIcon("tag"));
    gotoTag->setText(i18n("Tag"));

    // Disable the goto Tag popup menu, if there are no tags at all.
    if (!DatabaseAccess().db()->hasTags(ids))
        gotoTag->setEnabled(false);

    connect(gotoTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotGotoTag(int)));

    Album* currentAlbum = AlbumManager::instance()->currentAlbum();

    if (currentAlbum->type() == Album::PHYSICAL)
    {
        // If the currently selected album is the same as album to
        // which the image belongs, then disable the "Go To" Album.
        // (Note that in recursive album view these can be different).
        if (item.albumId() == currentAlbum->id())
            gotoAlbum->setEnabled(false);
    }
    else if (currentAlbum->type() == Album::DATE)
    {
        gotoDate->setEnabled(false);
    }
    QAction *gotoMenuAction = gotoMenu->menuAction();
    gotoMenuAction->setIcon(SmallIcon("go-jump"));
    gotoMenuAction->setText(i18n("Go To"));

    addAction(gotoMenuAction);
}

void ContextMenuHelper::addQueueManagerMenu()
{
    KMenu* queueMenu = new KMenu(i18n("Queue Manager"), d->menu);
    QStringList queueActions;
    queueActions << QString("image_add_to_current_queue")
                 << QString("image_add_to_new_queue");
    queueMenu->addAction(d->stdActionCollection->action(queueActions.at(0)));
    queueMenu->addAction(d->stdActionCollection->action(queueActions.at(1)));
    d->menu->addMenu(queueMenu);
}

void ContextMenuHelper::addActionCopy(QObject* recv, const char* slot)
{
    KAction* copy = KStandardAction::copy(recv, slot, d->menu);
    addAction(copy);
}

void ContextMenuHelper::addActionPaste(QObject* recv, const char* slot)
{
    KAction* paste = KStandardAction::paste(recv, slot, d->menu);

    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);
    if(!data || !KUrl::List::canDecode(data))
        paste->setEnabled(false);
    addAction(paste);
}

void ContextMenuHelper::addActionItemDelete(QObject* recv, const char* slot, int quantity)
{
    QAction *trashAction = new QAction(SmallIcon("user-trash"), i18np("Move to Trash",
                                       "Move %1 Files to Trash", quantity), d->menu);
    connect(trashAction, SIGNAL(triggered()),
            recv, slot);
    addAction(trashAction);
}

QAction* ContextMenuHelper::exec(const QPoint& pos, int& id, QAction* at)
{
    QAction* choice = d->menu->exec(pos, at);
    id = Unknown;

    QMapIterator<int, QAction*> it(d->actions);
    while (it.hasNext())
    {
        it.next();
        if (it.value() == choice)
        {
            id = it.key();
            break;
        }
    }
    return choice;
}

} // namespace Digikam
