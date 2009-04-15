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

// Qt includes

#include <QAction>
#include <QClipboard>
#include <QMap>
#include <QMenu>
#include <QMimeData>
#include <QString>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <kstandardaction.h>

// LibKIPI includes

#include <libkipi/plugin.h>
#include <libkipi/pluginloader.h>

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "imageinfo.h"
#include "lighttablewindow.h"
#include "queuemgrwindow.h"
#include "ratingpopupmenu.h"
#include "tagspopupmenu.h"

namespace Digikam
{

class ContextMenuHelperPriv
{
public:

    ContextMenuHelperPriv()
    {
        gotoAlbumAction     = 0;
        gotoDateAction      = 0;
        menu                = 0;
        setThumbnailAction  = 0;
        stdActionCollection = 0;
    }

    QAction*            gotoAlbumAction;
    QAction*            gotoDateAction;
    QAction*            setThumbnailAction;

    QList<qlonglong>    selectedIds;

    QMap<int, QAction*> queueActions;

    QMenu*              menu;

    KActionCollection*  stdActionCollection;
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
    if (d->setThumbnailAction)
        return;
    setSelectedIds(ids);

    if (album && ids.count() == 1)
    {
        if (album->type() == Album::PHYSICAL )
            d->setThumbnailAction = new QAction(i18n("Set as Album Thumbnail"), this);
        else if (album->type() == Album::TAG )
            d->setThumbnailAction = new QAction(i18n("Set as Tag Thumbnail"), this);

        addAction(d->setThumbnailAction);
        d->menu->addSeparator();
    }
}

void ContextMenuHelper::addServicesMenu(const ImageInfo& item, QMap<QAction*, KService::Ptr> &servicesMap)
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

        if (plugin && info->shouldLoad() && info->name() == "JPEGLossless")
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

void ContextMenuHelper::addAssignTagsMenu(imageIds& ids)
{
    setSelectedIds(ids);

    KMenu* assignTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::ASSIGN, d->menu);
    assignTagsPopup->menuAction()->setText(i18n("Assign Tag"));
    d->menu->addMenu(assignTagsPopup);

    connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalAssignTag(int)));
}

void ContextMenuHelper::addRemoveTagsMenu(imageIds& ids)
{
    setSelectedIds(ids);

    KMenu* removeTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::REMOVE, d->menu);
    removeTagsPopup->menuAction()->setText(i18n("Remove Tag"));
    d->menu->addMenu(removeTagsPopup);

    // Performance: Only check for tags if there are <250 images selected
    // Also disable the remove Tag popup menu, if there are no tags at all.
    if (ids.count() > 250 ||
            !DatabaseAccess().db()->hasTags(ids))
        removeTagsPopup->menuAction()->setEnabled(false);

    connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalRemoveTag(int)));
}

void ContextMenuHelper::addRatingMenu()
{
    KMenu* ratingMenu = new RatingPopupMenu(d->menu);
    ratingMenu->menuAction()->setText(i18n("Assign Rating"));
    d->menu->addMenu(ratingMenu);

    connect(ratingMenu, SIGNAL(signalRatingChanged(int)),
            this, SIGNAL(signalAssignRating(int)));
}

void ContextMenuHelper::addCreateTagFromAddressbookMenu()
{
#ifdef HAVE_KDEPIMLIBS
    d->ABCMenu = new KMenu(this);

    connect( d->ABCMenu, SIGNAL( aboutToShow() ),
             this, SLOT( slotABCContextMenu() ) );

    popmenu.addMenu(d->ABCMenu);
    d->ABCMenu->menuAction()->setIcon(SmallIcon("tag-addressbook"));
    d->ABCMenu->menuAction()->setText(i18n("Create Tag From Address Book"));
#endif // HAVE_KDEPIMLIBS
}

void ContextMenuHelper::addSelectTagsMenu(Q3ListViewItem *item)
{
    KMenu *selectTagsMenu         = new KMenu(i18nc("select tags menu", "Select"), d->menu);
    QAction *selectChildrenAction = 0;
    QAction *selectParentsAction  = 0;
    QAction *selectAllTagsAction  = 0;
    selectAllTagsAction           = selectTagsMenu->addAction(i18n("All Tags"));
    if (item)
    {
        selectTagsMenu->addSeparator();
        selectChildrenAction = selectTagsMenu->addAction(i18n("Children"));
        selectParentsAction  = selectTagsMenu->addAction(i18n("Parents"));
        // TODO: handle selectAllTagsAction
    }
    d->menu->addMenu(selectTagsMenu);
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

void ContextMenuHelper::addGotoMenu(imageIds& ids)
{
    if (d->gotoAlbumAction && d->gotoDateAction)
        return;
    setSelectedIds(ids);

    // when more then one item is selected, don't add the menu
    if (d->selectedIds.count() > 1) return;

    d->gotoAlbumAction = new QAction(SmallIcon("folder-image"),        i18n("Album"), this);
    d->gotoDateAction  = new QAction(SmallIcon("view-calendar-month"), i18n("Date"),  this);

    // the currently selected image is always the first item
    ImageInfo item(d->selectedIds.first());

    KMenu *gotoMenu  = new KMenu(d->menu);
    gotoMenu->addAction(d->gotoAlbumAction);
    gotoMenu->addAction(d->gotoDateAction);

    TagsPopupMenu *gotoTagsPopup = new TagsPopupMenu(d->selectedIds, TagsPopupMenu::DISPLAY, gotoMenu);
    QAction *gotoTag = gotoMenu->addMenu(gotoTagsPopup);
    gotoTag->setIcon(SmallIcon("tag"));
    gotoTag->setText(i18n("Tag"));

    // Disable the goto Tag popup menu, if there are no tags at all.
    if (!DatabaseAccess().db()->hasTags(d->selectedIds))
        gotoTag->setEnabled(false);

    Album* currentAlbum = AlbumManager::instance()->currentAlbum();

    if (currentAlbum->type() == Album::PHYSICAL)
    {
        // If the currently selected album is the same as album to
        // which the image belongs, then disable the "Go To" Album.
        // (Note that in recursive album view these can be different).
        if (item.albumId() == currentAlbum->id())
            d->gotoAlbumAction->setEnabled(false);
    }
    else if (currentAlbum->type() == Album::DATE)
    {
        d->gotoDateAction->setEnabled(false);
    }
    QAction *gotoMenuAction = gotoMenu->menuAction();
    gotoMenuAction->setIcon(SmallIcon("go-jump"));
    gotoMenuAction->setText(i18n("Go To"));

    connect(gotoTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalGotoTag(int)));

    addAction(gotoMenuAction);
}

void ContextMenuHelper::addQueueManagerMenu()
{
    KMenu* bqmMenu = new KMenu(i18n("Batch Queue Manager"), d->menu);
    QStringList queueActions;
    queueActions << QString("image_add_to_current_queue")
                 << QString("image_add_to_new_queue");
    bqmMenu->addAction(d->stdActionCollection->action(queueActions.at(0)));
    bqmMenu->addAction(d->stdActionCollection->action(queueActions.at(1)));

    // if queue list is empty, do not display the queue submenu
    if (QueueMgrWindow::queueManagerWindowCreated() &&
       !QueueMgrWindow::queueManagerWindow()->queuesMap().isEmpty())
    {
        QueueMgrWindow* qmw = QueueMgrWindow::queueManagerWindow();
        KMenu* queueMenu    = new KMenu(i18n("Add to Existing Queue"), bqmMenu);

        // queueActions is used by the exec() method to emit an appropriate signal.
        // Reset the map before filling in the actions.
        if (!d->queueActions.isEmpty())
            d->queueActions.clear();

        QList<QAction*> queueList;

        // get queue list from BQM window, do not access it directly, it might crash
        // when the list is changed
        QMap<int, QString> qmwMap = qmw->queuesMap();
        for (QMap<int, QString>::iterator it = qmwMap.begin(); it != qmwMap.end(); ++it)
        {
            QAction* action = new QAction(it.value(), this);
            queueList << action;
            d->queueActions[it.key()] = action;
        }
        queueMenu->addActions(queueList);
        bqmMenu->addMenu(queueMenu);
    }
    d->menu->addMenu(bqmMenu);
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

QAction* ContextMenuHelper::exec(const QPoint& pos, QAction* at)
{
    QAction* choice = d->menu->exec(pos, at);
    if (choice)
    {
        if (choice == d->gotoAlbumAction)
        {
            if (!d->selectedIds.isEmpty())
            {
                ImageInfo selectedItem(d->selectedIds.first());
                emit signalGotoAlbum(selectedItem);
            }
        }
        else if (choice == d->gotoDateAction)
        {
            if (!d->selectedIds.isEmpty())
            {
                ImageInfo selectedItem(d->selectedIds.first());
                emit signalGotoDate(selectedItem);
            }
        }
        else if (choice == d->setThumbnailAction)
        {
            if (!d->selectedIds.isEmpty())
            {
                ImageInfo selectedItem(d->selectedIds.first());
                emit signalSetThumbnail(selectedItem);
            }
        }
        else
        {
            for (QMap<int, QAction*>::iterator it = d->queueActions.begin();
                 it != d->queueActions.end(); ++it)
            {
                if (choice == it.value())
                {
                    emit signalAddToExistingQueue(it.key());
                    break;
                }
            }
        }
    }
    return choice;
}

void ContextMenuHelper::setSelectedIds(imageIds &ids)
{
    if (d->selectedIds.isEmpty())
        d->selectedIds = ids;
}

} // namespace Digikam
