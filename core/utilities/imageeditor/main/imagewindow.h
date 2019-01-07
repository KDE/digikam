/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMAGE_WINDOW_H
#define DIGIKAM_IMAGE_WINDOW_H

// Qt includes

#include <QCloseEvent>
#include <QString>
#include <QUrl>

// Local includes

#include "editorwindow.h"
#include "iteminfo.h"

class QDragMoveEvent;
class QDropEvent;

namespace Digikam
{

class SlideShowSettings;
class CollectionImageChangeset;

class ImageWindow : public EditorWindow
{
    Q_OBJECT

public:

    ~ImageWindow();

    static ImageWindow* imageWindow();
    static bool         imageWindowCreated();

    bool queryClose();
    void toggleTag(int tagID);

public:

    DInfoInterface* infoIface(DPluginAction* const ac);

    virtual VersionManager* versionManager() const;

public Q_SLOTS:

    void loadItemInfos(const ItemInfoList& imageInfoList,
                        const ItemInfo& imageInfoCurrent, const QString& caption);
    void openImage(const ItemInfo& info);

    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating);

Q_SIGNALS:

    void signalURLChanged(const QUrl& url);
    void signalSavingDialogProgress(float value);

private:

    ImageWindow();

    void loadIndex(const QModelIndex& index);

    void closeEvent(QCloseEvent* e);
    void showEvent(QShowEvent*);
    void dragMoveEvent(QDragMoveEvent* e);
    void dropEvent(QDropEvent* e);

    bool save();
    bool saveAs();
    bool saveNewVersion();
    bool saveCurrentVersion();
    bool saveNewVersionAs();
    bool saveNewVersionInFormat(const QString& format);

    QUrl saveDestinationUrl();
    bool hasOriginalToRestore();
    DImageHistory resolvedImageHistory(const DImageHistory& history);

    void prepareImageToSave();
    void saveFaceTagsToImage(const ItemInfo& info);

    void saveIsComplete();
    void saveAsIsComplete();
    void saveVersionIsComplete();
    void setViewToURL(const QUrl& url);
    void deleteCurrentItem(bool ask, bool permanently);
    void removeCurrent();

    void assignPickLabel(const ItemInfo& info, int pickId);
    void assignColorLabel(const ItemInfo& info, int colorId);
    void assignRating(const ItemInfo& info, int rating);
    void toggleTag(const ItemInfo& info, int tagID);

    ThumbBarDock* thumbBar()     const;
    Sidebar*      rightSideBar() const;

Q_SIGNALS: // private signals

    void loadCurrentLater();

private Q_SLOTS:

    void slotLoadItemInfosStage2();
    void slotThumbBarModelReady();

    void slotForward();
    void slotBackward();
    void slotFirst();
    void slotLast();
    void slotFileWithDefaultApplication();

    void slotToMainWindow();

    void slotThumbBarImageSelected(const ItemInfo&);
    void slotLoadCurrent();
    void slotDeleteCurrentItem();
    void slotDeleteCurrentItemPermanently();
    void slotDeleteCurrentItemPermanentlyDirectly();
    void slotTrashCurrentItemDirectly();

    void slotChanged();
    void slotUpdateItemInfo();
    void slotFileOriginChanged(const QString&);

    void slotRevert();
    void slotOpenOriginal();

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);

    void slotRatingChanged(const QUrl&, int);
    void slotColorLabelChanged(const QUrl&, int);
    void slotPickLabelChanged(const QUrl&, int);
    void slotToggleTag(const QUrl&, int);

    void slotFileMetadataChanged(const QUrl&);
    //void slotCollectionImageChange(const CollectionImageChangeset&);
    //void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotDroppedOnThumbbar(const QList<ItemInfo>& infos);

    void slotComponentsInfo();
    void slotDBStat();

    void slotAddedDropedItems(QDropEvent*);
    void slotOpenWith(QAction* action=0);

    void slotRightSideBarActivateTitles();
    void slotRightSideBarActivateComments();
    void slotRightSideBarActivateAssignedTags();

// -- Internal setup methods implemented in imagewindow_config.cpp ----------------------------------------

public Q_SLOTS:

    void slotSetup();
    void slotSetupICC();

    void slotSetupChanged();

// -- Internal setup methods implemented in imagewindow_setup.cpp ----------------------------------------

private:

    void setupActions();
    void setupConnections();
    void setupUserArea();

    void addServicesMenu();

private Q_SLOTS:

    void slotContextMenu();

// -- Extra tool methods implemented in imagewindow_tools.cpp ----------------------------------------

private Q_SLOTS:

    void slotFilePrint();
    void slotPresentation();

private:

    void slideShow(SlideShowSettings& settings);

// -- Export tools methods implemented in imagewindow_export.cpp -------------------------------------

private Q_SLOTS:

    void slotExportTool();

// -- Import tools methods implemented in imagewindow_import.cpp -------------------------------------

private Q_SLOTS:

    void slotImportedImagefromScanner(const QUrl& url);
    void slotImportTool();

// -- Internal private container --------------------------------------------------------------------

private:

    static ImageWindow* m_instance;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_WINDOW_H
