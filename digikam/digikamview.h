//////////////////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2002-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles Caulier <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef DIGIKAMVIEW_H
#define DIGIKAMVIEW_H

// Qt includes.
#include <qsplitter.h>
#include <qstringlist.h>
#include <qmap.h>

// KDE includes.

#include <kio/job.h>

class QString;
class QIconViewItem;
class KURL;

class DigikamApp;
class AlbumFolderView;
class AlbumIconView;
class AlbumIconItem;
class AlbumSettings;
class AlbumManager;
class Album;
class AlbumHistory;

class DigikamView : public QSplitter {

    Q_OBJECT

public:

    DigikamView(QWidget *parent);
    ~DigikamView();

    void applySettings(const AlbumSettings* settings);

    void getForwardHistory(QStringList &titles);
    void getBackwardHistory(QStringList &titles);    
    
private:

    void setupConnections();

private:

    DigikamApp               *mParent;
    AlbumFolderView          *mFolderView;
    AlbumIconView            *mIconView;
    AlbumManager             *mAlbumMan;
    AlbumHistory             *mAlbumHistory;
    
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
    
    // Image action slots
    void slot_imageView(AlbumIconItem* iconItem=0);
    void slot_imageCommentsEdit(AlbumIconItem* iconItem=0);
    void slot_imageExifOrientation(int orientation);
    void slot_imageRename(AlbumIconItem* iconItem=0);
    void slot_imageDelete();
    void slotImageProperties();
    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
    void slotSortImages(int order);

private slots:

    void slot_imageSelected();
    void slot_albumSelected(Album* album);

    void slot_albumsCleared();
    void slot_albumHighlight();

    void slot_imageCopyResult(KIO::Job* job);

    void slotFolderViewInFocus();
    void slotIconViewInFocus();
    
signals:

    void signal_albumSelected(bool val);
    void signal_tagSelected(bool val);
    void signal_imageSelected(bool val);
};

#endif // DIGIKAMVIEW_H
