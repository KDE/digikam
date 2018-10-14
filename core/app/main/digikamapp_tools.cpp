/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Extra tools
 *
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
    actionModel->addAction(m_expoBlendingAction,          postCategory);
    actionModel->addAction(m_calendarAction,              postCategory);
    actionModel->addAction(m_metadataEditAction,          postCategory);
    actionModel->addAction(m_timeAdjustAction,            postCategory);
    actionModel->addAction(m_presentationAction,          postCategory);
    actionModel->addAction(m_sendByMailAction,            postCategory);
    actionModel->addAction(m_printCreatorAction,          postCategory);
    actionModel->addAction(m_mediaServerAction,           postCategory);

#ifdef HAVE_PANORAMA
    actionModel->addAction(m_panoramaAction,              postCategory);
#endif

#ifdef HAVE_MEDIAPLAYER
    actionModel->addAction(m_videoslideshowAction,        postCategory);
#endif

#ifdef HAVE_HTMLGALLERY
    actionModel->addAction(m_htmlGalleryAction,           postCategory);
#endif

#ifdef HAVE_MARBLE
    actionModel->addAction(m_geolocationEditAction,       postCategory);
#endif

    QString exportCategory = i18nc("@title Export Tools",          "Export");

    foreach (QAction* const ac, exportActions())
    {
        actionModel->addAction(ac,                        exportCategory);
    }

    QString importCategory = i18nc("@title Import Tools",          "Import");

    foreach (QAction* const ac, importActions())
    {
        actionModel->addAction(ac,                        importCategory);
    }

#ifdef HAVE_KSANE
    actionModel->addAction(m_ksaneAction,                 importCategory);
#endif

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

void DigikamApp::slotTimeAdjust()
{
    QList<QUrl> urls = view()->selectedUrls(ApplicationSettings::Metadata);

    if (urls.isEmpty())
        return;

    QPointer<TimeAdjustDialog> dialog = new TimeAdjustDialog(this, new DBInfoIface(this, urls, ApplicationSettings::Metadata));

    connect(dialog, SIGNAL(signalDateTimeForUrl(QUrl,QDateTime,bool)),
            DIO::instance(), SLOT(slotDateTimeForUrl(QUrl,QDateTime,bool)));

    dialog->exec();

    delete dialog;
}

void DigikamApp::slotEditMetadata()
{
    QList<QUrl> urls = view()->selectedUrls(ApplicationSettings::Metadata);

    if (urls.isEmpty())
        return;

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(this, urls);
    dialog->exec();

    delete dialog;

    // Refresh Database with new metadata from files.
    CollectionScanner scanner;

    foreach (const QUrl& url, urls)
    {
        scanner.scanFile(url.toLocalFile(), CollectionScanner::Rescan);
        ImageAttributesWatch::instance()->fileMetadataChanged(url);
    }
}

void DigikamApp::slotEditGeolocation()
{
#ifdef HAVE_MARBLE
    ImageInfoList infos = d->view->selectedInfoList(ApplicationSettings::Metadata);

    if (infos.isEmpty())
        return;

    TagModel* const tagModel                    = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, this);
    TagPropertiesFilterModel* const filterModel = new TagPropertiesFilterModel(this);
    filterModel->setSourceAlbumModel(tagModel);
    filterModel->sort(0);

    QPointer<GeolocationEdit> dialog = new GeolocationEdit(filterModel,
                                                           new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools),
                                                           this);
    dialog->setItems(ImageGPS::infosToItems(infos));
    dialog->exec();

    delete dialog;
#endif
}

void DigikamApp::slotPresentation()
{
    d->view->presentation();
}

void DigikamApp::slotPrintCreator()
{
    QPointer<AdvPrintWizard> w = new AdvPrintWizard(this, new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools));
    w->exec();
    delete w;
}

} // namespace Digikam
