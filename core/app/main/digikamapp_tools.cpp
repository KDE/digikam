/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Extra tools
 *
 * Copyright (C) 2002-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikamapp.h"
#include "digikamapp_p.h"

namespace Digikam
{

void DigikamApp::setupSelectToolsAction()
{
    // Create action model
    ActionItemModel* const actionModel = new ActionItemModel(this);
    actionModel->setMode(ActionItemModel::ToplevelMenuCategory | ActionItemModel::SortCategoriesByInsertionOrder);

    // Builtin actions

    QString mainCategory   = i18nc("@title Main Tools",            "Main Tools");
    actionModel->addAction(d->ieAction,                   mainCategory);
    actionModel->addAction(d->openTagMngrAction,          mainCategory);
    actionModel->addAction(d->bqmAction,                  mainCategory);
    actionModel->addAction(d->maintenanceAction,          mainCategory);
    actionModel->addAction(d->ltAction,                   mainCategory);
    actionModel->addAction(d->advSearchAction,            mainCategory);

    QString postCategory   = i18nc("@title Post Processing Tools", "Post-Processing");

    foreach (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericTool, this))
    {
        actionModel->addAction(ac, postCategory);
    }

    foreach (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericMetadata, this))
    {
        actionModel->addAction(ac, postCategory);
    }

    QString exportCategory = i18nc("@title Export Tools",          "Export");

    foreach (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericExport, this))
    {
        actionModel->addAction(ac, exportCategory);
    }

    QString importCategory = i18nc("@title Import Tools",          "Import");

    foreach (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericImport, this))
    {
        actionModel->addAction(ac, importCategory);
    }

    // setup categorized view
    DCategorizedSortFilterProxyModel* const filterModel = actionModel->createFilterModel();

    ActionCategorizedView* const selectToolsActionView  = new ActionCategorizedView;
    selectToolsActionView->setupIconMode();
    selectToolsActionView->setModel(filterModel);
    selectToolsActionView->adjustGridSize();

    connect(selectToolsActionView, SIGNAL(clicked(QModelIndex)),
            actionModel, SLOT(trigger(QModelIndex)));

    d->view->setToolsIconView(selectToolsActionView);
}

void DigikamApp::slotMaintenance()
{
    MaintenanceDlg* const dlg = new MaintenanceDlg(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        d->maintenanceAction->setEnabled(false);

        MaintenanceMngr* const mngr = new MaintenanceMngr(this);

        connect(mngr, SIGNAL(signalComplete()),
                this, SLOT(slotMaintenanceDone()));

        mngr->setSettings(dlg->settings());
    }
}

void DigikamApp::slotMaintenanceDone()
{
    d->maintenanceAction->setEnabled(true);
    d->view->refreshView();

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->refreshView();
    }

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        QueueMgrWindow::queueManagerWindow()->refreshView();
    }
}

void DigikamApp::slotDatabaseMigration()
{
    DatabaseMigrationDialog dlg(this);
    dlg.exec();
}

} // namespace Digikam
