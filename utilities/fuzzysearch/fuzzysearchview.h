/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * Copyright (C) 2016-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef FUZZY_SEARCH_VIEW_H
#define FUZZY_SEARCH_VIEW_H

// Qt includes

#include <QScrollArea>

// Local includs

#include "statesavingobject.h"

class QDragEnterEvent;
class QDropEvent;
class QPixmap;

namespace Digikam
{

class Album;
class FuzzySearchFolderView;
class ImageInfo;
class LoadingDescription;
class SAlbum;
class PAlbum;
class TAlbum;
class SearchModel;
class SearchModificationHelper;
class SearchTextBar;

class FuzzySearchView : public QScrollArea, public StateSavingObject
{
    Q_OBJECT

public:

    explicit FuzzySearchView(SearchModel* const searchModel,
                             SearchModificationHelper* const searchModificationHelper,
                             QWidget* const parent = 0);
    virtual ~FuzzySearchView();

    SAlbum* currentAlbum() const;
    void setCurrentAlbum(SAlbum* const album);

    void setActive(bool val);
    void setImageInfo(const ImageInfo& info);

    void newDuplicatesSearch(PAlbum* const);
    void newDuplicatesSearch(QList<PAlbum*> const);
    void newDuplicatesSearch(QList<TAlbum*> const);

    virtual void setConfigGroup(const KConfigGroup& group);
    void doLoadState();
    void doSaveState();

protected:

    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);

private Q_SLOTS:

    void slotTabChanged(int);

    void slotHSChanged(int h, int s);
    void slotVChanged(int v);
    void slotPenColorChanged(const QColor&);
    void slotClearSketch();
    void slotSaveSketchSAlbum();
    void slotCheckNameEditSketchConditions();

    void slotAlbumSelected(Album* album);

    void slotSaveImageSAlbum();
    void slotCheckNameEditImageConditions();
    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

    void slotDirtySketch();
    void slotTimerSketchDone();
    void slotUndoRedoStateChanged(bool, bool);

    void slotMinLevelImageChanged(int);
    void slotMaxLevelImageChanged(int);
    void slotFuzzyAlbumsChanged();
    void slotTimerImageDone();

    void slotApplicationSettingsChanged();

private:

    void setCurrentImage(qlonglong imageid);
    void setCurrentImage(const ImageInfo& info);

    void createNewFuzzySearchAlbumFromSketch(const QString& name, bool force = false);
    void createNewFuzzySearchAlbumFromImage(const QString& name, bool force = false);

    void setColor(QColor c);

    QWidget* setupFindSimilarPanel() const;
    QWidget* setupSketchPanel()      const;
    void     setupConnections();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // FUZZY_SEARCH_VIEW_H
