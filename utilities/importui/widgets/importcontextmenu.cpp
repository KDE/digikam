/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-13
 * Description : Modified context menu helper for import tool
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "importcontextmenu.moc"

// Qt includes

#include <QAction>

// KDE includes

#include <kaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kservice.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <kopenwithdialog.h>
#include <krun.h>
#include <kstandardaction.h>

// Local includes

#include "importui.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "ratingwidget.h"
#include "tagmodificationhelper.h"
#include "tagspopupmenu.h"
#include "fileactionmngr.h"

namespace Digikam
{

class ImportContextMenuHelper::Private
{
public:

    explicit Private(ImportContextMenuHelper* const q) :
        importFilterModel(0),
        parent(0),
        ABCmenu(0),
        stdActionCollection(0),
        q(q)
    {}

    QList<qlonglong>             selectedIds;
    KUrl::List                   selectedItems;

    QMap<int, QAction*>          queueActions;
    QMap<QString, KService::Ptr> servicesMap;

    ImportFilterModel*           importFilterModel;

    QMenu*                       parent;
    QMenu*                       ABCmenu;

    KActionCollection*           stdActionCollection;

    ImportContextMenuHelper*     q;

public:

    QAction* copyFromMainCollection(const char* name)
    {
        QAction* mainAction = stdActionCollection->action(name);

        if (!mainAction)
        {
            return 0;
        }

        QAction* action = new QAction(mainAction->icon(), mainAction->text(), q);
        action->setToolTip(mainAction->toolTip());
        return action;
    }
};

ImportContextMenuHelper::ImportContextMenuHelper(QMenu* const parent, KActionCollection* const actionCollection)
    : QObject(parent), d(new Private(this))
{
    d->parent = parent;

    if (!actionCollection)
    {
        d->stdActionCollection = ImportUI::instance()->actionCollection();
    }
    else
    {
        d->stdActionCollection = actionCollection;
    }
}

ImportContextMenuHelper::~ImportContextMenuHelper()
{
    delete d;
}

void ImportContextMenuHelper::addAction(const char* name, bool addDisabled)
{
    QAction* action = d->stdActionCollection->action(name);
    addAction(action, addDisabled);
}

void ImportContextMenuHelper::addAction(QAction* action, bool addDisabled)
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

void ImportContextMenuHelper::addSubMenu(KMenu* subMenu)
{
    d->parent->addMenu(subMenu);
}

void ImportContextMenuHelper::addSeparator()
{
    d->parent->addSeparator();
}

void ImportContextMenuHelper::addAction(QAction* action, QObject* recv, const char* slot,
                                        bool addDisabled)
{
    if (!action)
    {
        return;
    }

    connect(action, SIGNAL(triggered()),
            recv, slot);

    addAction(action, addDisabled);
}

void ImportContextMenuHelper::addServicesMenu(const KUrl::List& selectedItems)
{
    setSelectedItems(selectedItems);

    // This code is inspired by KonqMenuActions:
    // kdebase/apps/lib/konq/konq_menuactions.cpp

    QStringList    mimeTypes;
    KService::List offers;

    foreach(const KUrl& item, d->selectedItems)
    {
        const QString mimeType = KMimeType::findByUrl(item, 0, true, true)->name();

        if (!mimeTypes.contains(mimeType))
        {
            mimeTypes << mimeType;
        }
    }

    if (!mimeTypes.isEmpty())
    {
        // Query trader
        const QString firstMimeType      = mimeTypes.takeFirst();
        const QString constraintTemplate = "'%1' in ServiceTypes";
        QStringList   constraints;

        foreach(const QString& mimeType, mimeTypes)
        {
            constraints << constraintTemplate.arg(mimeType);
        }

        offers = KMimeTypeTrader::self()->query(firstMimeType, "Application", constraints.join(" and "));

        // remove duplicate service entries
        QSet<QString> seenApps;

        for (KService::List::iterator it = offers.begin(); it != offers.end();)
        {
            const QString appName((*it)->name());

            if (!seenApps.contains(appName))
            {
                seenApps.insert(appName);
                ++it;
            }
            else
            {
                it = offers.erase(it);
            }
        }
    }

    if (!offers.isEmpty() && ImportUI::instance()->cameraUseUMSDriver())
    {
        KMenu* servicesMenu    = new KMenu(d->parent);
        qDeleteAll(servicesMenu->actions());

        QAction* serviceAction = servicesMenu->menuAction();
        serviceAction->setText(i18nc("@title:menu", "Open With"));

        foreach(KService::Ptr service, offers)
        {
            QString name         = service->name().replace('&', "&&");
            QAction* action      = servicesMenu->addAction(name);
            action->setIcon(KIcon(service->icon()));
            action->setData(service->name());
            d->servicesMap[name] = service;
        }

        servicesMenu->addSeparator();
        servicesMenu->addAction(i18nc("@item:inmenu", "Other..."));

        addAction(serviceAction);

        connect(servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }
    else if (ImportUI::instance()->cameraUseUMSDriver())
    {
        QAction* serviceAction = new QAction(i18nc("@title:menu", "Open With..."), this);
        addAction(serviceAction);

        connect(serviceAction, SIGNAL(triggered()),
                this, SLOT(slotOpenWith()));
    }
}

void ImportContextMenuHelper::slotOpenWith()
{
    // call the slot with an "empty" action
    slotOpenWith(0);
}

void ImportContextMenuHelper::slotOpenWith(QAction* action)
{
    KService::Ptr service;
    KUrl::List list = d->selectedItems;

    QString name = action ? action->data().toString() : QString();

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

void ImportContextMenuHelper::addRotateMenu(itemIds& /*ids*/)
{
//    setSelectedIds(ids);

//    KMenu* imageRotateMenu = new KMenu(i18n("Rotate"), d->parent);
//    imageRotateMenu->setIcon(KIcon("object-rotate-right"));

//    KAction* left = new KAction(this);
//    left->setObjectName("rotate_ccw");
//    left->setText(i18nc("rotate image left", "Left"));
//    connect(left, SIGNAL(triggered(bool)),
//            this, SLOT(slotRotate()));
//    imageRotateMenu->addAction(left);

//    KAction* right = new KAction(this);
//    right->setObjectName("rotate_cw");
//    right->setText(i18nc("rotate image right", "Right"));
//    connect(right, SIGNAL(triggered(bool)),
//            this, SLOT(slotRotate()));
//    imageRotateMenu->addAction(right);

//    d->parent->addMenu(imageRotateMenu);
}

void ImportContextMenuHelper::slotRotate()
{
//TODO: Implement rotate in import tool.
//    if (sender()->objectName() == "rotate_ccw")
//    {
//        FileActionMngr::instance()->transform(CamItemInfoList(d->selectedIds), KExiv2Iface::RotationMatrix::Rotate270);
//    }
//    else
//    {
//        FileActionMngr::instance()->transform(CamItemInfoList(d->selectedIds), KExiv2Iface::RotationMatrix::Rotate90);
//    }
}

void ImportContextMenuHelper::addAssignTagsMenu(itemIds& /*ids*/)
{
//    setSelectedIds(ids);

//    KMenu* assignTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::RECENTLYASSIGNED, d->parent);
//    assignTagsPopup->menuAction()->setText(i18n("Assign Tag"));
//    assignTagsPopup->menuAction()->setIcon(SmallIcon("tag"));
//    d->parent->addMenu(assignTagsPopup);

//    connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
//            this, SIGNAL(signalAssignTag(int)));

//    connect(assignTagsPopup, SIGNAL(signalPopupTagsView()),
//            this, SIGNAL(signalPopupTagsView()));
}

void ImportContextMenuHelper::addRemoveTagsMenu(itemIds& /*ids*/)
{
//    setSelectedIds(ids);

//    KMenu* removeTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::REMOVE, d->parent);
//    removeTagsPopup->menuAction()->setText(i18n("Remove Tag"));
//    removeTagsPopup->menuAction()->setIcon(SmallIcon("tag"));
//    d->parent->addMenu(removeTagsPopup);

//    connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
//            this, SIGNAL(signalRemoveTag(int)));
}

void ImportContextMenuHelper::addLabelsAction()
{
    KMenu* menuLabels           = new KMenu(i18n("Assign Labels"), d->parent);
    PickLabelMenuAction* pmenu  = new PickLabelMenuAction(d->parent);
    ColorLabelMenuAction* cmenu = new ColorLabelMenuAction(d->parent);
    RatingMenuAction* rmenu     = new RatingMenuAction(d->parent);
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

void ImportContextMenuHelper::slotABCMenuTriggered(QAction* action)
{
    QString name = action->iconText();
    emit signalAddNewTagFromABCMenu(name);
}

void ImportContextMenuHelper::setImportFilterModel(ImportFilterModel* model)
{
    d->importFilterModel = model;
}

QAction* ImportContextMenuHelper::exec(const QPoint& pos, QAction* at)
{
    QAction* choice = d->parent->exec(pos, at);

    if (choice)
    {
        // check if a BQM action has been triggered
        for (QMap<int, QAction*>::const_iterator it = d->queueActions.constBegin();
             it != d->queueActions.constEnd(); ++it)
        {
            if (choice == it.value())
            {
                //emit signalAddToExistingQueue(it.key());
                return choice;
            }
        }
    }

    return choice;
}

void ImportContextMenuHelper::setSelectedIds(itemIds& ids)
{
    if (d->selectedIds.isEmpty())
    {
        d->selectedIds = ids;
    }
}

void ImportContextMenuHelper::setSelectedItems(const KUrl::List& urls)
{
    if (d->selectedItems.isEmpty())
    {
        d->selectedItems = urls;
    }
}

} // namespace Digikam
