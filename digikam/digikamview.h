/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2002-16-10
 * Description : 
 * 
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
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

class QString;
class QIconViewItem;

class KURL;

namespace KIO
{
class Job;
}

namespace Digikam
{
class Sidebar;
class ImagePropertiesSideBarDB;
class DigikamApp;
class AlbumFolderView;
class AlbumIconView;
class AlbumIconItem;
class AlbumSettings;
class AlbumManager;
class Album;
class AlbumHistory;
class DateFolderView;
class TagFolderView;
class TagFilterView;
class SearchFolderView;

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
    
private:

    void setupConnections();
    void loadViewState();
    void saveViewState();

private:

    DigikamApp                        *mParent;
    AlbumFolderView                   *mFolderView;    
    AlbumIconView                     *mIconView;
    AlbumManager                      *mAlbumMan;
    AlbumHistory                      *mAlbumHistory;
    Digikam::Sidebar                  *mMainSidebar;
    DateFolderView                    *mDateFolderView;
    TagFolderView                     *mTagFolderView;
    SearchFolderView                  *mSearchFolderView;
    Digikam::ImagePropertiesSideBarDB *mRightSidebar;
    TagFilterView                     *mTagFilterView;
    int                                mInitialAlbumID;
    QSplitter                         *mSplitter;
    
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
    void slotAlbumImportFolder();
    void slotAlbumHistoryBack(int steps=1);
    void slotAlbumHistoryForward(int steps=1);
    void slotAlbumDeleted(Album *album);
    void slotSelectAlbum(const KURL &url);

    // Tag action slots
    void slotNewTag();
    void slotDeleteTag();
    void slotEditTag();

    // Search action slots
    void slotNewQuickSearch();
    void slotNewAdvancedSearch();
    
    // Image action slots
    void slot_imageView(AlbumIconItem* iconItem=0);
    void slot_imageExifOrientation(int orientation);
    void slot_imageRename(AlbumIconItem* iconItem=0);
    void slot_imageDelete();
    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
    void slotSortImages(int order);

    void slot_albumSelected(Album* album);
    
private slots:

    void slotAllAlbumsLoaded();
    
    void slot_imageSelected();

    void slot_albumsCleared();
    void slot_albumHighlight();

    void slot_imageCopyResult(KIO::Job* job);

    void slotLeftSidebarChangedTab(QWidget* w);

    void slotFirstItem(void);
    void slotPrevItem(void);    
    void slotNextItem(void);
    void slotLastItem(void);

signals:

    void signal_albumSelected(bool val);
    void signal_tagSelected(bool val);
    void signal_imageSelected(bool val);
};

}  // namespace Digikam

#endif // DIGIKAMVIEW_H
