/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
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

#ifndef SHOW_FOTO_H
#define SHOW_FOTO_H

// Qt includes

#include <QUrl>

// Local includes

#include "editorwindow.h"
#include "showfotoiteminfo.h"

namespace ShowFoto
{

class ShowFoto : public Digikam::EditorWindow
{
    Q_OBJECT

public:

    explicit ShowFoto(const QList<QUrl>& urlList);
    ~ShowFoto();

    virtual void show();

public Q_SLOTS:

    void slotSetup();
    void slotSetupICC();

private:

    bool setup(bool iccSetupPage=false);

    bool queryClose();

    void setupActions();
    void setupConnections();
    void setupUserArea();

    void readSettings();
    void saveSettings();
    void applySettings();

    void toggleActions(bool val);
    void addServicesMenu();

    void toggleNavigation(int index);

    bool save();
    bool saveAs();
    void moveFile();
    void finishSaving(bool success);
    QUrl saveDestinationUrl();
    bool saveNewVersion();
    bool saveCurrentVersion();
    bool saveNewVersionAs();
    bool saveNewVersionInFormat(const QString&);

    void saveIsComplete();
    void saveAsIsComplete();
    void saveVersionIsComplete();

    void slideShow(Digikam::SlideShowSettings& settings);

    void openFolder(const QUrl& url);
    void openUrls(const QList<QUrl>& urls);

    Digikam::ThumbBarDock* thumbBar()     const;
    Digikam::Sidebar*      rightSideBar() const;

private Q_SLOTS:

    void slotForward();
    void slotBackward();
    void slotLast();
    void slotFirst();
    void slotFilePrint();
    void slotFileWithDefaultApplication();
    void slotOpenWith(QAction* action=0);
    void slotShowfotoItemInfoActivated(const ShowfotoItemInfo& info);

    void slotOpenFile();
    void slotOpenUrl(const ShowfotoItemInfo& info);
    void slotOpenFolder(const QUrl&);
    void slotOpenFilesInFolder();
    void slotDroppedUrls(const QList<QUrl>& droppedUrls);
    void slotDeleteCurrentItem();

    void slotChanged();
    void slotUpdateItemInfo();

    void slotPrepareToLoad();
    void slotLoadingStarted(const QString& filename);
    void slotLoadingFinished(const QString& filename, bool success);
    void slotSavingStarted(const QString& filename);

    void slotContextMenu();
    void slotRevert();
    void slotSetupMetadataFilters(int);

    void slotAddedDropedItems(QDropEvent*);

    void slotImportFromScanner();
    void slotImportedImagefromScanner(const QUrl& url);

    void slotEditMetadata();
    void slotEditGeolocation();
    void slotHtmlGallery();
    void slotCalendar();
    void slotPresentation();
    void slotExpoBlending();
    void slotPanorama();
    void slotVideoSlideshow();
    void slotSendByMail();
    void slotPrintCreator();
    void slotMediaServer();

    void slotExportTool();
    void slotImportTool();

Q_SIGNALS:

    void signalLoadCurrentItem(const QList<QUrl>& urlList);
    void signalOpenFolder(const QUrl&);
    void signalOpenFile(const QList<QUrl>& urls);
    void signalInfoList(ShowfotoItemInfoList&);

private:

    class Private;
    Private* const d;
};

} // namespace ShowFoto

#endif // SHOW_FOTO_H
