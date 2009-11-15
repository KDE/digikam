/*
 * leftsidebarwidgets.h
 *
 *  Created on: 14.11.2009
 *      Author: languitar
 */

#ifndef LEFTSIDEBARWIDGETS_H
#define LEFTSIDEBARWIDGETS_H

// KDE includes
#include <kconfiggroup.h>

// Local includes
#include "sidebarwidget.h"
#include "albummodel.h"
#include "imagealbumfiltermodel.h"
#include "gpssearchfolderview.h"
#include "gpssearchview.h"

namespace Digikam
{

class AlbumFolderViewSideBarWidgetPriv;
/**
 * SideBarWidget for the folder view.
 *
 * @author jwienke
 */
class AlbumFolderViewSideBarWidget : public SideBarWidget
{
    Q_OBJECT
public:
    AlbumFolderViewSideBarWidget(QWidget *parent, AlbumModel *model);
    virtual ~AlbumFolderViewSideBarWidget();

    void setActive(bool active);
    void loadViewState(KConfigGroup &group);
    void saveViewState(KConfigGroup &group);
    void applySettings();
    void changeAlbumFromHistory(Album *album);
    QPixmap getIcon();
    QString getCaption();

public Q_SLOTS:
    void slotSelectAlbum(Album *album);

Q_SIGNALS:
    void signalFindDuplicatesInAlbum(Album*);

private:
    AlbumFolderViewSideBarWidgetPriv *d;

};

class TagViewSideBarWidgetPriv;
/**
 * SideBarWidget for the tag view.
 *
 * @author jwienke
 */
class TagViewSideBarWidget : public SideBarWidget
{
    Q_OBJECT
public:
    TagViewSideBarWidget(QWidget *parent, TagModel *model);
    virtual ~TagViewSideBarWidget();

    void setActive(bool active);
    void loadViewState(KConfigGroup &group);
    void saveViewState(KConfigGroup &group);
    void applySettings();
    void changeAlbumFromHistory(Album *album);
    QPixmap getIcon();
    QString getCaption();

    // TODO update, mainly legacy methods while not on mvc
    void refresh();
    void selectItem(int itemId);

    // TODO update, legacy signals
Q_SIGNALS:
    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);

    // TODO update, legacy slots
public Q_SLOTS:
    void slotNewTag();
    void slotDeleteTag();
    void slotEditTag();

Q_SIGNALS:
    void signalFindDuplicatesInAlbum(Album*);

private:
    TagViewSideBarWidgetPriv *d;

};

class DateFolderViewSideBarWidgetPriv;
/**
 * SideBarWidget for the date folder view.
 *
 * @author jwienke
 */
class DateFolderViewSideBarWidget : public SideBarWidget
{
    Q_OBJECT
public:
    DateFolderViewSideBarWidget(QWidget *parent, DateAlbumModel *model,
                    ImageAlbumFilterModel *imageFilterModel);
    virtual ~DateFolderViewSideBarWidget();

    void setActive(bool active);
    void loadViewState(KConfigGroup &group);
    void saveViewState(KConfigGroup &group);
    void applySettings();
    void changeAlbumFromHistory(Album *album);
    QPixmap getIcon();
    QString getCaption();

    void gotoDate(const QDate &date);

    // TODO update, mainly legacy methods while not on mvc
    void refresh();

private:
    DateFolderViewSideBarWidgetPriv *d;

};

class TimelineSideBarWidgetPriv;
/**
 * SideBarWidget for the date folder view.
 *
 * @author jwienke
 */
class TimelineSideBarWidget : public SideBarWidget
{
    Q_OBJECT
public:
    TimelineSideBarWidget(QWidget *parent);
    virtual ~TimelineSideBarWidget();

    void setActive(bool active);
    void loadViewState(KConfigGroup &group);
    void saveViewState(KConfigGroup &group);
    void applySettings();
    void changeAlbumFromHistory(Album *album);
    QPixmap getIcon();
    QString getCaption();

private:
    TimelineSideBarWidgetPriv *d;

};

class SearchSideBarWidgetPriv;
/**
 * SideBarWidget for the search.
 *
 * @author jwienke
 */
class SearchSideBarWidget : public SideBarWidget
{
    Q_OBJECT
public:
    SearchSideBarWidget(QWidget *parent);
    virtual ~SearchSideBarWidget();

    void setActive(bool active);
    void loadViewState(KConfigGroup &group);
    void saveViewState(KConfigGroup &group);
    void applySettings();
    void changeAlbumFromHistory(Album *album);
    QPixmap getIcon();
    QString getCaption();

    void newKeywordSearch();
    void newAdvancedSearch();

private:
    SearchSideBarWidgetPriv *d;

};

class FuzzySearchSideBarWidgetPriv;
/**
 * SideBarWidget for the fuzzy search.
 *
 * @author jwienke
 */
class FuzzySearchSideBarWidget : public SideBarWidget
{
    Q_OBJECT
public:
    FuzzySearchSideBarWidget(QWidget *parent);
    virtual ~FuzzySearchSideBarWidget();

    void setActive(bool active);
    void loadViewState(KConfigGroup &group);
    void saveViewState(KConfigGroup &group);
    void applySettings();
    void changeAlbumFromHistory(Album *album);
    QPixmap getIcon();
    QString getCaption();

    void newDuplicatesSearch(Album *album);
    void newSimilarSearch(const ImageInfo &imageInfo);

Q_SIGNALS:
    void signalUpdateFingerPrints();
    void signalGenerateFingerPrintsFirstTime();

private:
    FuzzySearchSideBarWidgetPriv *d;

};

#ifdef HAVE_MARBLEWIDGET
class GPSSearchSideBarWidgetPriv;
/**
 * SideBarWidget for the fuzzy search.
 *
 * @author jwienke
 */
class GPSSearchSideBarWidget : public SideBarWidget
{
    Q_OBJECT
public:
    GPSSearchSideBarWidget(QWidget *parent);
    virtual ~GPSSearchSideBarWidget();

    void setActive(bool active);
    void loadViewState(KConfigGroup &group);
    void saveViewState(KConfigGroup &group);
    void applySettings();
    void changeAlbumFromHistory(Album *album);
    QPixmap getIcon();
    QString getCaption();

Q_SIGNALS:
    void signalMapSelectedItems(const KUrl::List);
    void signalMapSoloItems(const KUrl::List, const QString&);

public Q_SLOTS:
    void slotDigikamViewNoCurrentItem();
    void slotDigikamViewImageSelected(const ImageInfoList &selectedImage, bool hasPrevious, bool hasNext, const ImageInfoList &allImages);

private:
    GPSSearchSideBarWidgetPriv *d;

};
#endif

}

#endif /* LEFTSIDEBARWIDGETS_H */
