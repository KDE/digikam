/***************************************************************************
                          digikamview.h  -  description
                             -------------------
    begin                : Sat Nov 16 10:11:43 CST 2002
    copyright            : (C) 2002 by Renchi Raju
    email                : renchi@pooh.tam.uiuc.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIGIKAMVIEW_H
#define DIGIKAMVIEW_H

#include <qsplitter.h>
#include <qstringlist.h>
#include <qmap.h>
#include <kio/job.h>

class QString;
class QIconViewItem;
class QResizeEvent;
class KURL;

class DigikamApp;
class AlbumFolderView;
class AlbumIconView;
class AlbumIconItem;
class AlbumSettings;

namespace Digikam
{
class AlbumManager;
class AlbumInfo;
}

class DigikamView : public QSplitter {

    Q_OBJECT

public:

    DigikamView(QWidget *parent);
    ~DigikamView();

    void applySettings(const AlbumSettings* settings);

private:

    void setupConnections();

private:

    DigikamApp               *mParent;
    AlbumFolderView          *mFolderView;
    AlbumIconView            *mIconView;
    Digikam::AlbumManager    *mAlbumMan;

public slots:

    void slot_newAlbum();
    void slot_sortAlbums(int order);
    void slot_deleteAlbum();
    void slot_thumbSizePlus();
    void slot_thumbSizeMinus();

    // Album action slots
    void slot_albumPropsEdit();
    void slot_albumAddImages();

    // Image action slots
    void slot_imageView(AlbumIconItem* iconItem=0);
    void slot_imageCommentsEdit(AlbumIconItem* iconItem=0);
    void slot_imageExifInfo(AlbumIconItem* iconItem=0);
    void slot_imageRename(AlbumIconItem* iconItem=0);
    void slot_imageDelete();
    void slotImageProperties();
    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
  

private slots:

    void slot_imageSelected();
    void slot_albumSelected(Digikam::AlbumInfo* album);

    void slot_albumsCleared();
    void slot_albumHighlight();

    void slot_imageCopyResult(KIO::Job* job);

signals:

    void signal_albumSelected(bool val);
    void signal_imageSelected(bool val);

};

#endif
