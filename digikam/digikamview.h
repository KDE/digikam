/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2002-16-10
 * Description : album view interface implementation
 *
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
 * Copyright      2006 by Gilles Caulier
 *
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

signals:

    void signal_albumSelected(bool val);
    void signal_tagSelected(bool val);
    void signal_imageSelected(bool val);
    void signal_noCurrentItem();

public slots:

    void slot_newAlbum();
    void slot_sortAlbums(int order);
    void slot_deleteAlbum();
    void slot_thumbSizePlus();
    void slot_thumbSizeMinus();

    // Album action slots
    void slot_albumPropsEdit();
    void slot_albumAddImages();
    void slot_albumOpenInKonqui();
    void slot_albumRefresh();
    void slotAlbumImportFolder();
    void slotAlbumHistoryBack(int steps=1);
    void slotAlbumHistoryForward(int steps=1);
    void slotAlbumDeleted(Album *album);
    void slotAlbumRenamed(Album *album);
    void slotSelectAlbum(const KURL &url);

    // Tag action slots
    void slotNewTag();
    void slotDeleteTag();
    void slotEditTag();

    // Search action slots
    void slotNewQuickSearch();
    void slotNewAdvancedSearch();

    // Image action slots
    void slot_imagePreview(AlbumIconItem* iconItem=0);
    void slot_imageEdit(AlbumIconItem* iconItem=0);
    void slot_imageExifOrientation(int orientation);
    void slot_imageRename(AlbumIconItem* iconItem=0);
    void slot_imageDelete();
    void slot_imageDeletePermanently();
    void slot_imageDeletePermanentlyDirectly();
    void slot_imageTrashDirectly();
    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
    void slotSortImages(int order);

    void slot_albumSelected(Album* album);

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

private slots:

    void slotAllAlbumsLoaded();

    void slotAlbumsCleared();
    void slotAlbumHighlight();

    void slotImageSelected();
    void slotDispatchImageSelected();
    void slotImageCopyResult(KIO::Job* job);

    void slotLeftSidebarChangedTab(QWidget* w);

    void slotFirstItem(void);
    void slotPrevItem(void);
    void slotNextItem(void);
    void slotLastItem(void);

    void slotEscapePreview();
    void slotEditImage();

private:

    DigikamViewPriv* d;
};

}  // namespace Digikam

#endif // DIGIKAMVIEW_H
