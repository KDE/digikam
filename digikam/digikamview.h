/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : implementation of album view interface.
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef DIGIKAMVIEW_H
#define DIGIKAMVIEW_H

// Qt includes

#include <QStringList>
#include <QMainWindow>
#include <QMap>

// KDE includes

#include <khbox.h>
#include <kurl.h>

// Local includes

#include "searchtextbar.h"
#include "imageinfo.h"
#include "digikammodelcollection.h"
#include "sidebarwidget.h"

namespace Digikam
{
class AlbumIconItem;
class AlbumSettings;
class Album;
class BatchSyncMetadata;
class FilterStatusBar;

class DigikamView : public KHBox
{
    Q_OBJECT

public:

    DigikamView(QWidget* parent, DigikamModelCollection* modelCollection);
    ~DigikamView();

    void applySettings();
    void refreshView();
    void clearHistory();
    void getForwardHistory(QStringList& titles);
    void getBackwardHistory(QStringList& titles);
    void showSideBars();
    void hideSideBars();
    void setThumbSize(int size);
    void toggleShowBar(bool);
    void setRecurseAlbums(bool recursive);
    void setRecurseTags(bool recursive);

    void connectIconViewFilter(FilterStatusBar* filter);

    KUrl::List allUrls() const;
    KUrl::List selectedUrls() const;
    bool hasCurrentItem() const;

    double zoomMin();
    double zoomMax();

    void toggleTag(int tagID);
    QList<SidebarWidget*> leftSidebarWidgets();

Q_SIGNALS:

    void signalAlbumSelected(bool val);
    void signalTagSelected(bool val);
    void signalImageSelected(const ImageInfoList& selectedImage, bool hasPrevious, bool hasNext, 
                             const ImageInfoList& allImages);
    void signalNoCurrentItem();
    void signalSelectionChanged(int numberOfSelectedItems);
    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);
    void signalThumbSizeChanged(int);
    void signalZoomChanged(double);
    void signalSwitchedToPreview();
    void signalSwitchedToIconView();
    void signalSwitchedToMapView();

    void signalGotoAlbumAndItem(const ImageInfo&);
    void signalGotoDateAndItem(AlbumIconItem*);
    void signalGotoTagAndItem(int tagID);
    void signalChangedTab(QWidget*);

public Q_SLOTS:

    void setZoomFactor(double zoom);

    // View Action slots
    void slotZoomIn();
    void slotZoomOut();
    void slotZoomTo100Percents();
    void slotFitToWindow();
    void slotSlideShowAll();
    void slotSlideShowSelection();
    void slotSlideShowRecursive();

    // Album action slots
    void slotNewAlbum();
    void slotSortAlbums(int order);
    void slotDeleteAlbum();
    void slotAlbumPropsEdit();
    void slotAlbumOpenInFileManager();
    void slotAlbumOpenInTerminal();
    void slotAlbumRefresh();
    void slotAlbumHistoryBack(int steps=1);
    void slotAlbumHistoryForward(int steps=1);
    void slotAlbumAdded(Album* album);
    void slotAlbumDeleted(Album* album);
    void slotAlbumRenamed(Album* album);
    void slotAlbumWriteMetadata();
    void slotAlbumReadMetadata();
    void slotAlbumSelected(Album* album);

    void slotGotoAlbumAndItem(const ImageInfo& imageInfo);
    void slotGotoDateAndItem(const ImageInfo& imageInfo);
    void slotGotoTagAndItem(int tagID);

    void slotSelectAlbum(const KUrl& url);

    // Tag action slots
    void slotNewTag();
    void slotDeleteTag();
    void slotEditTag();

    // Search action slots
    void slotNewKeywordSearch();
    void slotNewAdvancedSearch();
    void slotNewDuplicatesSearch(Album* album=0);

    // Image action slots
    void slotImageLightTable();
    void slotImageAddToLightTable();
    void slotImageAddToCurrentQueue();
    void slotImageAddToNewQueue();
    void slotImageAddToExistingQueue(int);
    void slotImagePreview();
    void slotMapWidgetView();
    void slotIconView();
    void slotImageEdit();
    void slotImageFindSimilar();
    void slotImageExifOrientation(int orientation);
    void slotImageRename();
    void slotImageDelete();
    void slotImageDeletePermanently();
    void slotImageDeletePermanentlyDirectly();
    void slotImageTrashDirectly();
    void slotImageWriteMetadata();
    void slotImageReadMetadata();
    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
    void slotSortImages(int order);
    void slotSortImagesOrder(int order);
    void slotGroupImages(int mode);
    void slotMoveSelectionToAlbum();

    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating);

    // Tools action slots.
    void slotEditor();
    void slotLightTable();
    void slotQueueMgr();

    void slotLeftSideBarActivate(QWidget* widget);
    void slotLeftSideBarActivate(SidebarWidget* widget);
    void slotLeftSideBarActivateAlbums();
    void slotLeftSideBarActivateTags();

private:

    void toggleZoomActions();
    void setupConnections();
    void loadViewState();
    void saveViewState();
    void changeAlbumFromHistory(Album* album, QWidget* widget);
    void slideShow(const ImageInfoList& infoList);
    void connectBatchSyncMetadata(BatchSyncMetadata* syncMetadata);

private Q_SLOTS:

    void slotAllAlbumsLoaded();

    void slotAlbumsCleared();

    void slotImageSelected();
    void slotTogglePreviewMode(const ImageInfo& info);
    void slotDispatchImageSelected();
    void slotItemsInfoFromAlbums(const ImageInfoList&);

    void slotLeftSidebarChangedTab(QWidget* w);

    void slotFirstItem();
    void slotPrevItem();
    void slotNextItem();
    void slotLastItem();
    void slotSelectItemByUrl(const KUrl&);

    void slotViewModeChanged();
    void slotEscapePreview();
    void slotCancelSlideShow();

    void slotThumbSizeEffect();
    void slotZoomFactorChanged(double);

    void slotSidebarTabTitleStyleChanged();

    void slotProgressMessageChanged(const QString& descriptionOfAction);
    void slotProgressValueChanged(float percent);
    void slotProgressFinished();
    void slotOrientationChangeFailed(const QStringList& failedFileNames);
    void slotRatingChanged(const KUrl&, int);

    void slotPopupFiltersView();

private:

    class DigikamViewPriv;
    DigikamViewPriv* const d;
};

}  // namespace Digikam

#endif // DIGIKAMVIEW_H
