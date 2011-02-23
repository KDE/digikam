/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : left sidebar widgets
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
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

#include "albummodel.h"
#include "albummodificationhelper.h"
#include "gpssearchview.h"
#include "imagealbumfiltermodel.h"
#include "searchmodificationhelper.h"
#include "sidebarwidget.h"
#include "imagefiltermodel.h"


namespace Digikam
{

template <class T>
class AlbumPointer;

class AlbumFolderViewSideBarWidgetPriv;
/**
 * SideBarWidget for the folder view.
 *
 * @author jwienke
 */
class AlbumFolderViewSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    AlbumFolderViewSideBarWidget(QWidget* parent, AlbumModel* model,
                                 AlbumModificationHelper* albumModificationHelper);
    virtual ~AlbumFolderViewSideBarWidget();

    void setActive(bool active);
    void doLoadState();
    void doSaveState();
    void applySettings();
    void changeAlbumFromHistory(Album* album);
    AlbumPointer<PAlbum> currentAlbum() const;
    QPixmap getIcon();
    QString getCaption();

public Q_SLOTS:

    void setCurrentAlbum(PAlbum* album);

Q_SIGNALS:

    void signalFindDuplicatesInAlbum(Album*);

private:

    AlbumFolderViewSideBarWidgetPriv* const d;
};

class TagViewSideBarWidgetPriv;
/**
 * SideBarWidget for the tag view.
 *
 * @author jwienke
 */
class TagViewSideBarWidget : public SidebarWidget
{

    Q_OBJECT

public:

    TagViewSideBarWidget(QWidget* parent, TagModel* model);
    virtual ~TagViewSideBarWidget();

    void setActive(bool active);
    void doLoadState();
    void doSaveState();
    void applySettings();
    void changeAlbumFromHistory(Album* album);
    AlbumPointer<TAlbum> currentAlbum() const;
    QPixmap getIcon();
    QString getCaption();

public Q_SLOTS:

    void setCurrentAlbum(TAlbum* album);

Q_SIGNALS:

    void signalFindDuplicatesInAlbum(Album*);

private:

    TagViewSideBarWidgetPriv* const d;
};

class DateFolderViewSideBarWidgetPriv;
/**
 * SideBarWidget for the date folder view.
 *
 * @author jwienke
 */
class DateFolderViewSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    DateFolderViewSideBarWidget(QWidget* parent, DateAlbumModel* model,
                                ImageAlbumFilterModel* imageFilterModel);
    virtual ~DateFolderViewSideBarWidget();

    void setActive(bool active);
    void doLoadState();
    void doSaveState();
    void applySettings();
    void changeAlbumFromHistory(Album* album);
    AlbumPointer<DAlbum> currentAlbum() const;
    QPixmap getIcon();
    QString getCaption();

    void gotoDate(const QDate& date);

private:

    DateFolderViewSideBarWidgetPriv* const d;
};

class TimelineSideBarWidgetPriv;
/**
 * SideBarWidget for the date folder view.
 *
 * @author jwienke
 */
class TimelineSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    TimelineSideBarWidget(QWidget* parent, SearchModel* searchModel,
                          SearchModificationHelper* searchModificationHelper);
    virtual ~TimelineSideBarWidget();

    void setActive(bool active);
    void doLoadState();
    void doSaveState();
    void applySettings();
    void changeAlbumFromHistory(Album* album);
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

    TimelineSideBarWidgetPriv* const d;
};

class SearchSideBarWidgetPriv;
/**
 * SideBarWidget for the search.
 *
 * @author jwienke
 */
class SearchSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    SearchSideBarWidget(QWidget* parent, SearchModel* searchModel,
                        SearchModificationHelper* searchModeificationHelper);
    virtual ~SearchSideBarWidget();

    void setActive(bool active);
    void doLoadState();
    void doSaveState();
    void applySettings();
    void changeAlbumFromHistory(Album* album);
    QPixmap getIcon();
    QString getCaption();

    void newKeywordSearch();
    void newAdvancedSearch();

private:

    SearchSideBarWidgetPriv* const d;
};

class FuzzySearchSideBarWidgetPriv;
/**
 * SideBarWidget for the fuzzy search.
 *
 * @author jwienke
 */
class FuzzySearchSideBarWidget : public SidebarWidget
{
    Q_OBJECT
public:
    FuzzySearchSideBarWidget(QWidget* parent, SearchModel* searchModel,
                             SearchModificationHelper* searchModificationHelper);
    virtual ~FuzzySearchSideBarWidget();

    void setActive(bool active);
    void doLoadState();
    void doSaveState();
    void applySettings();
    void changeAlbumFromHistory(Album* album);
    QPixmap getIcon();
    QString getCaption();

    void newDuplicatesSearch(Album* album);
    void newSimilarSearch(const ImageInfo& imageInfo);

Q_SIGNALS:

    void signalUpdateFingerPrints();
    void signalGenerateFingerPrintsFirstTime();

private:

    FuzzySearchSideBarWidgetPriv* const d;
};

class GPSSearchSideBarWidgetPriv;
/**
 * SideBarWidget for the gps search.
 *
 * @author jwienke
 */
class GPSSearchSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    GPSSearchSideBarWidget(QWidget* parent, SearchModel* searchModel,
                           SearchModificationHelper* searchModificationHelper,
                           ImageFilterModel* imageFilterModel, QItemSelectionModel* itemSelectionModel);
    virtual ~GPSSearchSideBarWidget();

    void setActive(bool active);
    void doLoadState();
    void doSaveState();
    void applySettings();
    void changeAlbumFromHistory(Album* album);
    QPixmap getIcon();
    QString getCaption();

Q_SIGNALS:

    void signalMapSoloItems(const QList<qlonglong>&, const QString&);

private:

    GPSSearchSideBarWidgetPriv* const d;
};

class PeopleSideBarWidgetPriv;
/**
 * SideBarWidget for People
 *
 * @author Aditya Bhatt
 */
class PeopleSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    PeopleSideBarWidget(QWidget* parent, TagModel* tagModel,
                        SearchModificationHelper* searchModificationHelper);
    virtual ~PeopleSideBarWidget();

    void setActive(bool active);
    void doLoadState();
    void doSaveState();
    void applySettings();
    void changeAlbumFromHistory(Album* album);
    QPixmap getIcon();
    QString getCaption();

private Q_SLOTS:

    void slotInit();

Q_SIGNALS:

    void requestFaceMode(bool on);

    void signalDetectFaces();
    void signalScanForFacesFirstTime();
    void signalResumeScanForFaces();
    void signalFindDuplicatesInAlbum(Album*);

private:

    PeopleSideBarWidgetPriv* const d;

};

} // namespace Digikam

#endif /* LEFTSIDEBARWIDGETS_H */
