/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : implementation of album view interface.
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef DIGIKAM_VIEW_H
#define DIGIKAM_VIEW_H

// Qt includes

#include <QStringList>
#include <QUrl>

// Local includes

#include "applicationsettings.h"
#include "metaengine_rotation.h"
#include "digikam_config.h"
#include "searchtextbar.h"
#include "imageinfo.h"
#include "digikammodelcollection.h"
#include "sidebarwidget.h"
#include "stackedview.h"
#include "dlayoutbox.h"

namespace Digikam
{

class AlbumIconItem;
class Album;
class PAlbum;
class TAlbum;
class BatchSyncMetadata;
class FilterStatusBar;
class SlideShowSettings;
class DCategorizedView;
class ImageFilterModel;

class DigikamView : public DHBox
{
    Q_OBJECT

public:

    explicit DigikamView(QWidget* const parent,
                         DigikamModelCollection* const modelCollection);
    ~DigikamView();

    void applySettings();
    void refreshView();
    void clearHistory();
    void getForwardHistory(QStringList& titles);
    void getBackwardHistory(QStringList& titles);

    void showSideBars();
    void hideSideBars();
    void toggleLeftSidebar();
    void toggleRightSidebar();
    void previousLeftSideBarTab();
    void nextLeftSideBarTab();
    void previousRightSideBarTab();
    void nextRightSideBarTab();

    void setToolsIconView(DCategorizedView* const view);
    void setThumbSize(int size);
    void toggleShowBar(bool);
    void setRecurseAlbums(bool recursive);
    void setRecurseTags(bool recursive);
    void imageTransform(MetaEngineRotation::TransformationAction transform);

    void connectIconViewFilter(FilterStatusBar* const filter);

    QUrl      currentUrl()     const;
    bool      hasCurrentItem() const;
    ImageInfo currentInfo()    const;
    Album*    currentAlbum()   const;

    /**
     * Get currently selected items. By default only the first images in groups are
     * given, while all can be obtained by setting the grouping parameter to true.
     * Given an operation, it will be determined from settings/user query whether
     * only the first or all items in a group are returned.
     * Ideally only the latter (giving an operation) is used.
     */
    QList<QUrl>   selectedUrls(bool grouping = false)                         const;
    QList<QUrl>   selectedUrls(const ApplicationSettings::OperationType type) const;
    ImageInfoList selectedInfoList(const bool currentFirst = false,
                                   const bool grouping = false)               const;
    ImageInfoList selectedInfoList(const ApplicationSettings::OperationType type,
                                   const bool currentFirst = false)           const;
    /**
     * Get all items in the current view.
     * Whether only the first or all grouped items are returned is determined
     * as described above.
     */
    QList<QUrl>   allUrls(bool grouping = false)                         const;
    ImageInfoList allInfo(const bool grouping = false)                   const;
    ImageInfoList allInfo(const ApplicationSettings::OperationType type) const;

    /**
     * Query whether the operation to be performed on currently selected items
     * (all=false, default) or all items in the currently active view (all=true)
     * should be performed on all grouped items or just the first.
     *
     * @brief needGroupResolving
     * @param type Type of operation to be performed.
     * @param all Whether to apply to all items in the current view or just selected
     * @return Whether to perform operation on all grouped items or just the first
     */
    bool needGroupResolving(const ApplicationSettings::OperationType type,
                            const bool all = false) const;

    double zoomMin() const;
    double zoomMax() const;

    void presentation();

    void toggleTag(int tagID);
    void toggleFullScreen(bool set);

    QList<SidebarWidget*>        leftSidebarWidgets() const;
    StackedView::StackedViewMode viewMode()           const;

Q_SIGNALS:

    void signalAlbumSelected(Album*);
    void signalImageSelected(const ImageInfoList& selectedImage, const ImageInfoList& allImages);
    void signalNoCurrentItem();
    void signalSelectionChanged(int numberOfSelectedItems);
    void signalThumbSizeChanged(int);
    void signalZoomChanged(double);
    void signalSwitchedToPreview();
    void signalSwitchedToIconView();
    void signalSwitchedToMapView();
    void signalSwitchedToTableView();
    void signalSwitchedToTrashView();

    void signalGotoAlbumAndItem(const ImageInfo&);
    void signalGotoDateAndItem(AlbumIconItem*);
    void signalGotoTagAndItem(int tagID);
    void signalChangedTab(QWidget*);
    void signalFuzzySidebarActive(bool active);

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
    void slotSlideShowManualFromCurrent();
    void slotSlideShowManualFrom(const ImageInfo& info);

    // Album action slots
    void slotRefresh();
    void slotNewAlbum();
    void slotSortAlbums(int role);
    void slotDeleteAlbum();
    void slotRenameAlbum();
    void slotAlbumPropsEdit();
    void slotAlbumOpenInFileManager();
    void slotAlbumHistoryBack(int steps=1);
    void slotAlbumHistoryForward(int steps=1);
    void slotAlbumWriteMetadata();
    void slotAlbumReadMetadata();
    void slotAlbumSelected(QList<Album*> albums);

    void slotGotoAlbumAndItem(const ImageInfo& imageInfo);
    void slotGotoDateAndItem(const ImageInfo& imageInfo);
    void slotGotoTagAndItem(int tagID);

    void slotSelectAlbum(const QUrl& url);
    void slotSetCurrentWhenAvailable(const qlonglong id);

    void slotSetAsAlbumThumbnail(const ImageInfo& info);

    // Tag action slots
    void slotNewTag();
    void slotDeleteTag();
    void slotEditTag();
    void slotOpenTagsManager();
    void slotAssignTag();

    // Search action slots
    void slotNewKeywordSearch();
    void slotNewAdvancedSearch();
    void slotNewDuplicatesSearch(PAlbum* album=0);
    void slotNewDuplicatesSearch(QList<PAlbum*> albums);
    void slotNewDuplicatesSearch(QList<TAlbum*> albums);

    // Image action slots
    void slotImageLightTable();
    void slotImageAddToLightTable();
    void slotImageAddToCurrentQueue();
    void slotImageAddToNewQueue();
    void slotImageAddToExistingQueue(int);
    void slotImagePreview();
    void slotMapWidgetView();
    void slotTableView();
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
    void slotSeparateImages(int mode);
    void slotImageSeparationSortOrder(int order);
    void slotMoveSelectionToAlbum();
    void slotImagePaste();

    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);
    void slotAssignRating(int rating);
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);

    // Tools action slots.
    void slotEditor();
    void slotLightTable();
    void slotQueueMgr();
    void slotFileWithDefaultApplication();

    void slotLeftSideBarActivate(QWidget* widget);
    void slotLeftSideBarActivate(SidebarWidget* widget);
    void slotLeftSideBarActivateAlbums();
    void slotLeftSideBarActivateTags();

    void slotRightSideBarActivateTitles();
    void slotRightSideBarActivateComments();
    void slotRightSideBarActivateAssignedTags();

    void slotFocusAndNextImage();

    void slotCreateGroupFromSelection();
    void slotCreateGroupByTimeFromSelection();
    void slotCreateGroupByFilenameFromSelection();
    void slotRemoveSelectedFromGroup();
    void slotUngroupSelected();

private:

    void toggleZoomActions();
    void setupConnections();
    void loadViewState();
    void saveViewState();
    void changeAlbumFromHistory(QList<Album*> album, QWidget* const widget);
    void slideShow(const ImageInfoList& infoList);

private Q_SLOTS:

    void slotAllAlbumsLoaded();

    void slotAlbumsCleared();

    void slotImageSelected();
    void slotTogglePreviewMode(const ImageInfo& info);
    void slotDispatchImageSelected();

    void slotLeftSidebarChangedTab(QWidget* w);

    void slotFirstItem();
    void slotPrevItem();
    void slotNextItem();
    void slotLastItem();
    void slotSelectItemByUrl(const QUrl&);
    void slotAwayFromSelection();

    void slotViewModeChanged();
    void slotEscapePreview();

    void slotSlideShowBuilderComplete(const SlideShowSettings& settings);

    void slotThumbSizeEffect();
    void slotZoomFactorChanged(double);

    void slotSidebarTabTitleStyleChanged();

    void slotImageChangeFailed(const QString& message, const QStringList& fileNames);

    void slotRatingChanged(const QUrl&, int);
    void slotColorLabelChanged(const QUrl&, int);
    void slotPickLabelChanged(const QUrl&, int);
    void slotToggleTag(const QUrl&, int);

    void slotPopupFiltersView();
    void slotSetupMetadataFilters(int);

    void slotAlbumRefreshComplete();

    void slotShowContextMenu(QContextMenuEvent* event,
                             const QList<QAction*>& extraGroupingActions = QList<QAction*>());

    void slotShowContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info,
                                   const QList<QAction*>& extraGroupingActions = QList<QAction*>(),
                                   ImageFilterModel* imageFilterModel = 0);

    void slotShowGroupContextMenu(QContextMenuEvent* event,
                                  const QList<ImageInfo>& selectedInfos,
                                  ImageFilterModel* imageFilterModel = 0);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_VIEW_H
