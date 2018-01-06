/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-15
 * Description : collections setup tab model/view
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPCOLLECTIONVIEW_H
#define SETUPCOLLECTIONVIEW_H

// Qt includes

#include <QAbstractItemModel>
#include <QAbstractItemDelegate>
#include <QList>
#include <QTreeView>
#include <QStyledItemDelegate>
#include <QSignalMapper>
#include <QPushButton>
#include <QToolButton>

// Local includes

#include "collectionlocation.h"
#include "dwitemdelegate.h"

namespace Digikam
{

class SetupCollectionModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    /** SetupCollectionModel is a model specialized for use in
     *  SetupCollectionTreeView. It provides a reads the current collections
     *  from CollectionManager, displays them in three categories,
     *  and supports adding and removing collections
     */

    enum SetupCollectionDataRole
    {
        /// Returns true if the model index is the index of a category
        IsCategoryRole             = Qt::UserRole,
        /// The text for the category button
        CategoryButtonDisplayRole  = Qt::UserRole + 1,
        CategoryButtonMapId        = Qt::UserRole + 2,
        /// Returns true if the model index is the index of a button
        IsButtonRole               = Qt::UserRole + 3,
        /// The pixmap of the button
        ButtonDecorationRole       = Qt::UserRole + 4,
        ButtonMapId                = Qt::UserRole + 5
    };

    enum Columns
    {
        ColumnStatus       = 0,
        ColumnName         = 1,
        ColumnPath         = 2,
        ColumnDeleteButton = 3,
        NumberOfColumns
    };

    enum Category
    {
        CategoryLocal      = 0,
        CategoryRemovable  = 1,
        CategoryRemote     = 2,
        NumberOfCategories
    };

public:

    explicit SetupCollectionModel(QObject* const parent = 0);
    ~SetupCollectionModel();

    /// Read collections from CollectionManager
    void loadCollections();

    /// Set a widget used as parent for dialogs and message boxes
    void setParentWidgetForDialogs(QWidget* const widget);

    /// Apply the changed settings to CollectionManager
    void apply();

    QModelIndex indexForCategory(Category category) const;
    QList<QModelIndex> categoryIndexes()            const;

    // QAbstractItemModel implementation
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;

/*
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    virtual QMimeData * mimeData(const QModelIndexList& indexes) const;
*/

Q_SIGNALS:

    /// Emitted when all collections were loaded and the model reset in loadCollections
    void collectionsLoaded();

public Q_SLOTS:

    /** Forward category button clicked signals to this slot.
     *  mappedId is retrieved with the CategoryButtonMapId role
     *  for the model index of the button
     */
    void slotCategoryButtonPressed(int mappedId);

    /** Forward button clicked signals to this slot.
     *  mappedId is retrieved with the ButtonMapId role
     *  for the model index of the button
     */
    void slotButtonPressed(int mappedId);

protected Q_SLOTS:

    void addCollection(int category);
    void deleteCollection(int internalId);

protected:

    QModelIndex indexForId(int id, int column) const;

    int categoryButtonMapId(const QModelIndex& index) const;
    int buttonMapId(const QModelIndex& index) const;

    static Category typeToCategory(CollectionLocation::Type type);

protected:

    class Item
    {
    public:

        Item();
        explicit Item(const CollectionLocation& location);
        Item(const QString& path, const QString& label, SetupCollectionModel::Category category);

        CollectionLocation location;
        QString            label;
        QString            path;
        int                parentId;
        bool               deleted;
    };

protected:

    QList<Item> m_collections;
    QWidget*    m_dialogParentWidget;
};

// -----------------------------------------------------------------------

class SetupCollectionTreeView : public QTreeView
{
    Q_OBJECT

public:

    explicit SetupCollectionTreeView(QWidget* const parent = 0);

    void setModel(SetupCollectionModel* model);

protected Q_SLOTS:

    void modelLoadedCollections();

private:

    void setModel(QAbstractItemModel* model)
    {
        setModel(static_cast<SetupCollectionModel*>(model));
    }
};

// -----------------------------------------------------------------------

class SetupCollectionDelegate : public DWItemDelegate
{
    Q_OBJECT

public:

    SetupCollectionDelegate(QAbstractItemView* const view, QObject* const parent = 0);
    ~SetupCollectionDelegate();

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual bool     editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
    virtual void     paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual void     setEditorData(QWidget* editor, const QModelIndex& index) const;
    virtual void     setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    virtual QSize    sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual void     updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    virtual QList<QWidget*> createItemWidgets(const QModelIndex& index) const;
    virtual void            updateItemWidgets(const QList<QWidget*> widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const;

Q_SIGNALS:

    void categoryButtonPressed(int mappedId);
    void buttonPressed(int mappedId);

protected:

    QStyledItemDelegate* m_styledDelegate;

    QPushButton*         m_samplePushButton;
    QToolButton*         m_sampleToolButton;
    int                  m_categoryMaxStyledWidth;

    QSignalMapper*       m_categoryButtonMapper;
    QSignalMapper*       m_buttonMapper;
};

} // namespace Digikam

#endif /* SETUPCOLLECTIONVIEW_H */
