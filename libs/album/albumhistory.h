/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : Albums history manager.
 *
 * Copyright (C) 2004      by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef ALBUMHISTORY_H
#define ALBUMHISTORY_H

/** @file albumhistory.h */

// Qt includes

#include <QList>
#include <QObject>
#include <QStringList>

// Local includes

#include "albumlabelstreeview.h"

namespace Digikam
{

class Album;
class HistoryItem;
class HistoryPosition;
class ImageInfo;
class ImageInfoList;

/**
 * Manages the history of the last visited albums.
 *
 * The user is able to navigate through the albums, he has
 * opened during a session.
 */
class AlbumHistory : public QObject
{
    Q_OBJECT

public:

    AlbumHistory();
    ~AlbumHistory();

    void addAlbums(QList<Album*> const albums, QWidget* const widget = 0);
    void addAlbums(QList<Album*> const albums, QWidget* const widget, QHash<AlbumLabelsTreeView::Labels, QList<int> > selectedLabels);
    void deleteAlbum(Album* const album);
    void clearHistory();
    void back(QList<Album*>& album, QWidget** const widget, unsigned int steps=1);
    void forward(QList<Album*>& album, QWidget** const widget, unsigned int steps=1);
    void getCurrentAlbum(Album** const album, QWidget** const widget) const;

    void getBackwardHistory(QStringList& list) const;
    void getForwardHistory(QStringList& list) const;

    bool isForwardEmpty() const;
    bool isBackwardEmpty() const;

    QHash<AlbumLabelsTreeView::Labels, QList<int> > neededLabels();

Q_SIGNALS:

    void signalSetCurrent(qlonglong imageId);
    void signalSetSelectedInfos(const QList<ImageInfo>&);

public Q_SLOTS:

    void slotAlbumCurrentChanged();
    void slotAlbumDeleted(Album* album);
    void slotAlbumsCleared();
    void slotAlbumSelected();
    void slotClearSelectPAlbum(const ImageInfo& imageInfo);
    void slotClearSelectTAlbum(int id);
    void slotCurrentChange(const ImageInfo& info);
    void slotImageSelected(const ImageInfoList& selectedImage);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ALBUMHISTORY_H */
