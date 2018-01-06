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

#include "importcontextmenu.h"

// Qt includes

#include <QAction>
#include <QIcon>
#include <QMimeType>
#include <QMimeDatabase>

// KDE includes

#include <kmimetypetrader.h>
#include <klocalizedstring.h>
#include <kactioncollection.h>

#ifdef HAVE_KIO
#   include <kopenwithdialog.h>
#endif

// Local includes

#include "importui.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "ratingwidget.h"
#include "tagmodificationhelper.h"
#include "tagspopupmenu.h"
#include "fileactionmngr.h"
#include "dfileoperations.h"

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
    {
    }

    QList<qlonglong>             selectedIds;
    QList<QUrl>                  selectedItems;

    QMap<int, QAction*>          queueActions;
    QMap<QString, KService::Ptr> servicesMap;

    ImportFilterModel*           importFilterModel;

    QMenu*                       parent;
    QMenu*                       ABCmenu;

    KActionCollection*           stdActionCollection;

    ImportContextMenuHelper*     q;

public:

    QAction* copyFromMainCollection(const QString& name) const
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

ImportContextMenuHelper::ImportContextMenuHelper(QMenu* const parent, KActionCollection* const actionCollection)
    : QObject(parent),
      d(new Private(this))
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

void ImportContextMenuHelper::addAction(const QString& name, bool addDisabled)
{
    QAction* const action = d->stdActionCollection->action(name);
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

void ImportContextMenuHelper::addSubMenu(QMenu* subMenu)
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

void ImportContextMenuHelper::addServicesMenu(const QList<QUrl>& selectedItems)
{
    setSelectedItems(selectedItems);

    // This code is inspired by KonqMenuActions:
    // kdebase/apps/lib/konq/konq_menuactions.cpp

    QStringList    mimeTypes;
    KService::List offers;

    foreach(const QUrl& item, d->selectedItems)
    {
        const QString mimeType = QMimeDatabase().mimeTypeForFile(item.toLocalFile(), QMimeDatabase::MatchExtension).name();

        if (!mimeTypes.contains(mimeType))
        {
            mimeTypes << mimeType;
        }
    }

    if (!mimeTypes.isEmpty())
    {
        // Query trader
        const QString firstMimeType      = mimeTypes.takeFirst();
        const QString constraintTemplate = QString::fromUtf8("'%1' in ServiceTypes");
        QStringList   constraints;

        foreach(const QString& mimeType, mimeTypes)
        {
            constraints << constraintTemplate.arg(mimeType);
        }

        offers = KMimeTypeTrader::self()->query(firstMimeType, QLatin1String("Application"), constraints.join(QLatin1String(" and ")));

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
        QMenu* const servicesMenu    = new QMenu(d->parent);
        qDeleteAll(servicesMenu->actions());

        QAction* const serviceAction = servicesMenu->menuAction();
        serviceAction->setText(i18nc("@title:menu", "Open With"));

        foreach(KService::Ptr service, offers)
        {
            QString name          = service->name().replace(QLatin1Char('&'), QLatin1String("&&"));
            QAction* const action = servicesMenu->addAction(name);
            action->setIcon(QIcon::fromTheme(service->icon()));
            action->setData(service->name());
            d->servicesMap[name] = service;
        }
#ifdef HAVE_KIO

        servicesMenu->addSeparator();
        servicesMenu->addAction(i18nc("@item:inmenu", "Other..."));

        addAction(serviceAction);

        connect(servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }
    else if (ImportUI::instance()->cameraUseUMSDriver())
    {
        QAction* const serviceAction = new QAction(i18nc("@title:menu", "Open With..."), this);
        addAction(serviceAction);

        connect(serviceAction, SIGNAL(triggered()),
                this, SLOT(slotOpenWith()));
#endif // HAVE_KIO
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
    QList<QUrl> list = d->selectedItems;

    QString name = action ? action->data().toString() : QString();

#ifdef HAVE_KIO
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
                DFileOperations::runFiles(dlg->text(), list);
            }

            delete dlg;
            return;
        }

        delete dlg;
    }
    else
#endif // HAVE_KIO
    {
        service = d->servicesMap[name];
    }

    DFileOperations::runFiles(service.data(), list);
}

void ImportContextMenuHelper::addRotateMenu(itemIds& /*ids*/)
{
//    setSelectedIds(ids);

//    QMenu* const imageRotateMenu = new QMenu(i18n("Rotate"), d->parent);
//    imageRotateMenu->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-right")));

//    QAction* const left = new QAction(this);
//    left->setObjectName(QLatin1String("rotate_ccw"));
//    left->setText(i18nc("rotate image left", "Left"));
//    connect(left, SIGNAL(triggered(bool)),
//            this, SLOT(slotRotate()));
//    imageRotateMenu->addAction(left);

//    QAction* const right = new QAction(this);
//    right->setObjectName(QLatin1String("rotate_cw");
//    right->setText(i18nc("rotate image right", "Right")));
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
//        FileActionMngr::instance()->transform(CamItemInfoList(d->selectedIds), MetaEngineRotation::Rotate270);
//    }
//    else
//    {
//        FileActionMngr::instance()->transform(CamItemInfoList(d->selectedIds), MetaEngineRotation::Rotate90);
//    }
}

void ImportContextMenuHelper::addAssignTagsMenu(itemIds& /*ids*/)
{
    //setSelectedIds(ids);

    //QMenu* const assignTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::RECENTLYASSIGNED, d->parent);
    //assignTagsPopup->menuAction()->setText(i18n("Assign Tag"));
    //assignTagsPopup->menuAction()->setIcon(QIcon::fromTheme(QLatin1String("tag")));
    //d->parent->addMenu(assignTagsPopup);

    //connect(assignTagsPopup, SIGNAL(signalTagActivated(int)),
    //        this, SIGNAL(signalAssignTag(int)));

    //connect(assignTagsPopup, SIGNAL(signalPopupTagsView()),
    //        this, SIGNAL(signalPopupTagsView()));
}

void ImportContextMenuHelper::addRemoveTagsMenu(itemIds& /*ids*/)
{
    //setSelectedIds(ids);

    //QMenu* const removeTagsPopup = new TagsPopupMenu(ids, TagsPopupMenu::REMOVE, d->parent);
    //removeTagsPopup->menuAction()->setText(i18n("Remove Tag"));
    //removeTagsPopup->menuAction()->setIcon(QIcon::fromTheme(QLatin1String("tag")));
    //d->parent->addMenu(removeTagsPopup);

    //connect(removeTagsPopup, SIGNAL(signalTagActivated(int)),
    //        this, SIGNAL(signalRemoveTag(int)));
}

void ImportContextMenuHelper::addLabelsAction()
{
    QMenu* const menuLabels           = new QMenu(i18n("Assign Labels"), d->parent);
    PickLabelMenuAction* const pmenu  = new PickLabelMenuAction(d->parent);
    ColorLabelMenuAction* const cmenu = new ColorLabelMenuAction(d->parent);
    RatingMenuAction* const rmenu     = new RatingMenuAction(d->parent);
    menuLabels->addAction(pmenu->menuAction());
    menuLabels->addAction(cmenu->menuAction());
    menuLabels->addAction(rmenu->menuAction());
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
    QAction* const choice = d->parent->exec(pos, at);

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

void ImportContextMenuHelper::setSelectedItems(const QList<QUrl>& urls)
{
    if (d->selectedItems.isEmpty())
    {
        d->selectedItems = urls;
    }
}

} // namespace Digikam
