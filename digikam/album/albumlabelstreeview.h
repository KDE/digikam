/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Album Labels Tree View.
 *
 * Copyright (C) 2014 Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#ifndef ALBUMLABELSTREEVIEW_H
#define ALBUMLABELSTREEVIEW_H

// Qt includes

#include <QTreeWidget>

// KDE includes

#include <kio/job.h>

// Local includes

#include "databaseconstants.h"
#include "album.h"
#include <statesavingobject.h>

namespace Digikam
{

class AlbumLabelsSearchHandler;

class AlbumLabelsTreeView : public QTreeWidget, public StateSavingObject
{
    Q_OBJECT

public:
    explicit AlbumLabelsTreeView(QWidget *parent = 0, bool setCheckable = false);
    ~AlbumLabelsTreeView();

    bool    isCheckable();
    bool    isLoadingState();
    QPixmap goldenStarPixmap();
    QPixmap colorRectPixmap(QColor color);

    QHash<QString, QList<int> > selectedLabels();

    void doLoadState();
    void doSaveState();

    void restoreSelectionFromHistory(QHash<QString, QList<int> > neededLabels);
    void setCurrentAlbum();

private:
    void initTreeView();
    void initRatingsTree();
    void initPicksTree();
    void initColorsTree();

Q_SIGNALS:
    void signalSetCurrentAlbum();

private:
    class Private;
    Private* const d;
};

// --------------------------------------------------------------------

class AlbumLabelsSearchHandler : public QObject
{
    Q_OBJECT

public:
    explicit AlbumLabelsSearchHandler(AlbumLabelsTreeView* treeWidget);
    ~AlbumLabelsSearchHandler();

    Album*     albumForSelectedItems();
    KUrl::List imagesUrls();
    QString    generatedName();
    void       setCurrentAlbum();
    void       restoreSelectionFromHistory(QHash<QString, QList<int> > neededLabels);
    bool       isRestoringSelectionFromHistory();

private:
    QString createXMLForCurrentSelection(QHash<QString, QList<int> > selectedLabels);
    SAlbum* search(const QString& xml);

    void generateAlbumNameForExporting(QList<int> ratings, QList<int> colorsList, QList<int> picksList);
    void imagesUrlsForCurrentAlbum();

private Q_SLOTS:
    void slotSelectionChanged();
    void slotCheckStateChanged();
    void slotSetCurrentAlbum();
    void slotData(KIO::Job* job, QByteArray data);

Q_SIGNALS:
    void checkStateChanged(Album* album, Qt::CheckState checkState);

private:
    class Private;
    Private* const d;
};

} // namespace Digikam
#endif // ALBUMLABELSTREEVIEW_H
