/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : item properties side bar using data from digiKam database.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_ITEM_PROPERTIES_SIDEBAR_DB_H
#define DIGIKAM_ITEM_PROPERTIES_SIDEBAR_DB_H

// Qt includes

#include <QUrl>
#include <QWidget>
#include <QRect>

// Local includes

#include "dimagehistory.h"
#include "iteminfolist.h"
#include "itempropertiessidebar.h"
#include "digikam_export.h"
#include "digikam_config.h"

namespace Digikam
{

class DImg;
class SidebarSplitter;
class ItemInfo;
class ImageChangeset;
class ItemDescEditTab;
class ImageTagChangeset;
class ItemPropertiesVersionsTab;
class GPSItemInfo;

class ItemPropertiesSideBarDB : public ItemPropertiesSideBar
{
    Q_OBJECT

public:

    explicit ItemPropertiesSideBarDB(QWidget* const parent,
                                      SidebarSplitter* const splitter,
                                      Qt::Edge side=Qt::LeftEdge,
                                      bool mimimizedDefault=false);
    ~ItemPropertiesSideBarDB();

    void populateTags();
    void refreshTagsView();

    ///This is for image editor to be able to update the filter list in sidebar
    ItemPropertiesVersionsTab* getFiltersHistoryTab() const;
    ItemDescEditTab*           imageDescEditTab()     const;

    virtual void itemChanged(const QUrl& url, const QRect& rect = QRect(), DImg* const img = 0);

    virtual void itemChanged(const ItemInfo& info, const QRect& rect = QRect(),
                             DImg* const img = 0, const DImageHistory& history = DImageHistory());

    virtual void itemChanged(const ItemInfoList& infos);


#ifdef HAVE_MARBLE

    static bool GPSItemInfofromItemInfo(const ItemInfo&, GPSItemInfo* const);

#endif // HAVE_MARBLE

Q_SIGNALS:

    void signalFirstItem();
    void signalPrevItem();
    void signalNextItem();
    void signalLastItem();

public Q_SLOTS:

    void slotAssignRating(int rating);
    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();

    void slotPopupTagsView();

    virtual void slotNoCurrentItem();

private Q_SLOTS:

    void slotChangedTab(QWidget* tab);
    void slotFileMetadataChanged(const QUrl& url);
    void slotImageChangeDatabase(const ImageChangeset& changeset);
    void slotImageTagChanged(const ImageTagChangeset& changeset);

private:

    void itemChanged(const QUrl& url, const ItemInfo& info, const QRect& rect, DImg* const img, const DImageHistory& history);
    void itemChanged(const ItemInfoList& infos, const QRect& rect, DImg* const img, const DImageHistory& history);
    void setImagePropertiesInformation(const QUrl& url);

protected:

    /**
     * load the last view state from disk - called by StateSavingObject#loadState()
     */
    void doLoadState();

    /**
     * save the view state to disk - called by StateSavingObject#saveState()
     */
    void doSaveState();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_PROPERTIES_SIDEBAR_DB_H
