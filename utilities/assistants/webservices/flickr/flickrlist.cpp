/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : Flickr/23HQ file list view and items.
 *
 * Copyright (C) 2009 by Pieter Edelman <pieter dot edelman at gmx dot net>
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

#include "flickrlist.h"

// Qt includes

#include <QApplication>
#include <QComboBox>
#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "wscomboboxdelegate.h"

namespace Digikam
{

FlickrList::FlickrList(QWidget* const parent, bool is_23)
    : DImagesList(parent),
      m_public(Qt::Unchecked),
      m_family(Qt::Unchecked),
      m_friends(Qt::Unchecked),
      m_safetyLevel(FlickrList::SAFE),
      m_contentType(FlickrList::PHOTO),
      m_userIsEditing(false),
      m_is23(is_23)
{
    // Catch a click on the items.
    connect(listView(), SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemClicked(QTreeWidgetItem*,int)));

    // Catch it if the items change.
    connect(listView(), SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

void FlickrList::setPublic(Qt::CheckState isPublic)
{
    /* Change the general public flag for photos in the list. */
    m_public = isPublic;
    setPermissionState(PUBLIC, isPublic);
}

void FlickrList::setFamily(Qt::CheckState isFamily)
{
    /* Change the general family flag for photos in the list. */
    m_family = isFamily;
    setPermissionState(FAMILY, m_family);
}

void FlickrList::setFriends(Qt::CheckState isFriends)
{
    /* Change the general friends flag for photos in the list. */
    m_friends = isFriends;
    setPermissionState(FRIENDS, m_friends);
}

void FlickrList::setSafetyLevels(SafetyLevel safetyLevel)
{
    /* Change the general safety level for photos in the list. */
    m_safetyLevel = safetyLevel;

    if (safetyLevel != MIXEDLEVELS)
    {
        for (int i = 0; i < listView()->topLevelItemCount(); ++i)
        {
            FlickrListViewItem* const lvItem = dynamic_cast<FlickrListViewItem*>(listView()->topLevelItem(i));

            if (lvItem)
                lvItem->setSafetyLevel(m_safetyLevel);
        }
    }
}

void FlickrList::setContentTypes(ContentType contentType)
{
    /* Change the general content type for photos in the list. */
    m_contentType = contentType;

    if (contentType != MIXEDTYPES)
    {
        for (int i = 0; i < listView()->topLevelItemCount(); ++i)
        {
            FlickrListViewItem* const lvItem = dynamic_cast<FlickrListViewItem*>(listView()->topLevelItem(i));

            if (lvItem)
                lvItem->setContentType(m_contentType);
        }
    }
}

void FlickrList::setPermissionState(FieldType type, Qt::CheckState state)
{
    /* When the state of one of the three permission levels changes, distribute
     * the global change to each individual photo. */

    if (state != Qt::PartiallyChecked)
    {
        for (int i = 0; i < listView()->topLevelItemCount(); ++i)
        {
            FlickrListViewItem* const lvItem = dynamic_cast<FlickrListViewItem*>(listView()->topLevelItem(i));

            if (lvItem)
            {
                if (type == PUBLIC)
                {
                    lvItem->setPublic(state);
                }
                else if (type == FAMILY)
                {
                    lvItem->setFamily(state);
                }
                else if (type == FRIENDS)
                {
                    lvItem->setFriends(state);
                }
            }
        }
    }
}

void FlickrList::slotItemClicked(QTreeWidgetItem* item, int column)
{
    // If a click occurs from one of the three permission checkbox columns,
    // it means something has changed in the permissions.
    if ((column == PUBLIC) || (column == FAMILY) || (column == FRIENDS))
    {
        singlePermissionChanged(item, column);
    }
    // If a click occurs in the Safety Level or Content Type column, it means
    // that editing should start on these items.
    else if ((column == static_cast<int>(FlickrList::SAFETYLEVEL)) || (column == static_cast<int>(FlickrList::CONTENTTYPE)))
    {
        m_userIsEditing                    = true;
        ComboBoxDelegate* const cbDelegate = dynamic_cast<ComboBoxDelegate*>(listView()->itemDelegateForColumn(column));

        if (cbDelegate)
        {
            cbDelegate->startEditing(item, column);
        }
    }
}

void FlickrList::slotItemChanged(QTreeWidgetItem* item, int column)
{
    // If an item in the Safety Level or Content Type column changes, it should
    // be distributed further.
    if ((column == SAFETYLEVEL) || (column == CONTENTTYPE))
    {
        singleComboBoxChanged(item, column);
    }
}

void FlickrList::singlePermissionChanged(QTreeWidgetItem* item, int column)
{
    /* Callback for when the user clicks a checkbox in one of the permission
     * columns. */

    if ((column == PUBLIC) || (column == FAMILY) || (column == FRIENDS))
    {
        // Call the toggled() method of the item on which the selection
        // occurred.
        FlickrListViewItem* const lvItem = dynamic_cast<FlickrListViewItem*>(item);

        if (lvItem)
        {
            lvItem->toggled();

            // Count the number of set checkboxes for the selected column.
            int numChecked = 0;

            for (int i = 0; i < listView()->topLevelItemCount(); ++i)
            {
                FlickrListViewItem* const titem = dynamic_cast<FlickrListViewItem*>(listView()->topLevelItem(i));

                if (titem)
                {
                    if (((column == PUBLIC)  && (titem->isPublic())) ||
                        ((column == FAMILY)  && (titem->isFamily())) ||
                        ((column == FRIENDS) && (titem->isFriends())))
                    {
                        numChecked += 1;
                    }
                }
            }

            // Determine the new state.
            Qt::CheckState state = Qt::PartiallyChecked;

            if (numChecked == 0)
            {
                state = Qt::Unchecked;
            }
            else if (numChecked == listView()->topLevelItemCount())
            {
                state = Qt::Checked;
            }

            // If needed, signal the change.
            if ((column == PUBLIC) && (state != m_public))
            {
                setPublic(state);
                emit signalPermissionChanged(PUBLIC, state);
            }

            if ((column == FAMILY) && (state != m_family))
            {
                setFamily(state);
                emit signalPermissionChanged(FAMILY, state);
            }

            if ((column == FRIENDS) && (state != m_friends))
            {
                setFriends(state);
                emit signalPermissionChanged(FRIENDS, state);
            }
        }
    }
}

void FlickrList::singleComboBoxChanged(QTreeWidgetItem* item, int column)
{
    /* Callback for when one of the comboboxes for Safety Level or Content
     * Type changes. */

    // Make sure to only process changes from user editing, because this
    // function also responds to programmatic changes, which it causes itself
    // again.
    if (((column == SAFETYLEVEL) || (column == CONTENTTYPE)) && m_userIsEditing)
    {
        // The user has stopped editing.
        m_userIsEditing = false;

        // Convert the value from the model to the setting for the
        // FlickrListViewItem.
        FlickrListViewItem* const lvItem = dynamic_cast<FlickrListViewItem*>(item);

        if (lvItem)
        {
            int data = lvItem->data(column, Qt::DisplayRole).toInt();

            if (column == SAFETYLEVEL)
            {
                lvItem->setSafetyLevel(static_cast<SafetyLevel>(data));
            }
            else if (column == CONTENTTYPE)
            {
                lvItem->setContentType(static_cast<ContentType>(data));
            }

            // Determine how much photos are set to different Safety Levels/Content
            // Types.
            QMap<int, int> nums = QMap<int, int>();

            for (int i = 0; i < listView()->topLevelItemCount(); ++i)
            {
                FlickrListViewItem* const titem = dynamic_cast<FlickrListViewItem*>(listView()->topLevelItem(i));

                if (titem)
                {
                    if (column == SAFETYLEVEL)
                    {
                        nums[lvItem->safetyLevel()]++;
                    }
                    else if (column == CONTENTTYPE)
                    {
                        nums[lvItem->contentType()]++;
                    }
                }
            }

            // If there's only one Safety Level or Content Type, make everything
            // uniform
            if (nums.count() == 1)
            {
                QMapIterator<int, int> i(nums);
                i.next();

                if (column == SAFETYLEVEL)
                {
                    SafetyLevel safetyLevel = static_cast<SafetyLevel>(i.key());
                    setSafetyLevels(safetyLevel);
                    emit signalSafetyLevelChanged(safetyLevel);
                }
                else if (column == CONTENTTYPE)
                {
                    ContentType contentType = static_cast<ContentType>(i.key());
                    setContentTypes(contentType);
                    emit signalContentTypeChanged(contentType);
                }
            }
            // If there are different Safety Levels/Content Types among the photos,
            // signal that.
            else
            {
                if (column == SAFETYLEVEL)
                {
                    setSafetyLevels(MIXEDLEVELS);
                    emit signalSafetyLevelChanged(MIXEDLEVELS);
                }
                else if (column == CONTENTTYPE)
                {
                    setContentTypes(MIXEDTYPES);
                    emit signalContentTypeChanged(MIXEDTYPES);
                }
            }
        }
    }
}

void FlickrList::slotAddImages(const QList<QUrl>& list)
{
    /* Replaces the ImagesList::slotAddImages method, so that
     * FlickrListViewItems can be added instead of ImagesListViewItems */

    // Figure out which permissions should be used. If permissions are set to
    // intermediate, default to the most public option.
    bool isPublic, isFamily, isFriends;
    (m_public  == Qt::PartiallyChecked) ? isPublic  = true : isPublic  = m_public;
    (m_family  == Qt::PartiallyChecked) ? isFamily  = true : isFamily  = m_family;
    (m_friends == Qt::PartiallyChecked) ? isFriends = true : isFriends = m_friends;

    // Figure out safety level and content type. If these are intermediate, use
    // the Flickr defaults.
    SafetyLevel safetyLevel;
    ContentType contentType;
    (m_safetyLevel == MIXEDLEVELS) ? safetyLevel = SAFE  : safetyLevel = m_safetyLevel;
    (m_contentType == MIXEDTYPES)  ? contentType = PHOTO : contentType = m_contentType;

    // Figure out which of the supplied URL's should actually be added and which
    // of them already exist.
    bool found;
    QList<QUrl> added_urls;
    QList<QUrl>::const_iterator it;

    for (it = list.constBegin(); it != list.constEnd(); ++it)
    {
        QUrl imageUrl = *it;
        found         = false;

        for (int i = 0; i < listView()->topLevelItemCount(); ++i)
        {
            FlickrListViewItem* const currItem = dynamic_cast<FlickrListViewItem*>(listView()->topLevelItem(i));

            if (currItem && currItem->url() == imageUrl)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Insterting new item " << imageUrl.fileName();
            new FlickrListViewItem(listView(), imageUrl, m_is23,
                                   isPublic, isFamily, isFriends,
                                   safetyLevel, contentType);
            added_urls.append(imageUrl);
        }
    }

    // Duplicate the signalImageListChanged of the ImageWindow, to enable the
    // upload button again.
    emit signalImageListChanged();
}

// ------------------------------------------------------------------------------------------------

FlickrListViewItem::FlickrListViewItem(DImagesListView* const view,
                                       const QUrl& url,
                                       bool is23 = false,
                                       bool accessPublic  = true,
                                       bool accessFamily  = true,
                                       bool accessFriends = true,
                                       FlickrList::SafetyLevel safetyLevel = FlickrList::SAFE,
                                       FlickrList::ContentType contentType = FlickrList::PHOTO)
    : DImagesListViewItem(view, url),
      m_is23(is23)
{
    /* Initialize the FlickrListViewItem with the ImagesListView and a QUrl
     * object pointing to the location on disk.
     * If the photo is meant for 23HQ, the service_23 flag should be set to
     * true.
     * The access_public, access_family and access_friends flags determine if
     * the public, family and friends permissions of this particular photo. */

    // Set the flags for checkboxes to appear
    setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    // Set the text and checkbox for the public column.
    setCheckState(static_cast<DImagesListView::ColumnType>(FlickrList::PUBLIC), accessPublic ? Qt::Checked : Qt::Unchecked);

    // Set the tooltips to guide the user to the mass settings options.
    setToolTip(static_cast<DImagesListView::ColumnType>(FlickrList::PUBLIC),
               i18n("Check if photo should be publicly visible or use Upload "
                    "Options tab to specify this for all images"));
    setToolTip(static_cast<DImagesListView::ColumnType>(FlickrList::FAMILY),
               i18n("Check if photo should be visible to family or use Upload "
                    "Options tab to specify this for all images"));
    setToolTip(static_cast<DImagesListView::ColumnType>(FlickrList::FRIENDS),
               i18n("Check if photo should be visible to friends or use "
                    "Upload Options tab to specify this for all images"));
    setToolTip(static_cast<DImagesListView::ColumnType>(FlickrList::SAFETYLEVEL),
               i18n("Indicate the safety level for the photo or use Upload "
                    "Options tab to specify this for all images"));
    setToolTip(static_cast<DImagesListView::ColumnType>(FlickrList::CONTENTTYPE),
               i18n("Indicate what kind of image this is or use Upload "
                    "Options tab to specify this for all images"));

    // Set the other checkboxes.
    setFamily(accessFamily);
    setFriends(accessFriends);
    setPublic(accessPublic);
    setSafetyLevel(safetyLevel);
    setContentType(contentType);

    // Extra per image tags handling.
    setToolTip(static_cast<DImagesListView::ColumnType>(
                   FlickrList::TAGS),
               i18n("Add extra tags per image or use Upload Options tab to "
                    "add tags for all images"));
    //m_tagLineEdit = new QLineEdit(view);
    //m_tagLineEdit->setToolTip(i18n("Enter extra tags, separated by commas."));
    //view->setItemWidget(this, static_cast<DImagesListView::ColumnType>(
    //                    FlickrList::TAGS), m_tagLineEdit);
    updateItemWidgets();
}

FlickrListViewItem::~FlickrListViewItem()
{

}

void FlickrListViewItem::updateItemWidgets()
{
    m_tagLineEdit = new QLineEdit(view());
    m_tagLineEdit->setToolTip(i18n("Enter extra tags, separated by commas."));
    view()->setItemWidget(this, static_cast<DImagesListView::ColumnType>(
                          FlickrList::TAGS), m_tagLineEdit);
}

QStringList FlickrListViewItem::extraTags() const
{
    return m_tagLineEdit->text().split(QLatin1Char(','), QString::SkipEmptyParts);
}

void FlickrListViewItem::toggled()
{
    // The m_family and m_friends states should be set first, so that the
    // setPublic method has the proper values to work with.
    if (!m_is23)
    {
        if (data(FlickrList::FAMILY, Qt::CheckStateRole) != QVariant())
        {
            setFamily(checkState(static_cast<DImagesListView::ColumnType>(FlickrList::FAMILY)));
        }

        if (data(FlickrList::FRIENDS, Qt::CheckStateRole) != QVariant())
        {
            setFriends(checkState(static_cast<DImagesListView::ColumnType>(FlickrList::FRIENDS)));
        }
    }

    setPublic(checkState(static_cast<DImagesListView::ColumnType>(FlickrList::PUBLIC)));
}

void FlickrListViewItem::setPublic(bool status)
{
    /* Set the public status of the entry. If public is true, hide the
     * family and friends checkboxes, otherwise, make them appear. */

    // Set the status.
    m_public = status;

    // Toggle the family and friends checkboxes, if applicable.
    if (!m_is23)
    {
        if (m_public)
        {
            // Hide the checkboxes by feeding them a bogus QVariant for the
            // CheckStateRole. This might seem like a hack, but it's described in
            // the Qt FAQ at
            // http://www.qtsoftware.com/developer/faqs/faq.2007-04-23.8353273326.
            setData(static_cast<DImagesListView::ColumnType>(FlickrList::FAMILY),  Qt::CheckStateRole, QVariant());
            setData(static_cast<DImagesListView::ColumnType>(FlickrList::FRIENDS), Qt::CheckStateRole, QVariant());
        }
        else
        {
            // Show the checkboxes.
            setCheckState(static_cast<DImagesListView::ColumnType>(FlickrList::FAMILY),  m_family  ? Qt::Checked : Qt::Unchecked);
            setCheckState(static_cast<DImagesListView::ColumnType>(FlickrList::FRIENDS), m_friends ? Qt::Checked : Qt::Unchecked);
        }
    }

    // Toggle the public checkboxes
    if (m_public)
    {
        setCheckState(FlickrList::PUBLIC, Qt::Checked);
    }
    else
    {
        setCheckState(FlickrList::PUBLIC, Qt::Unchecked);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Public status set to" << m_public;
}

void FlickrListViewItem::setFamily(bool status)
{
    /* Set the family status. */
    m_family = status;

    if ((!m_is23) && (data(FlickrList::FAMILY, Qt::CheckStateRole) != QVariant()))
    {
        setCheckState(FlickrList::FAMILY, m_family ? Qt::Checked : Qt::Unchecked);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Family status set to" << m_family;
}

void FlickrListViewItem::setFriends(bool status)
{
    /* Set the family status. */
    m_friends = status;

    if ((!m_is23) && (data(FlickrList::FRIENDS, Qt::CheckStateRole) != QVariant()))
    {
        setCheckState(FlickrList::FRIENDS, m_friends ? Qt::Checked : Qt::Unchecked);
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Friends status set to" << m_friends;
}

void FlickrListViewItem::setSafetyLevel(FlickrList::SafetyLevel safetyLevel)
{
    m_safetyLevel = safetyLevel;
    setData(FlickrList::SAFETYLEVEL, Qt::DisplayRole, QVariant(safetyLevel));
    qCDebug(DIGIKAM_GENERAL_LOG) << "Safety level set to" << safetyLevel;
}

void FlickrListViewItem::setContentType(FlickrList::ContentType contentType)
{
    m_contentType = contentType;
    setData(FlickrList::CONTENTTYPE, Qt::DisplayRole, QVariant(contentType));
    qCDebug(DIGIKAM_GENERAL_LOG) << "Content type set to" << contentType;
}

bool FlickrListViewItem::isPublic() const
{
    /* Return whether the photo is public. */
    return m_public;
}

bool FlickrListViewItem::isFamily() const
{
    /* Return whether the photo is accessible for family. */
    return m_family;
}

bool FlickrListViewItem::isFriends() const
{
    /* Return whether the photo is accessible for friends. */
    return m_friends;
}

FlickrList::SafetyLevel FlickrListViewItem::safetyLevel() const
{
    return m_safetyLevel;
}

FlickrList::ContentType FlickrListViewItem::contentType() const
{
    return m_contentType;
}

} // namespace Digikam
