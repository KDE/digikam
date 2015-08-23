/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class
 *
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2010-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "contextmenuhelper.moc"

// Qt includes

#include <QAction>
#include <QWidgetAction>
#include <QClipboard>
#include <QMap>
#include <QMenu>
#include <QMimeData>
#include <QPointer>
#include <QString>
#include <QTimer>

// KDE includes

#include <kaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <kopenwithdialog.h>
#include <krun.h>
#include <kstandardaction.h>

#ifdef HAVE_KDEPIMLIBS

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#endif

#include <Akonadi/Item>
#include <Akonadi/Contact/ContactSearchJob>

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // HAVE_KDEPIMLIBS

// Local includes

#include "album.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albummodificationhelper.h"
#include "abstractalbummodel.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "fileoperation.h"
#include "imageinfo.h"
#include "imagefiltermodel.h"
#include "lighttablewindow.h"
#include "queuemgrwindow.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "ratingwidget.h"
#include "tagmodificationhelper.h"
#include "tagspopupmenu.h"
#include "fileactionmngr.h"
#include "tagscache.h"
#include "dimg.h"

#ifdef HAVE_KIPI
#include "kipipluginloader.h"
#endif /* HAVE_KIPI */

namespace Digikam
{

class ContextMenuHelper::Private
{
public:

    explicit Private(ContextMenuHelper* const q) :
        gotoAlbumAction(0),
        gotoDateAction(0),
        setThumbnailAction(0),
        imageFilterModel(0),
        albumModel(0),
        parent(0),
        ABCmenu(0),
        stdActionCollection(0),
        q(q)
    {}

    QAction*                     gotoAlbumAction;
    QAction*                     gotoDateAction;
    QAction*                     setThumbnailAction;

    QList<qlonglong>             selectedIds;
    KUrl::List                   selectedItems;

    QMap<int, QAction*>          queueActions;
    QMap<QString, KService::Ptr> servicesMap;

    ImageFilterModel*            imageFilterModel;
    AbstractCheckableAlbumModel* albumModel;

    QMenu*                       parent;
    QMenu*                       ABCmenu;

    KActionCollection*           stdActionCollection;

    ContextMenuHelper*           q;

public:

    QModelIndex indexForAlbumFromAction(QObject* sender) const
    {
        QAction* action = 0;

        if ((action = qobject_cast<QAction*>(sender)))
        {
            Album* const album = action->data().value<AlbumPointer<> >();
            return albumModel->indexForAlbum(album);
        }

        return QModelIndex();
    }

    QAction* copyFromMainCollection(const char* name) const
    {
        QAction* const mainAction = stdActionCollection->action(name);

        if (!mainAction)
        {
            return 0;
        }

        QAction* const action = new QAction(mainAction->icon(), mainAction->text(), q);
        action->setToolTip(mainAction->toolTip());
        return action;
    }
};

ContextMenuHelper::ContextMenuHelper(QMenu* const parent, KActionCollection* const actionCollection)
    : QObject(parent),
      d(new Private(this))
{
    d->parent = parent;

    if (!actionCollection)
    {
        d->stdActionCollection = DigikamApp::instance()->actionCollection();
    }
    else
    {
        d->stdActionCollection = actionCollection;
    }
}

ContextMenuHelper::~ContextMenuHelper()
{
    delete d;
}

void ContextMenuHelper::addAction(const char* name, bool addDisabled)
{
    QAction* const action = d->stdActionCollection->action(name);
    addAction(action, addDisabled);
}

void ContextMenuHelper::addAction(QAction* action, bool addDisabled)
{
    if (!action)
    {
        return;
    }

    if (action->isEnabled() || addDisabled)
    {
        d->parent->addAction(action);
    }
}

void ContextMenuHelper::addSubMenu(KMenu* subMenu)
{
    d->parent->addMenu(subMenu);
}

void ContextMenuHelper::addSeparator()
{
    d->parent->addSeparator();
}

void ContextMenuHelper::addAction(QAction* action, QObject* recv, const char* slot,
                                  bool addDisabled)
{
    if (!action)
    {
        return;
    }

    connect(action, SIGNAL(triggered()), recv, slot);
    addAction(action, addDisabled);
}

void ContextMenuHelper::addStandardActionLightTable()
{
    QAction* action = 0;
    QStringList ltActionNames;
    ltActionNames << QString("image_add_to_lighttable")
                  << QString("image_lighttable");

    if (LightTableWindow::lightTableWindowCreated() && !LightTableWindow::lightTableWindow()->isEmpty())
    {
        action = d->stdActionCollection->action(ltActionNames.at(0));
    }
    else
    {
        action = d->stdActionCollection->action(ltActionNames.at(1));
    }

    addAction(action);
}

void ContextMenuHelper::addStandardActionThumbnail(const imageIds &ids, Album* album)
{
    if (d->setThumbnailAction)
    {
        return;
    }

    setSelectedIds(ids);

    if (album && ids.count() == 1)
    {
        if (album->type() == Album::PHYSICAL)
        {
            d->setThumbnailAction = new QAction(i18n("Set as Album Thumbnail"), this);
        }
        else if (album->type() == Album::TAG)
        {
            d->setThumbnailAction = new QAction(i18n("Set as Tag Thumbnail"), this);
        }

        addAction(d->setThumbnailAction);
        d->parent->addSeparator();
    }
}

void ContextMenuHelper::addServicesMenu(const KUrl::List& selectedItems)
{
    setSelectedItems(selectedItems);

    KService::List offers = FileOperation::servicesForOpenWith(selectedItems);

    if (!offers.isEmpty())
    {
        KMenu* const servicesMenu = new KMenu(d->parent);
        qDeleteAll(servicesMenu->actions());

        QAction* const serviceAction = servicesMenu->menuAction();
        serviceAction->setText(i18n("Open With"));

        foreach(const KService::Ptr& service, offers)
        {
            QString name          = service->name().replace('&', "&&");
            QAction* const action = servicesMenu->addAction(name);
            action->setIcon(KIcon(service->icon()));
            action->setData(service->name());
            d->servicesMap[name]  = service;
        }

        servicesMenu->addSeparator();
        servicesMenu->addAction(i18n("Other..."));

        addAction(serviceAction);

        connect(servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }
    else
    {
        QAction* const serviceAction = new QAction(i18n("Open With..."), this);
        addAction(serviceAction);

        connect(serviceAction, SIGNAL(triggered()),
                this, SLOT(slotOpenWith()));
    }
}

void ContextMenuHelper::slotOpenWith()
{
    // call the slot with an "empty" action
    slotOpenWith(0);
}

void ContextMenuHelper::slotOpenWith(QAction* action)
{
    KService::Ptr service;
    KUrl::List list = d->selectedItems;
    QString name    = action ? action->data().toString() : QString();

    if (name.isEmpty())
    {
        QPointer<KOpenWithDialog> dlg = new KOpenWithDialog(list);

        if (dlg->exec() != KOpenWithDialog::Accepted)
        {
            delete dlg;
            return;
        }

        service = dlg->service();

        if (!service)
        {
            // User entered a custom command
            if (!dlg->text().isEmpty())
            {
                KRun::run(dlg->text(), list, d->parent);
            }

            delete dlg;
            return;
        }

        delete dlg;
    }
    else
    {
        service = d->servicesMap[name];
    }

    KRun::run(*service, list, d->parent);
}

bool ContextMenuHelper::imageIdsHaveSameCategory(const imageIds& ids, DatabaseItem::Category category)
{
    bool sameCategory = true;
    QVariantList varList;

    foreach(const qlonglong& id, ids)
    {
        varList = DatabaseAccess().db()->getImagesFields(id, DatabaseFields::Category);

        if (varList.isEmpty() ||
            (DatabaseItem::Category)varList.first().toInt() != category)
        {
            sameCategory = false;
            break;
        }
    }
    return sameCategory;
}

void ContextMenuHelper::addActionNewTag(TagModificationHelper* helper, TAlbum* tag)
{
    QAction* const newTagAction = new QAction(SmallIcon("tag-new"), i18n("New Tag..."), this);
    addAction(newTagAction);
    helper->bindTag(newTagAction, tag);

    connect(newTagAction, SIGNAL(triggered()),
            helper, SLOT(slotTagNew()));
}

void ContextMenuHelper::addActionDeleteTag(TagModificationHelper* helper, TAlbum* tag)
{
    QAction* const deleteTagAction = new QAction(SmallIcon("user-trash"), i18n("Delete Tag"), this);
    addAction(deleteTagAction);
    helper->bindTag(deleteTagAction, tag);

    connect(deleteTagAction, SIGNAL(triggered()),
            helper, SLOT(slotTagDelete()));
}

void ContextMenuHelper::addActionDeleteTags(Digikam::TagModificationHelper* helper, QList< TAlbum* > tags)
{
    QAction* const deleteTagsAction = new QAction(SmallIcon("user-trash"), i18n("Delete Tags"), this);
    addAction(deleteTagsAction);
    helper->bindMultipleTags(deleteTagsAction, tags);

    connect(deleteTagsAction, SIGNAL(triggered()),
            helper, SLOT(slotMultipleTagDel()));
}

void ContextMenuHelper::addActionEditTag(TagModificationHelper* helper, TAlbum* tag)
{
    QAction* const editTagAction = new QAction(SmallIcon("tag-properties"), i18nc("Edit Tag Properties", "Properties..."), this);
    addAction(editTagAction);
    helper->bindTag(editTagAction, tag);

    connect(editTagAction, SIGNAL(triggered()),
            helper, SLOT(slotTagEdit()));
}

void ContextMenuHelper::addActionNewAlbum(AlbumModificationHelper* helper, PAlbum* parentAlbum)
{
    QAction* const action = d->copyFromMainCollection("album_new");
    addAction(action);
    helper->bindAlbum(action, parentAlbum);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumNew()));
}

void ContextMenuHelper::addActionDeleteAlbum(AlbumModificationHelper* helper, PAlbum* album)
{
    QAction* const action = d->copyFromMainCollection("album_delete");
    addAction(action, !(album->isRoot() || album->isAlbumRoot()));
    helper->bindAlbum(action, album);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumDelete()));
}

void ContextMenuHelper::addActionEditAlbum(AlbumModificationHelper* helper, PAlbum* album)
{
    QAction* const action = d->copyFromMainCollection("album_propsEdit");
    addAction(action, !album->isRoot());
    helper->bindAlbum(action, album);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumEdit()));
}

void ContextMenuHelper::addActionRenameAlbum(AlbumModificationHelper* helper, PAlbum* album)
{
    QAction* const action = d->copyFromMainCollection("album_rename");
    addAction(action, !(album->isRoot() || album->isAlbumRoot()));
    helper->bindAlbum(action, album);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumRename()));
}

void ContextMenuHelper::addActionResetAlbumIcon(AlbumModificationHelper* helper, PAlbum* album)
{
    QAction* const action = new QAction(SmallIcon("view-refresh"), i18n("Reset Album Icon"), this);
    addAction(action, !album->isRoot());
    helper->bindAlbum(action, album);

    connect(action, SIGNAL(triggered()),
            helper, SLOT(slotAlbumResetIcon()));
}

void ContextMenuHelper::addAssignTagsMenu(const imageIds &ids)
{
    setSelectedIds(ids);

    KMenu* const assignTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::RECENTLYASSIGNED, d->parent);
    assignTagsPopup->menuAction()->setText(i18n("Assign Tag"));
    assignTagsPopup->menuAction()->setIcon(SmallIcon("tag"));
    d->parent->addMenu(assignTagsPopup);

    connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalAssignTag(int)));

    connect(assignTagsPopup, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));
}

void ContextMenuHelper::addRemoveTagsMenu(const imageIds &ids)
{
    setSelectedIds(ids);

    KMenu* const removeTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::REMOVE, d->parent);
    removeTagsPopup->menuAction()->setText(i18n("Remove Tag"));
    removeTagsPopup->menuAction()->setIcon(SmallIcon("tag"));
    d->parent->addMenu(removeTagsPopup);

    // Performance: Only check for tags if there are <250 images selected
    // Otherwise enable it regardless if there are tags or not
    if (ids.count() < 250)
    {
        QList<int> tagIDs = DatabaseAccess().db()->getItemCommonTagIDs(ids);
        bool enable       = false;

        foreach (int tag, tagIDs)
        {
            if (TagsCache::instance()->colorLabelForTag(tag) == -1 &&
                TagsCache::instance()->pickLabelForTag(tag)  == -1 &&
                TagsCache::instance()->isInternalTag(tag)    == false)
            {
                enable = true;
                break;
            }
        }

        removeTagsPopup->menuAction()->setEnabled(enable);
    }

    connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalRemoveTag(int)));
}

void ContextMenuHelper::addLabelsAction()
{
    KMenu* const menuLabels           = new KMenu(i18n("Assign Labels"), d->parent);
    PickLabelMenuAction* const pmenu  = new PickLabelMenuAction(d->parent);
    ColorLabelMenuAction* const cmenu = new ColorLabelMenuAction(d->parent);
    RatingMenuAction* const rmenu     = new RatingMenuAction(d->parent);
    menuLabels->addAction(pmenu);
    menuLabels->addAction(cmenu);
    menuLabels->addAction(rmenu);
    addSubMenu(menuLabels);

    connect(pmenu, SIGNAL(signalPickLabelChanged(int)),
            this, SIGNAL(signalAssignPickLabel(int)));

    connect(cmenu, SIGNAL(signalColorLabelChanged(int)),
            this, SIGNAL(signalAssignColorLabel(int)));

    connect(rmenu, SIGNAL(signalRatingChanged(int)),
            this, SIGNAL(signalAssignRating(int)));
}

// TODO: Port from KABC::AdressBook to to libakonadi-kontact. For instance using Akonadi::ContactSearchJob.
// See http://techbase.kde.org/Development/AkonadiPorting/AddressBook

void ContextMenuHelper::addCreateTagFromAddressbookMenu()
{
#ifdef HAVE_KDEPIMLIBS

    delete d->ABCmenu;

    d->ABCmenu = new QMenu(d->parent);

    QAction* const abcAction = d->ABCmenu->menuAction();
    abcAction->setIcon(SmallIcon("tag-addressbook"));
    abcAction->setText(i18n("Create Tag From Address Book"));
    d->parent->addMenu(d->ABCmenu);

    QAction* const nothingFound = d->ABCmenu->addAction(i18n("No address book entries found"));
    nothingFound->setEnabled(false);

    Akonadi::ContactSearchJob* const job = new Akonadi::ContactSearchJob();
    job->setQuery(Akonadi::ContactSearchJob::ContactUid, "");

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotABCSearchResult(KJob*)));

#endif // HAVE_KDEPIMLIBS
}

#ifdef HAVE_KDEPIMLIBS

void ContextMenuHelper::slotABCSearchResult(KJob* job)
{
    if (job->error())
    {
        kDebug() << "Akonadi search was not succesfull";
        return;
    }

    Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>(job);
    const KABC::Addressee::List contacts = searchJob->contacts();

    if (contacts.isEmpty())
    {
        kDebug() << "No contacts in Akonadi";
        return;
    }

    QStringList names;

    foreach (const KABC::Addressee& addr, contacts)
    {
        if (!addr.realName().isNull())
        {
            names.append(addr.realName());
        }
    }

    names.removeDuplicates();
    names.sort();

    if (names.isEmpty())
    {
        kDebug() << "No names in the address book";
        return;
    }

    d->ABCmenu->clear();

    foreach (const QString& name, names)
    {
        d->ABCmenu->addAction(name);
    }

    connect(d->ABCmenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotABCMenuTriggered(QAction*)));
}

void ContextMenuHelper::slotABCMenuTriggered(QAction* action)
{
    QString name = action->iconText();
    emit signalAddNewTagFromABCMenu(name);
}

#endif // HAVE_KDEPIMLIBS

void ContextMenuHelper::slotDeselectAllAlbumItems()
{
    QAction* const selectNoneAction = d->stdActionCollection->action("selectNone");
    QTimer::singleShot(75, selectNoneAction, SIGNAL(triggered()));
}

void ContextMenuHelper::addImportMenu()
{
    KMenu* const menuImport = new KMenu(i18n("Import"), d->parent);
    QList<QAction*> importActions;

#ifdef HAVE_KIPI
    importActions = KipiPluginLoader::instance()->kipiActionsByCategory(KIPI::ImportPlugin);
#endif /* HAVE_KIPI */

    if (!importActions.isEmpty())
    {
        menuImport->addActions(importActions);
    }
    else
    {
        QAction* const noPlugins = new QAction(i18n("No import plugins available"), this);
        noPlugins->setEnabled(false);
        menuImport->addAction(noPlugins);
    }

    d->parent->addMenu(menuImport);
}

void ContextMenuHelper::addExportMenu()
{
    KMenu* const menuExport = new KMenu(i18n("Export"), d->parent);
    QList<QAction*> exportActions;

#ifdef HAVE_KIPI
    exportActions = KipiPluginLoader::instance()->kipiActionsByCategory(KIPI::ExportPlugin);
#endif /* HAVE_KIPI */

#if 0
    QAction* selectAllAction = 0;
    selectAllAction = d->stdActionCollection->action("selectAll");
#endif

    if (!exportActions.isEmpty())
    {
        menuExport->addActions(exportActions);
    }
    else
    {
        QAction* const noPlugins = new QAction(i18n("No export plugins available"), this);
        noPlugins->setEnabled(false);
        menuExport->addAction(noPlugins);
    }

    d->parent->addMenu(menuExport);
}

void ContextMenuHelper::addBatchMenu()
{
    KMenu* const menuKIPIBatch = new KMenu(i18n("Batch Process"), d->parent);
    QList<QAction*> batchActions;

#ifdef HAVE_KIPI
    batchActions = KipiPluginLoader::instance()->kipiActionsByCategory(KIPI::BatchPlugin);
#endif /* HAVE_KIPI */

#if 0
    QAction* selectAllAction = 0;
    selectAllAction = d->stdActionCollection->action("selectAll");
#endif

    if (!batchActions.isEmpty())
    {
        menuKIPIBatch->addActions(batchActions);
    }
    else
    {
        QAction* const noPlugins = new QAction(i18n("No batch process plugins available"), this);
        noPlugins->setEnabled(false);
        menuKIPIBatch->addAction(noPlugins);
    }

    d->parent->addMenu(menuKIPIBatch);
}

void ContextMenuHelper::addAlbumActions()
{
    QList<QAction*> albumActions;

#ifdef HAVE_KIPI
    albumActions = KipiPluginLoader::instance()->kipiActionsByCategory(KIPI::CollectionsPlugin);
#endif /* HAVE_KIPI */

    if (!albumActions.isEmpty())
    {
        d->parent->addActions(albumActions);
    }
}

void ContextMenuHelper::addGotoMenu(const imageIds &ids)
{
    if (d->gotoAlbumAction && d->gotoDateAction)
    {
        return;
    }

    setSelectedIds(ids);

    // the currently selected image is always the first item
    ImageInfo item;

    if (!d->selectedIds.isEmpty())
    {
        item = ImageInfo(d->selectedIds.first());
    }

    if (item.isNull())
    {
        return;
    }

    // when more then one item is selected, don't add the menu
    if (d->selectedIds.count() > 1)
    {
        return;
    }

    d->gotoAlbumAction    = new QAction(SmallIcon("folder-image"),        i18n("Album"), this);
    d->gotoDateAction     = new QAction(SmallIcon("view-calendar-month"), i18n("Date"),  this);
    KMenu* const gotoMenu = new KMenu(d->parent);
    gotoMenu->addAction(d->gotoAlbumAction);
    gotoMenu->addAction(d->gotoDateAction);

    TagsPopupMenu* const gotoTagsPopup = new TagsPopupMenu(d->selectedIds, TagsPopupMenu::DISPLAY, gotoMenu);
    QAction* const gotoTag             = gotoMenu->addMenu(gotoTagsPopup);
    gotoTag->setIcon(SmallIcon("tag"));
    gotoTag->setText(i18n("Tag"));

    // Disable the goto Tag popup menu, if there are no tags at all.
    if (!DatabaseAccess().db()->hasTags(d->selectedIds))
    {
        gotoTag->setEnabled(false);
    }

    /**
     * TODO:tags to be ported to multiple selection
     */
    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    Album* currentAlbum     = 0;

    if(!albumList.isEmpty())
    {
        currentAlbum = albumList.first();
    }
    else
    {
        return;
    }

    if (currentAlbum->type() == Album::PHYSICAL)
    {
        // If the currently selected album is the same as album to
        // which the image belongs, then disable the "Go To" Album.
        // (Note that in recursive album view these can be different).
        if (item.albumId() == currentAlbum->id())
        {
            d->gotoAlbumAction->setEnabled(false);
        }
    }
    else if (currentAlbum->type() == Album::DATE)
    {
        d->gotoDateAction->setEnabled(false);
    }

    QAction* const gotoMenuAction = gotoMenu->menuAction();
    gotoMenuAction->setIcon(SmallIcon("go-jump"));
    gotoMenuAction->setText(i18n("Go To"));

    connect(gotoTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SIGNAL(signalGotoTag(int)));

    addAction(gotoMenuAction);
}

void ContextMenuHelper::addQueueManagerMenu()
{
    KMenu* const bqmMenu = new KMenu(i18n("Batch Queue Manager"), d->parent);
    bqmMenu->menuAction()->setIcon(KIcon("bqm-diff"));
    bqmMenu->addAction(d->stdActionCollection->action("image_add_to_current_queue"));
    bqmMenu->addAction(d->stdActionCollection->action("image_add_to_new_queue"));

    // if queue list is empty, do not display the queue submenu
    if (QueueMgrWindow::queueManagerWindowCreated() &&
        !QueueMgrWindow::queueManagerWindow()->queuesMap().isEmpty())
    {
        QueueMgrWindow* const qmw = QueueMgrWindow::queueManagerWindow();
        KMenu* const queueMenu    = new KMenu(i18n("Add to Existing Queue"), bqmMenu);

        // queueActions is used by the exec() method to emit an appropriate signal.
        // Reset the map before filling in the actions.
        if (!d->queueActions.isEmpty())
        {
            d->queueActions.clear();
        }

        QList<QAction*> queueList;

        // get queue list from BQM window, do not access it directly, it might crash
        // when the list is changed
        QMap<int, QString> qmwMap = qmw->queuesMap();

        for (QMap<int, QString>::const_iterator it = qmwMap.constBegin(); it != qmwMap.constEnd(); ++it)
        {
            QAction* const action = new QAction(it.value(), this);
            queueList << action;
            d->queueActions[it.key()] = action;
        }

        queueMenu->addActions(queueList);
        bqmMenu->addMenu(queueMenu);
    }

    d->parent->addMenu(bqmMenu);

    // NOTE: see bug #252130 : we need to disable new items to add on BQM is this one is running.
    bqmMenu->setDisabled(QueueMgrWindow::queueManagerWindow()->isBusy());
}

void ContextMenuHelper::setAlbumModel(AbstractCheckableAlbumModel* model)
{
    d->albumModel = model;
}

void ContextMenuHelper::addAlbumCheckUncheckActions(Album* album)
{
    bool     enabled   = false;
    QString  allString = i18n("All Albums");
    QVariant albumData;

    if (album)
    {
        enabled   = true;
        albumData = QVariant::fromValue(AlbumPointer<>(album));

        if (album->type() == Album::TAG)
            allString = i18n("All Tags");
    }

    KMenu* const selectTagsMenu = new KMenu(i18nc("select tags menu", "Select"));
    addSubMenu(selectTagsMenu);

    selectTagsMenu->addAction(allString, d->albumModel, SLOT(checkAllAlbums()));
    selectTagsMenu->addSeparator();
    QAction* const selectChildrenAction = selectTagsMenu->addAction(i18n("Children"), this, SLOT(slotSelectChildren()));
    QAction* const selectParentsAction  = selectTagsMenu->addAction(i18n("Parents"),  this, SLOT(slotSelectParents()));
    selectChildrenAction->setData(albumData);
    selectParentsAction->setData(albumData);

    KMenu* const deselectTagsMenu = new KMenu(i18nc("deselect tags menu", "Deselect"));
    addSubMenu(deselectTagsMenu);

    deselectTagsMenu->addAction(allString, d->albumModel, SLOT(resetAllCheckedAlbums()));
    deselectTagsMenu->addSeparator();
    QAction* const deselectChildrenAction = deselectTagsMenu->addAction(i18n("Children"), this, SLOT(slotDeselectChildren()));
    QAction* const deselectParentsAction  = deselectTagsMenu->addAction(i18n("Parents"),  this, SLOT(slotDeselectParents()));
    deselectChildrenAction->setData(albumData);
    deselectParentsAction->setData(albumData);

    d->parent->addAction(i18n("Invert Selection"), d->albumModel, SLOT(invertCheckedAlbums()));

    selectChildrenAction->setEnabled(enabled);
    selectParentsAction->setEnabled(enabled);
    deselectChildrenAction->setEnabled(enabled);
    deselectParentsAction->setEnabled(enabled);
}

void ContextMenuHelper::slotSelectChildren()
{
    if (!d->albumModel)
    {
        return;
    }

    d->albumModel->checkAllAlbums(d->indexForAlbumFromAction(sender()));
}

void ContextMenuHelper::slotDeselectChildren()
{
    if (!d->albumModel)
    {
        return;
    }

    d->albumModel->resetCheckedAlbums(d->indexForAlbumFromAction(sender()));
}

void ContextMenuHelper::slotSelectParents()
{
    if (!d->albumModel)
    {
        return;
    }

    d->albumModel->checkAllParentAlbums(d->indexForAlbumFromAction(sender()));
}

void ContextMenuHelper::slotDeselectParents()
{
    if (!d->albumModel)
    {
        return;
    }

    d->albumModel->resetCheckedParentAlbums(d->indexForAlbumFromAction(sender()));
}

void ContextMenuHelper::addGroupMenu(const imageIds &ids, const QList<QAction*>& extraMenuItems)
{
    QList<QAction*> actions = groupMenuActions(ids);

    if (actions.isEmpty() && extraMenuItems.isEmpty())
    {
        return;
    }

    if (!extraMenuItems.isEmpty())
    {
        if (!actions.isEmpty())
        {
            QAction* separator = new QAction(this);
            separator->setSeparator(true);
            actions << separator;
        }

        actions << extraMenuItems;
    }

    KMenu* const menu = new KMenu(i18n("Group"));

    foreach(QAction* const action, actions)
    {
        menu->addAction(action);
    }

    d->parent->addMenu(menu);
}

void ContextMenuHelper::addGroupActions(const imageIds &ids)
{
    foreach(QAction* const action, groupMenuActions(ids))
    {
        d->parent->addAction(action);
    }
}

void ContextMenuHelper::setImageFilterModel(ImageFilterModel* model)
{
    d->imageFilterModel = model;
}

QList<QAction*> ContextMenuHelper::groupMenuActions(const imageIds &ids)
{
    setSelectedIds(ids);

    QList<QAction*> actions;

    if (ids.isEmpty())
    {
        if (d->imageFilterModel)
        {
            if (!d->imageFilterModel->isAllGroupsOpen())
            {
                QAction* const openAction = new QAction(i18nc("@action:inmenu", "Open All Groups"), this);
                connect(openAction, SIGNAL(triggered()), this, SLOT(slotOpenGroup()));
                actions << openAction;
            }
            else
            {
                QAction* const closeAction = new QAction(i18nc("@action:inmenu", "Close All Groups"), this);
                connect(closeAction, SIGNAL(triggered()), this, SLOT(slotOpenGroup()));
                actions << closeAction;
            }
        }

        return actions;
    }

    ImageInfo info(ids.first());

    if (ids.size() == 1)
    {
        if (info.hasGroupedImages())
        {
            if (d->imageFilterModel)
            {
                if (!d->imageFilterModel->isGroupOpen(info.id()))
                {
                    QAction* const action = new QAction(i18nc("@action:inmenu", "Show Grouped Images"), this);
                    connect(action, SIGNAL(triggered()), this, SLOT(slotOpenGroups()));
                    actions << action;
                }
                else
                {
                    QAction* const action = new QAction(i18nc("@action:inmenu", "Hide Grouped Images"), this);
                    connect(action, SIGNAL(triggered()), this, SLOT(slotCloseGroups()));
                    actions << action;
                }
            }

            QAction* const separator = new QAction(this);
            separator->setSeparator(true);
            actions << separator;

            QAction* const clearAction = new QAction(i18nc("@action:inmenu", "Ungroup"), this);
            connect(clearAction, SIGNAL(triggered()), this, SIGNAL(signalUngroup()));
            actions << clearAction;
        }
        else if (info.isGrouped())
        {
            QAction* const action = new QAction(i18nc("@action:inmenu", "Remove From Group"), this);
            connect(action, SIGNAL(triggered()), this, SIGNAL(signalRemoveFromGroup()));
            actions << action;

            // TODO: set as group leader / pick image
        }
    }
    else
    {
        QAction* const closeAction = new QAction(i18nc("@action:inmenu", "Group Selected Here"), this);
        connect(closeAction, SIGNAL(triggered()), this, SIGNAL(signalCreateGroup()));
        actions << closeAction;

        QAction* const closeActionDate = new QAction(i18nc("@action:inmenu", "Group Selected By Time"), this);
        connect(closeActionDate, SIGNAL(triggered()), this, SIGNAL(signalCreateGroupByTime()));
        actions << closeActionDate;

        QAction* const separator = new QAction(this);
        separator->setSeparator(true);
        actions << separator;

        if (d->imageFilterModel)
        {
            QAction* const openAction = new QAction(i18nc("@action:inmenu", "Show Grouped Images"), this);
            connect(openAction, SIGNAL(triggered()), this, SLOT(slotOpenGroups()));
            actions << openAction;

            QAction* const closeAction = new QAction(i18nc("@action:inmenu", "Hide Grouped Images"), this);
            connect(closeAction, SIGNAL(triggered()), this, SLOT(slotCloseGroups()));
            actions << closeAction;

            QAction* const separator2 = new QAction(this);
            separator2->setSeparator(true);
            actions << separator2;
        }

        QAction* const removeAction = new QAction(i18nc("@action:inmenu", "Remove Selected From Groups"), this);
        connect(removeAction, SIGNAL(triggered()), this, SIGNAL(signalRemoveFromGroup()));
        actions << removeAction;

        QAction* const clearAction = new QAction(i18nc("@action:inmenu", "Ungroup Selected"), this);
        connect(clearAction, SIGNAL(triggered()), this, SIGNAL(signalUngroup()));
        actions << clearAction;
    }

    return actions;
}

void ContextMenuHelper::setGroupsOpen(bool open)
{
    if (!d->imageFilterModel || d->selectedIds.isEmpty())
    {
        return;
    }

    GroupImageFilterSettings settings = d->imageFilterModel->groupImageFilterSettings();

    foreach(qlonglong id, d->selectedIds)
    {
        ImageInfo info(id);

        if (info.hasGroupedImages())
        {
            settings.setOpen(id, open);
        }
    }

    d->imageFilterModel->setGroupImageFilterSettings(settings);
}

void ContextMenuHelper::slotOpenGroups()
{
    setGroupsOpen(true);
}

void ContextMenuHelper::slotCloseGroups()
{
    setGroupsOpen(false);
}

void ContextMenuHelper::slotOpenAllGroups()
{
    if (!d->imageFilterModel)
    {
        return;
    }

    d->imageFilterModel->setAllGroupsOpen(true);
}

void ContextMenuHelper::slotCloseAllGroups()
{
    if (!d->imageFilterModel)
    {
        return;
    }

    d->imageFilterModel->setAllGroupsOpen(false);
}


void ContextMenuHelper::addStandardActionCut(QObject* recv, const char* slot)
{
    KAction* const cut = KStandardAction::cut(recv, slot, d->parent);
    addAction(cut);
}

void ContextMenuHelper::addStandardActionCopy(QObject* recv, const char* slot)
{
    KAction* const copy = KStandardAction::copy(recv, slot, d->parent);
    addAction(copy);
}

void ContextMenuHelper::addStandardActionPaste(QObject* recv, const char* slot)
{
    KAction* const paste        = KStandardAction::paste(recv, slot, d->parent);
    const QMimeData* const data = kapp->clipboard()->mimeData(QClipboard::Clipboard);

    if (!data || !KUrl::List::canDecode(data))
    {
        paste->setEnabled(false);
    }

    addAction(paste, true);
}

void ContextMenuHelper::addStandardActionItemDelete(QObject* recv, const char* slot, int quantity)
{
    QAction* const trashAction = new QAction(SmallIcon("user-trash"), i18ncp("@action:inmenu Pluralized",
                                                                       "Move to Trash", "Move %1 Files to Trash", quantity), d->parent);
    connect(trashAction, SIGNAL(triggered()),
            recv, slot);

    addAction(trashAction);
}

QAction* ContextMenuHelper::exec(const QPoint& pos, QAction* at)
{
    QAction* const choice = d->parent->exec(pos, at);

    if (choice)
    {
        if (d->selectedIds.count() == 1)
        {
            ImageInfo selectedItem(d->selectedIds.first());

            if (choice == d->gotoAlbumAction)
            {
                emit signalGotoAlbum(selectedItem);
            }
            else if (choice == d->gotoDateAction)
            {
                emit signalGotoDate(selectedItem);
            }
            else if (choice == d->setThumbnailAction)
            {
                emit signalSetThumbnail(selectedItem);
            }
        }

        // check if a BQM action has been triggered
        for (QMap<int, QAction*>::const_iterator it = d->queueActions.constBegin();
             it != d->queueActions.constEnd(); ++it)
        {
            if (choice == it.value())
            {
                emit signalAddToExistingQueue(it.key());
                return choice;
            }
        }
    }

    return choice;
}

void ContextMenuHelper::setSelectedIds(const imageIds &ids)
{
    if (d->selectedIds.isEmpty())
    {
        d->selectedIds = ids;
    }
}

void ContextMenuHelper::setSelectedItems(const KUrl::List& urls)
{
    if (d->selectedItems.isEmpty())
    {
        d->selectedItems = urls;
    }
}

} // namespace Digikam
