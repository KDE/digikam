/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2002-16-10
 * Description : implementation of album view interface. 
 *
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier
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

namespace KIO
{
class Job;
}

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

    void applySettings(const AlbumSettings* settings);
    void clearHistory();
    void getForwardHistory(QStringList &titles);
    void getBackwardHistory(QStringList &titles);
    void showSideBars();
    void hideSideBars();

signals:

    void signalAlbumSelected(bool val);
    void signalTagSelected(bool val);
    void signalImageSelected(const QPtrList<ImageInfo>& list, bool, bool);
    void signalNoCurrentItem();
    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);

public slots:

    // View Action slots
    void slotThumbSizePlus();
    void slotThumbSizeMinus();
    void slotSlideShowAll();
    void slotSlideShowSelection();
    void slotSlideShowRecursive();

    // Album action slots
    void slotNewAlbum();
    void slotSortAlbums(int order);
    void slotDeleteAlbum();
    void slotSelectAlbum(const KURL &url);
    void slotAlbumPropsEdit();
    void slotAlbumAddImages();
    void slotAlbumOpenInKonqui();
    void slotAlbumRefresh();
    void slotAlbumImportFolder();
    void slotAlbumHistoryBack(int steps=1);
    void slotAlbumHistoryForward(int steps=1);
    void slotAlbumDeleted(Album *album);
    void slotAlbumRenamed(Album *album);
    void slotAlbumSyncPicturesMetadata();
    void slotAlbumSyncPicturesMetadataDone();
    void slotAlbumSelected(Album* album);

    // Tag action slots
    void slotNewTag();
    void slotDeleteTag();
    void slotEditTag();

    // Search action slots
    void slotNewQuickSearch();
    void slotNewAdvancedSearch();

    // Image action slots
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

private:

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
    void slotToggledToPreviewMode(bool);
    void slotDispatchImageSelected();
    void slotImageCopyResult(KIO::Job* job);
    void slotItemsInfoFromAlbums(const ImageInfoList&);

    void slotLeftSidebarChangedTab(QWidget* w);

    void slotFirstItem(void);
    void slotPrevItem(void);
    void slotNextItem(void);
    void slotLastItem(void);

    void slotEscapePreview();

private:

    DigikamViewPriv* d;
};

}  // namespace Digikam

#endif // DIGIKAMVIEW_H
