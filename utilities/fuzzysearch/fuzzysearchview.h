/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FUZZYSEARCHVIEW_H
#define FUZZYSEARCHVIEW_H

// Qt includes.

#include <QWidget>

class QDragEnterEvent;
class QDropEvent;
class QPixmap;

namespace Digikam
{

class SAlbum;
class ImageInfo;
class SearchTextBar;
class LoadingDescription;
class FuzzySearchFolderView;
class FuzzySearchViewPriv;

class FuzzySearchView : public QWidget
{
    Q_OBJECT

public:

    FuzzySearchView(QWidget *parent=0);
    ~FuzzySearchView();

    FuzzySearchFolderView* folderView() const;
    SearchTextBar* searchBar() const;

    void setActive(bool val);
    void setImageInfo(const ImageInfo& info);

protected:

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

private:

    void setImageId(qlonglong imageid);

    void readConfig();
    void writeConfig();

    void createNewFuzzySearchAlbumFromSketch(const QString& name);
    void createNewFuzzySearchAlbumFromImage(const QString& name);

    bool checkName(QString& name);
    bool checkAlbum(const QString& name) const;

private slots:

    void slotTabChanged(int);

    void slotHSChanged(int h, int s);
    void slotVChanged();
    void slotClearSketch();
    void slotDirtySketch();
    void slotSaveSketchSAlbum();
    void slotCheckNameEditSketchConditions();

    void slotAlbumSelected(SAlbum*);
    void slotRenameAlbum(SAlbum*);

    void slotSaveImageSAlbum();
    void slotCheckNameEditImageConditions();
    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    FuzzySearchViewPriv *d;
};

}  // NameSpace Digikam

#endif /* FUZZYSEARCHVIEW_H */
