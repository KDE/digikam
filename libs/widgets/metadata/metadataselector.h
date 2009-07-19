/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-16
 * Description : metadata selector.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Libkexiv2 includes

#include <libkexiv2/version.h>

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

    MetadataSelectorItem(MdKeyListViewItem* parent, const QString& key, const QString& title, const QString& desc);
    virtual ~MetadataSelectorItem();

    QString key() const;
    QString mdKeyTitle() const;

private:

    QString            m_key;
    MdKeyListViewItem *m_parent;
};

// ------------------------------------------------------------------------------------

class DIGIKAM_EXPORT MetadataSelector : public QTreeWidget
{

public:

    MetadataSelector(QWidget* parent);
    virtual ~MetadataSelector();

#if KEXIV2_VERSION >= 0x010000
    void setTagsMap(const DMetadata::TagsMap& map);
#endif

    void setcheckedTagsList(const QStringList& list);
    QStringList checkedTagsList();

    void clearSelection();
    void selectAll();
};

// ------------------------------------------------------------------------------------

class MetadataSelectorViewPriv;

class DIGIKAM_EXPORT MetadataSelectorView : public QWidget
{
    Q_OBJECT

public:

    MetadataSelectorView(QWidget* parent);
    virtual ~MetadataSelectorView();

    int itemsCount() const;

#if KEXIV2_VERSION >= 0x010000
    void setTagsMap(const DMetadata::TagsMap& map);
#endif

    void setcheckedTagsList(const QStringList& list);

    void setDefaultFilter(const char** list);
    QStringList defaultFilter() const;

    QStringList checkedTagsList() const;

private Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings&);
    void slotDeflautSelection();
    void slotSelectAll();
    void slotClearSelection();

private:

    MetadataSelectorViewPriv* const d;
};

}  // namespace Digikam

#endif /* METADATA_SELECTOR_H */
