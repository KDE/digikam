/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGE_WINDOW_H
#define IMAGE_WINDOW_H

// Qt includes

#include <QCloseEvent>
#include <QString>
#include <QUrl>

// Local includes

#include "editorwindow.h"
#include "imageinfo.h"

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

    virtual VersionManager* versionManager() const;

public Q_SLOTS:

    void slotSetup();
    void slotSetupICC();

    void loadImageInfos(const ImageInfoList& imageInfoList,
                        const ImageInfo& imageInfoCurrent, const QString& caption);
    void openImage(const ImageInfo& info);

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

    void setupActions();
    void setupConnections();
    void setupUserArea();

    bool save();
    bool saveAs();
    bool saveNewVersion();
    bool saveCurrentVersion();
    bool saveNewVersionAs();
    bool saveNewVersionInFormat(const QString& format);

    void addServicesMenu();

    QUrl saveDestinationUrl();
    bool hasOriginalToRestore();
    DImageHistory resolvedImageHistory(const DImageHistory& history);

    void prepareImageToSave();
    void saveFaceTagsToImage(const ImageInfo& info);

    void saveIsComplete();
    void saveAsIsComplete();
    void saveVersionIsComplete();
    void setViewToURL(const QUrl& url);
    void deleteCurrentItem(bool ask, bool permanently);
    void removeCurrent();

    void slideShow(SlideShowSettings& settings);

    void assignPickLabel(const ImageInfo& info, int pickId);
    void assignColorLabel(const ImageInfo& info, int colorId);
    void assignRating(const ImageInfo& info, int rating);
    void toggleTag(const ImageInfo& info, int tagID);

    ThumbBarDock* thumbBar()     const;
    Sidebar*      rightSideBar() const;

Q_SIGNALS: // private signals

    void loadCurrentLater();

private Q_SLOTS:

    void slotLoadImageInfosStage2();
    void slotThumbBarModelReady();

    void slotForward();
    void slotBackward();
    void slotFirst();
    void slotLast();
    void slotFilePrint();
    void slotFileWithDefaultApplication();

    void slotToMainWindow();

    void slotThumbBarImageSelected(const ImageInfo&);
    void slotLoadCurrent();
    void slotDeleteCurrentItem();
    void slotDeleteCurrentItemPermanently();
    void slotDeleteCurrentItemPermanentlyDirectly();
    void slotTrashCurrentItemDirectly();

    void slotChanged();
    void slotUpdateItemInfo();
    void slotFileOriginChanged(const QString&);

    void slotContextMenu();
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
    void slotDroppedOnThumbbar(const QList<ImageInfo>& infos);

    void slotComponentsInfo();
    void slotDBStat();

    void slotSetupChanged();

    void slotAddedDropedItems(QDropEvent*);
    void slotOpenWith(QAction* action=0);

    void slotRightSideBarActivateTitles();
    void slotRightSideBarActivateComments();
    void slotRightSideBarActivateAssignedTags();

    void slotImportFromScanner();
    void slotImportedImagefromScanner(const QUrl& url);

    void slotEditMetadata();
    void slotEditGeolocation();
    void slotPresentation();
    void slotHtmlGallery();
    void slotCalendar();
    void slotExpoBlending();
    void slotPanorama();
    void slotVideoSlideshow();
    void slotSendByMail();
    void slotPrintCreator();
    void slotMediaServer();
    
    void slotExportTool();
    void slotImportTool();

private:

    static ImageWindow* m_instance;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGE_WINDOW_H
