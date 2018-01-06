/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-16
 * Description : metadata selector.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATA_SELECTOR_H
#define METADATA_SELECTOR_H

// Qt includes

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "searchtextbar.h"
#include "dmetadata.h"

namespace Digikam
{

class MdKeyListViewItem;

class DIGIKAM_EXPORT MetadataSelectorItem : public QTreeWidgetItem
{

public:

    MetadataSelectorItem(MdKeyListViewItem* const parent, const QString& key, const QString& title, const QString& desc);
    virtual ~MetadataSelectorItem();

    QString key() const;
    QString mdKeyTitle() const;

private:

    QString            m_key;
    MdKeyListViewItem* m_parent;
};

// ------------------------------------------------------------------------------------

class DIGIKAM_EXPORT MetadataSelector : public QTreeWidget
{

public:

    explicit MetadataSelector(QWidget* const parent);
    virtual ~MetadataSelector();

    void setTagsMap(const DMetadata::TagsMap& map);

    void setcheckedTagsList(const QStringList& list);
    QStringList checkedTagsList();

    void clearSelection();
    void selectAll();
};

// ------------------------------------------------------------------------------------

class DIGIKAM_EXPORT MetadataSelectorView : public QWidget
{
    Q_OBJECT

public:

    enum ControlElement
    {
        SelectAllBtn = 0x01,
        ClearBtn     = 0x02,
        DefaultBtn   = 0x04,
        SearchBar    = 0x08
    };
    Q_DECLARE_FLAGS(ControlElements, ControlElement)

    explicit MetadataSelectorView(QWidget* const parent);
    virtual ~MetadataSelectorView();

    int itemsCount() const;

    void setTagsMap(const DMetadata::TagsMap& map);

    void setcheckedTagsList(const QStringList& list);

    void setDefaultFilter(const QStringList& list);
    QStringList defaultFilter() const;

    QStringList checkedTagsList() const;

    void setControlElements(ControlElements controllerMask);

    void clearSelection();
    void selectAll();
    void selectDefault();

private Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings&);
    void slotDeflautSelection();
    void slotSelectAll();
    void slotClearSelection();

private:

    void cleanUpMdKeyItem();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::MetadataSelectorView::ControlElements)

#endif // METADATA_SELECTOR_H
