/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

// Qt includes

#include <QCloseEvent>
#include <QString>

// KDE includes

#include <kurl.h>

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

    bool setup();
    bool setupICC();

    bool queryClose();
    virtual VersionManager* versionManager() const;

    void toggleTag(int tagID);

public Q_SLOTS:

    void loadImageInfos(const ImageInfoList& imageInfoList,
                        const ImageInfo& imageInfoCurrent, const QString& caption);
    void openImage(const ImageInfo& info);

    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating);

Q_SIGNALS:

    void signalFileDeleted(const KUrl& url);
    void signalURLChanged(const KUrl& url);
    void signalSavingDialogProgress(float value);

private:

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

    KUrl saveDestinationUrl();
    bool hasOriginalToRestore();
    DImageHistory resolvedImageHistory(const DImageHistory& history);

    void prepareImageToSave();

    void saveIsComplete();
    void saveAsIsComplete();
    void saveVersionIsComplete();
    void setViewToURL(const KUrl& url);
    void deleteCurrentItem(bool ask, bool permanently);
    void removeCurrent();
    void slotFileOriginChanged(const QString&);

    void slideShow(SlideShowSettings& settings);

    void assignPickLabel(const ImageInfo& info, int pickId);
    void assignColorLabel(const ImageInfo& info, int colorId);
    void assignRating(const ImageInfo& info, int rating);

    ThumbBarDock* thumbBar() const;
    Sidebar* rightSideBar() const;

    ImageWindow();

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

    void slotToMainWindow();

    void slotThumbBarImageSelected(const ImageInfo&);
    void slotLoadCurrent();
    void slotDeleteCurrentItem();
    void slotDeleteCurrentItemPermanently();
    void slotDeleteCurrentItemPermanentlyDirectly();
    void slotTrashCurrentItemDirectly();

    void slotChanged();
    void slotUpdateItemInfo();

    void slotContextMenu();
    void slotRevert();
    void slotOpenOriginal();

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);

    void slotRatingChanged(const KUrl&, int);
    void slotColorLabelChanged(const KUrl&, int);
    void slotPickLabelChanged(const KUrl&, int);

    void slotFileMetadataChanged(const KUrl&);
    //void slotCollectionImageChange(const CollectionImageChangeset&);
    //void slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void slotDroppedOnThumbbar(const QList<ImageInfo>& infos);

    void slotComponentsInfo();
    void slotDBStat();

    void slotSetupChanged();

private:

    class ImageWindowPriv;
    ImageWindowPriv* const d;

    static ImageWindow* m_instance;
};

}  // namespace Digikam

#endif /* IMAGEWINDOW_H */
