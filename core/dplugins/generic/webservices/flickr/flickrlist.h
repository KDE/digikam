/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : Flickr/23HQ file list view and items.
 *
 * Copyright (C) 2009      by Pieter Edelman <pieter dot edelman at gmx dot net>
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_FLICKR_LIST_H
#define DIGIKAM_FLICKR_LIST_H

// Qt includes

#include <QUrl>
#include <QList>
#include <QLineEdit>

// Local includes

#include "ditemslist.h"

namespace Digikam
{

class FlickrList : public DItemsList
{
    Q_OBJECT

public:

    /** The different columns in a Flickr list.
     */
    enum FieldType
    {
        SAFETYLEVEL = DItemsListView::User1,
        CONTENTTYPE = DItemsListView::User2,
        TAGS        = DItemsListView::User3,
        PUBLIC      = DItemsListView::User4,
        FAMILY      = DItemsListView::User5,
        FRIENDS     = DItemsListView::User6
    };

    /** The different possible safety levels recognized by Flickr.
     */
    enum SafetyLevel
    {
        SAFE        = 1,
        MODERATE    = 2,
        RESTRICTED  = 3,
        MIXEDLEVELS = -1
    };

    /** The different possible content types recognized by Flickr.
     */
    enum ContentType
    {
        PHOTO      = 1,
        SCREENSHOT = 2,
        OTHER      = 3,
        MIXEDTYPES = -1
    };

public:

    explicit FlickrList(QWidget* const parent = 0, bool = false);
    ~FlickrList();

    void setPublic(Qt::CheckState);
    void setFamily(Qt::CheckState);
    void setFriends(Qt::CheckState);
    void setSafetyLevels(SafetyLevel);
    void setContentTypes(ContentType);

Q_SIGNALS:

    // Signal for notifying when the states of one of the permission columns has
    // changed. The first argument specifies which permission has changed, the
    // second the state.
    void signalPermissionChanged(FlickrList::FieldType, Qt::CheckState);

    void signalSafetyLevelChanged(FlickrList::SafetyLevel);
    void signalContentTypeChanged(FlickrList::ContentType);

public Q_SLOTS:

    void slotAddImages(const QList<QUrl>& list) Q_DECL_OVERRIDE;

private:

    void setPermissionState(FieldType, Qt::CheckState);
    void singlePermissionChanged(QTreeWidgetItem*, int);
    void singleComboBoxChanged(QTreeWidgetItem*, int);


private Q_SLOTS:

    void slotItemChanged(QTreeWidgetItem*, int);
    void slotItemClicked(QTreeWidgetItem*, int);

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------

class FlickrListViewItem : public DItemsListViewItem
{

public:

    explicit FlickrListViewItem(DItemsListView* const view,
                                const QUrl& url,
                                bool, bool, bool, bool,
                                FlickrList::SafetyLevel,
                                FlickrList::ContentType);
    ~FlickrListViewItem();

    void setPublic(bool);
    void setFamily(bool);
    void setFriends(bool);
    void setSafetyLevel(FlickrList::SafetyLevel);
    void setContentType(FlickrList::ContentType);
    bool isPublic() const;
    bool isFamily() const;
    bool isFriends() const;
    FlickrList::SafetyLevel safetyLevel() const;
    FlickrList::ContentType contentType() const;

    /**
     * Returns the list of extra tags that the user specified for this image.
     */
    QStringList extraTags() const;

    /** This method should be called when one of the checkboxes is clicked.
     */
    void toggled();

    void updateItemWidgets() Q_DECL_OVERRIDE;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FLICKR_LIST_H
