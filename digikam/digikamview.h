/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : implementation of album view interface.
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{
class AlbumIconItem;
class AlbumSettings;
class Album;
class AlbumIconViewFilter;
class BatchSyncMetadata;
class DigikamViewPriv;

class DigikamView : public KHBox
{
    Q_OBJECT

public:

    DigikamView(QWidget *parent);
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

    void connectIconViewFilter(AlbumIconViewFilter *filter);

    KUrl::List allUrls() const;
    KUrl::List selectedUrls() const;

Q_SIGNALS:

    void signalAlbumSelected(bool val);
    void signalTagSelected(bool val);
    void signalImageSelected(const ImageInfoList& selectedImage, bool hasPrevious, bool hasNext, const ImageInfoList& allImages);
    void signalNoCurrentItem();
    void signalSelectionChanged(int numberOfSelectedItems);
    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);
    void signalThumbSizeChanged(int);
    void signalZoomChanged(double, int);
    void signalTogglePreview(bool);

    void signalGotoAlbumAndItem(const ImageInfo&);
    void signalGotoDateAndItem(AlbumIconItem *);
    void signalGotoTagAndItem(int tagID);
    void signalChangedTab(QWidget*);

public Q_SLOTS:

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
    void slotNewAlbumFromSelection();
    void slotSortAlbums(int order);
    void slotDeleteAlbum();
    void slotSelectAlbum(const KUrl& url);
    void slotAlbumPropsEdit();
    void slotAlbumOpenInKonqui();
    void slotAlbumRefresh();
    void slotAlbumHistoryBack(int steps=1);
    void slotAlbumHistoryForward(int steps=1);
    void slotAlbumAdded(Album *album);
    void slotAlbumDeleted(Album *album);
    void slotAlbumRenamed(Album *album);
    void slotAlbumWriteMetadata();
    void slotAlbumReadMetadata();
    void slotAlbumSelected(Album* album);

    void slotGotoAlbumAndItem(const ImageInfo& imageInfo);
    void slotGotoDateAndItem(const ImageInfo& imageInfo);
    void slotGotoTagAndItem(int tagID);

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
    void slotGroupImages(int mode);

    // Image Rating slots
    void slotAssignRating(int rating);
    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();

    // Tools action slots.
    void slotLightTable();
    void slotQueueMgr();

private:

    void toggleZoomActions();
    void setupConnections();
    void loadViewState();
    void saveViewState();
    void changeAlbumFromHistory(Album *album, QWidget *widget);
    void slideShow(const ImageInfoList& infoList);
    void connectBatchSyncMetadata(BatchSyncMetadata *syncMetadata);

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

    void slotToggledToPreviewMode(bool);
    void slotEscapePreview();
    void slotCancelSlideShow();

    void slotThumbSizeEffect();
    void slotZoomFactorChanged(double);

    void slotSidebarTabTitleStyleChanged();

    void slotProgressMessageChanged(const QString& descriptionOfAction);
    void slotProgressValueChanged(float percent);
    void slotProgressFinished();
    void slotOrientationChangeFailed(const QStringList& failedFileNames);

private:

    DigikamViewPriv* const d;
};

}  // namespace Digikam

#endif // DIGIKAMVIEW_H
