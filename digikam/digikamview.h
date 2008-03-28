/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : implementation of album view interface. 
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu> 
 * Copyright (C) 2002-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qhbox.h>
#include <qstringlist.h>
#include <qmap.h>

// Local includes.

#include "imageinfo.h"

class KURL;

namespace Digikam
{
class AlbumIconItem;
class AlbumSettings;
class Album;
class DigikamViewPriv;

class DigikamView : public QHBox
{
    Q_OBJECT

public:

    DigikamView(QWidget *parent);
    ~DigikamView();

    void applySettings();
    void refreshView();
    void clearHistory();
    void getForwardHistory(QStringList &titles);
    void getBackwardHistory(QStringList &titles);
    void showSideBars();
    void hideSideBars();
    void setThumbSize(int size);

signals:

    void signalAlbumSelected(bool val);
    void signalTagSelected(bool val);
    void signalImageSelected(const QPtrList<ImageInfo>& list, bool, bool,
                             const KURL::List& listAll);
    void signalNoCurrentItem();
    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);
    void signalThumbSizeChanged(int);
    void signalZoomChanged(double, int);
    void signalTogglePreview(bool);
    void signalGotoAlbumAndItem(AlbumIconItem *);
    void signalGotoDateAndItem(AlbumIconItem *);
    void signalGotoTagAndItem(int tagID);
    void signalChangedTab(QWidget*);

public slots:

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
    void slotSelectAlbum(const KURL &url);
    void slotAlbumPropsEdit();
    void slotAlbumOpenInKonqui();
    void slotAlbumRefresh();
    void slotAlbumImportFolder();
    void slotAlbumHistoryBack(int steps=1);
    void slotAlbumHistoryForward(int steps=1);
    void slotAlbumAdded(Album *album);
    void slotAlbumDeleted(Album *album);
    void slotAlbumRenamed(Album *album);
    void slotAlbumSyncPicturesMetadata();
    void slotAlbumSyncPicturesMetadataDone();
    void slotAlbumSelected(Album* album);
    void slotGotoAlbumAndItem(AlbumIconItem* iconItem);
    void slotGotoDateAndItem(AlbumIconItem* iconItem);
    void slotGotoTagAndItem(int tagID);

    // Tag action slots
    void slotNewTag();
    void slotDeleteTag();
    void slotEditTag();

    // Search action slots
    void slotNewQuickSearch();
    void slotNewAdvancedSearch();

    // Image action slots
    void slotImageLightTable();
    void slotImageAddToLightTable();
    void slotImagePreview();
    void slotImageEdit();
    void slotImageExifOrientation(int orientation);
    void slotImageRename(AlbumIconItem* iconItem=0);
    void slotImageDelete();
    void slotImageDeletePermanently();
    void slotImageDeletePermanentlyDirectly();
    void slotImageTrashDirectly();
    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
    void slotSortImages(int order);

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

private:

    void toggleZoomActions();
    void setupConnections();
    void loadViewState();
    void saveViewState();
    void changeAlbumFromHistory(Album *album, QWidget *widget);
    void imageEdit(AlbumIconItem* iconItem=0);
    void slideShow(ImageInfoList &infoList);

private slots:

    void slotAllAlbumsLoaded();

    void slotAlbumsCleared();
    void slotAlbumHighlight();

    void slotImageSelected();
    void slotTogglePreviewMode(AlbumIconItem *iconItem=0);
    void slotDispatchImageSelected();
    void slotItemsInfoFromAlbums(const ImageInfoList&);

    void slotLeftSidebarChangedTab(QWidget* w);

    void slotFirstItem(void);
    void slotPrevItem(void);
    void slotNextItem(void);
    void slotLastItem(void);

    void slotToggledToPreviewMode(bool);
    void slotEscapePreview();
    void slotCancelSlideShow();

    void slotThumbSizeEffect();
    void slotZoomFactorChanged(double);

private:

    DigikamViewPriv* d;
};

}  // namespace Digikam

#endif // DIGIKAMVIEW_H
