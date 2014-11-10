/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : left sidebar widgets
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Moahmed Anwer <mohammed dot ahmed anwer at gmail dot com>
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

#ifndef LEFTSIDEBARWIDGETS_H
#define LEFTSIDEBARWIDGETS_H

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "config-digikam.h"
#include "albummodel.h"
#include "albummodificationhelper.h"
#include "imagealbumfiltermodel.h"
#include "searchmodificationhelper.h"
#include "sidebarwidget.h"
#include "imagefiltermodel.h"
#include "albumlabelstreeview.h"

#ifdef HAVE_KGEOMAP
#include "gpssearchview.h"
#endif // HAVE_KGEOMAP

namespace Digikam
{

template <class T>
class AlbumPointer;

/**
 * SideBarWidget for the folder view.
 *
 * @author jwienke
 */
class AlbumFolderViewSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    AlbumFolderViewSideBarWidget(QWidget* const parent, AlbumModel* const model,
                                 AlbumModificationHelper* const albumModificationHelper);
    virtual ~AlbumFolderViewSideBarWidget();

    void    setActive(bool active);
    void    doLoadState();
    void    doSaveState();
    void    applySettings();
    void    changeAlbumFromHistory(QList<Album*> album);
    QPixmap getIcon();
    QString getCaption();

    AlbumPointer<PAlbum> currentAlbum() const;

public Q_SLOTS:

    void setCurrentAlbum(PAlbum* album);

Q_SIGNALS:

    void signalFindDuplicatesInAlbum(Album*);

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------------------------------------

/**
 * SideBarWidget for the tag view.
 *
 * @author jwienke
 */
class TagViewSideBarWidget : public SidebarWidget
{

    Q_OBJECT

public:

    TagViewSideBarWidget(QWidget* const parent, TagModel* const model);
    virtual ~TagViewSideBarWidget();

    void    setActive(bool active);
    void    doLoadState();
    void    doSaveState();
    void    applySettings();
    void    changeAlbumFromHistory(QList<Album*> album);
    QPixmap getIcon();
    QString getCaption();

    AlbumPointer<TAlbum> currentAlbum() const;

private:

    void setNoTagsAlbum();

public Q_SLOTS:

    void setCurrentAlbum(TAlbum* album);
    void slotOpenTagManager();
    void slotToggleTagsSelection(int radioClicked);

Q_SIGNALS:

    void signalFindDuplicatesInAlbum(Album*);

public:

    // Declared as public due to use by Private
    class Private;

private:

    Private* const d;
};

// -----------------------------------------------------------------------------------------

/**
 * SideBarWidget for the Labels.
 *
 * @author Mohamed Anwer
 */
class LabelsSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    LabelsSideBarWidget(QWidget* const parent);
    virtual ~LabelsSideBarWidget();

    AlbumLabelsTreeView* labelsTree();

    void    setActive(bool active);
    void    applySettings();
    void    changeAlbumFromHistory(QList<Album *> album);
    void    doLoadState();
    void    doSaveState();
    QPixmap getIcon();
    QString getCaption();

    QHash<AlbumLabelsTreeView::Labels, QList<int> > selectedLabels();

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------------------------------------

/**
 * SideBarWidget for the date folder view.
 *
 * @author jwienke
 */
class DateFolderViewSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    DateFolderViewSideBarWidget(QWidget* const parent, DateAlbumModel* const model,
                                ImageAlbumFilterModel* const imageFilterModel);
    virtual ~DateFolderViewSideBarWidget();

    void    setActive(bool active);
    void    doLoadState();
    void    doSaveState();
    void    applySettings();
    void    changeAlbumFromHistory(QList<Album*> album);
    QPixmap getIcon();
    QString getCaption();

    AlbumPointer<DAlbum> currentAlbum() const;

    void gotoDate(const QDate& date);

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------------------------------------

/**
 * SideBarWidget for the date folder view.
 *
 * @author jwienke
 */
class TimelineSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    TimelineSideBarWidget(QWidget* const parent, SearchModel* const searchModel,
                          SearchModificationHelper* const searchModificationHelper);
    virtual ~TimelineSideBarWidget();

    void    setActive(bool active);
    void    doLoadState();
    void    doSaveState();
    void    applySettings();
    void    changeAlbumFromHistory(QList<Album*> album);
    QPixmap getIcon();
    QString getCaption();

private Q_SLOTS:

    void slotInit();
    void slotScrollBarValueChanged(int);
    void slotRefDateTimeChanged();
    void slotScaleChanged(int);
    void slotTimeUnitChanged(int);
    void slotCursorPositionChanged();
    void slotSelectionChanged();
    void slotResetSelection();
    void slotSaveSelection();
    void slotUpdateCurrentDateSearchAlbum();
    void slotAlbumSelected(Album*);
    void slotCheckAboutSelection();

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------------------------------------

/**
 * SideBarWidget for the search.
 *
 * @author jwienke
 */
class SearchSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    SearchSideBarWidget(QWidget* const parent, SearchModel* const searchModel,
                        SearchModificationHelper* const searchModificationHelper);
    virtual ~SearchSideBarWidget();

    void    setActive(bool active);
    void    doLoadState();
    void    doSaveState();
    void    applySettings();
    void    changeAlbumFromHistory(QList<Album*> album);
    QPixmap getIcon();
    QString getCaption();

    void newKeywordSearch();
    void newAdvancedSearch();

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------------------------------------

/**
 * SideBarWidget for the fuzzy search.
 *
 * @author jwienke
 */
class FuzzySearchSideBarWidget : public SidebarWidget
{
    Q_OBJECT
public:
    FuzzySearchSideBarWidget(QWidget* const parent, SearchModel* const searchModel,
                             SearchModificationHelper* const searchModificationHelper);
    virtual ~FuzzySearchSideBarWidget();

    void    setActive(bool active);
    void    doLoadState();
    void    doSaveState();
    void    applySettings();
    void    changeAlbumFromHistory(QList<Album*> album);
    QPixmap getIcon();
    QString getCaption();

    void newDuplicatesSearch(Album* album);
    void newSimilarSearch(const ImageInfo& imageInfo);

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------------------------------------

#ifdef HAVE_KGEOMAP

/**
 * SideBarWidget for the gps search.
 *
 * @author jwienke
 */
class GPSSearchSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    GPSSearchSideBarWidget(QWidget* const parent, SearchModel* const searchModel,
                           SearchModificationHelper* const searchModificationHelper,
                           ImageFilterModel* const imageFilterModel, QItemSelectionModel* const itemSelectionModel);
    virtual ~GPSSearchSideBarWidget();

    void    setActive(bool active);
    void    doLoadState();
    void    doSaveState();
    void    applySettings();
    void    changeAlbumFromHistory(QList<Album*> album);
    QPixmap getIcon();
    QString getCaption();

Q_SIGNALS:

    void signalMapSoloItems(const QList<qlonglong>&, const QString&);

private:

    class Private;
    Private* const d;
};

#endif // HAVE_KGEOMAP

// -----------------------------------------------------------------------------------------

#ifdef HAVE_KFACE

/**
 * SideBarWidget for People
 *
 * @author Aditya Bhatt
 */
class PeopleSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    PeopleSideBarWidget(QWidget* const parent, TagModel* const tagModel,
                        SearchModificationHelper* const searchModificationHelper);
    virtual ~PeopleSideBarWidget();

    void    setActive(bool active);
    void    doLoadState();
    void    doSaveState();
    void    applySettings();
    void    changeAlbumFromHistory(QList<Album*> album);
    QPixmap getIcon();
    QString getCaption();

private Q_SLOTS:

    void slotInit();
    void slotScanForFaces();

Q_SIGNALS:

    void requestFaceMode(bool on);

    void signalFindDuplicatesInAlbum(Album*);

private:

    class Private;
    Private* const d;
};

#endif /* HAVE_KFACE */

} // namespace Digikam

#endif /* LEFTSIDEBARWIDGETS_H */
