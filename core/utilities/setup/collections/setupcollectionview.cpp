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
 * Copyright (C)      2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#define INTERNALID 65535

#include "setupcollectionview.h"

// Qt includes

#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStandardPaths>
#include <QLineEdit>
#include <QUrl>
#include <QIcon>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmessagebox.h"
#include "dfiledialog.h"
#include "applicationsettings.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "newitemsfinder.h"

namespace Digikam
{

SetupCollectionDelegate::SetupCollectionDelegate(QAbstractItemView* const view, QObject* const parent)
    : DWItemDelegate(view, parent),
      m_categoryMaxStyledWidth(0)
{
    // We keep a standard delegate that does all the normal drawing work for us
    // DWItemDelegate handles the widgets, for the rest of the work we act as a proxy to m_styledDelegate
    m_styledDelegate = new QStyledItemDelegate(parent);

    // forward all signals
    connect(m_styledDelegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
            this, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));

    connect(m_styledDelegate, SIGNAL(commitData(QWidget*)),
            this, SIGNAL(commitData(QWidget*)));

    connect(m_styledDelegate, SIGNAL(sizeHintChanged(QModelIndex)),
            this, SIGNAL(sizeHintChanged(QModelIndex)));

    // For size hint. To get a valid size hint, the widgets seem to need a parent widget
    m_samplePushButton = new QPushButton(view);
    m_samplePushButton->hide();
    m_sampleToolButton = new QToolButton(view);
    m_sampleToolButton->hide();

    // Implement mapping of signals. Every button gets a mapping ID from the model
    m_categoryButtonMapper = new QSignalMapper(this);
    m_buttonMapper         = new QSignalMapper(this);

    connect(m_categoryButtonMapper, SIGNAL(mapped(int)),
            this, SIGNAL(categoryButtonPressed(int)));

    connect(m_buttonMapper, SIGNAL(mapped(int)),
            this, SIGNAL(buttonPressed(int)));
}

SetupCollectionDelegate::~SetupCollectionDelegate()
{
}

QList<QWidget*> SetupCollectionDelegate::createItemWidgets(const QModelIndex& /*index*/) const
{
    // We only need a push button for certain indexes and a tool button for others,
    // but we have no index here, but need to provide the widgets for each index

    QList<QWidget*> list;
    QPushButton* const pushButton = new QPushButton();
    list << pushButton;

    connect(pushButton, SIGNAL(clicked()),
            m_categoryButtonMapper, SLOT(map()));

    QToolButton* const toolButton = new QToolButton();
    toolButton->setAutoRaise(true);
    list << toolButton;

    connect(toolButton, SIGNAL(clicked()),
            m_buttonMapper, SLOT(map()));

    return list;
}

QSize SetupCollectionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // get default size hint
    QSize hint = m_styledDelegate->sizeHint(option, index);

    // We need to handle those two cases where we display widgets
    if (index.data(SetupCollectionModel::IsCategoryRole).toBool())
    {
        // get the largest size hint for the icon/text of all category entries
        int maxStyledWidth = 0;

        foreach(const QModelIndex& catIndex, static_cast<const SetupCollectionModel*>(index.model())->categoryIndexes())
        {
            maxStyledWidth = qMax(maxStyledWidth, m_styledDelegate->sizeHint(option, catIndex).width());
        }

        const_cast<SetupCollectionDelegate*>(this)->m_categoryMaxStyledWidth = maxStyledWidth;

        // set real text on sample button to compute correct size hint
        m_samplePushButton->setText(index.data(SetupCollectionModel::CategoryButtonDisplayRole).toString());
        QSize widgetHint = m_samplePushButton->sizeHint();

        // add largest of the icon/text sizes (so that all buttons are aligned) and our button size hint
        hint.setWidth(m_categoryMaxStyledWidth + widgetHint.width());
        hint.setHeight(qMax(hint.height(), widgetHint.height()));
    }
    else if (index.data(SetupCollectionModel::IsButtonRole).toBool())
    {
        // set real pixmap on sample button to compute correct size hint
        QIcon pix      = index.data(SetupCollectionModel::ButtonDecorationRole).value<QIcon>();
        m_sampleToolButton->setIcon(index.data(SetupCollectionModel::ButtonDecorationRole).value<QIcon>());
        QSize widgetHint = m_sampleToolButton->sizeHint();

        // combine hints
        hint.setWidth(hint.width() + widgetHint.width());
        hint.setHeight(qMax(hint.height(), widgetHint.height()));
    }

    return hint;
}

void SetupCollectionDelegate::updateItemWidgets(const QList<QWidget*> widgets,
                                                const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const
{
    QPushButton* const pushButton = static_cast<QPushButton*>(widgets.at(0));
    QToolButton* const toolButton = static_cast<QToolButton*>(widgets.at(1));

    if (index.data(SetupCollectionModel::IsCategoryRole).toBool())
    {
        // set text from model
        pushButton->setText(index.data(SetupCollectionModel::CategoryButtonDisplayRole).toString());
        // resize according to size hint
        pushButton->resize(pushButton->sizeHint());
        // move to position in line. We have cached the icon/text size hint from sizeHint()
        pushButton->move(m_categoryMaxStyledWidth, (option.rect.height() - pushButton->height()) / 2);
        pushButton->show();
        toolButton->hide();

        pushButton->setEnabled(itemView()->isEnabled());

        // get the mapping id from model. The signal mapper will associate the id with the signal from this button.
        m_categoryButtonMapper->setMapping(pushButton, index.data(SetupCollectionModel::CategoryButtonMapId).toInt());
    }
    else if (index.data(SetupCollectionModel::IsButtonRole).toBool())
    {
        toolButton->setIcon(index.data(SetupCollectionModel::ButtonDecorationRole).value<QIcon>());
        toolButton->resize(toolButton->sizeHint());
        toolButton->move(0, (option.rect.height() - toolButton->height()) / 2);
        toolButton->show();
        pushButton->hide();

        toolButton->setEnabled(itemView()->isEnabled());

        m_buttonMapper->setMapping(toolButton, index.data(SetupCollectionModel::ButtonMapId).toInt());
    }
    else
    {
        pushButton->hide();
        toolButton->hide();
    }
}

QWidget* SetupCollectionDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return m_styledDelegate->createEditor(parent, option, index);
}

bool SetupCollectionDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                          const QStyleOptionViewItem& option, const QModelIndex& index)
{
    return static_cast<QAbstractItemDelegate*>(m_styledDelegate)->editorEvent(event, model, option, index);
}

void SetupCollectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    m_styledDelegate->paint(painter, option, index);
}

void SetupCollectionDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    m_styledDelegate->setEditorData(editor, index);
}

void SetupCollectionDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    m_styledDelegate->setModelData(editor, model, index);
}

void SetupCollectionDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    m_styledDelegate->updateEditorGeometry(editor, option, index);
}

// ------------- View ----------------- //

SetupCollectionTreeView::SetupCollectionTreeView(QWidget* const parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setExpandsOnDoubleClick(false);
    setHeaderHidden(true);

    // Set custom delegate
    setItemDelegate(new SetupCollectionDelegate(this, this));
}

void SetupCollectionTreeView::setModel(SetupCollectionModel* collectionModel)
{
    if (model())
    {
        disconnect(model(), 0, this, 0);
    }

    // we need to do some things after the model has loaded its data
    connect(collectionModel, SIGNAL(collectionsLoaded()),
            this, SLOT(modelLoadedCollections()));

    // connect button click signals from the delegate to the model
    connect(static_cast<SetupCollectionDelegate*>(itemDelegate()), SIGNAL(categoryButtonPressed(int)),
            collectionModel, SLOT(slotCategoryButtonPressed(int)));

    connect(static_cast<SetupCollectionDelegate*>(itemDelegate()), SIGNAL(buttonPressed(int)),
            collectionModel, SLOT(slotButtonPressed(int)));

    // give model a widget to use as parent for message boxes
    collectionModel->setParentWidgetForDialogs(this);

    QTreeView::setModel(collectionModel);
}

void SetupCollectionTreeView::modelLoadedCollections()
{
    // make category entries span the whole line
    for (int i = 0; i < model()->rowCount(QModelIndex()); ++i)
    {
        setFirstColumnSpanned(i, QModelIndex(), true);
    }

    // show all entries
    expandAll();

    // Resize name and path column
    header()->setSectionResizeMode(SetupCollectionModel::ColumnName, QHeaderView::Stretch);
    header()->setSectionResizeMode(SetupCollectionModel::ColumnPath, QHeaderView::Stretch);

    // Resize last column, so that delete button is always rightbound
    header()->setStretchLastSection(false); // defaults to true
    header()->setSectionResizeMode(SetupCollectionModel::ColumnDeleteButton, QHeaderView::Fixed);
    resizeColumnToContents(SetupCollectionModel::ColumnDeleteButton);

    // Resize first column
    // This is more difficult because we need to ignore the width of the category entries,
    // which are formally location in the first column (although spanning the whole line).
    // resizeColumnToContents fails therefore.
    SetupCollectionModel* const collectionModel = static_cast<SetupCollectionModel*>(model());
    QModelIndex categoryIndex = collectionModel->indexForCategory(SetupCollectionModel::CategoryLocal);
    QModelIndex firstChildOfFirstCategory       = collectionModel->index(0, SetupCollectionModel::ColumnStatus, categoryIndex);
    QSize hint = sizeHintForIndex(firstChildOfFirstCategory);
    setColumnWidth(SetupCollectionModel::ColumnStatus, hint.width() + indentation());
}

// ------------- Model ----------------- //

SetupCollectionModel::Item::Item()
    : parentId(INTERNALID),
      deleted(false)
{
}

SetupCollectionModel::Item::Item(const CollectionLocation& location)
    : location(location),
      deleted(false)
{
    parentId = SetupCollectionModel::typeToCategory(location.type());
}

SetupCollectionModel::Item::Item(const QString& path, const QString& label, SetupCollectionModel::Category category)
    : label(label),
      path(path),
      parentId(category),
      deleted(false)
{
}

/*
    Internal data structure:

    The category entries get a model index with INTERNALID and are identified by their row().
    The item entries get the index in m_collections as INTERNALID.
    No item is ever removed from m_collections, deleted entries are only marked as such.

    Items have a location, a parentId, and a name and label field.
    parentId always contains the category, needed to implement parent().
    The location is the location if it exists, or is null if the item was added.
    Name and label are null if unchanged, then the values from location are used.
    They are valid if edited (label) or the location was added (both valid, location null).
*/

SetupCollectionModel::SetupCollectionModel(QObject* const parent)
    : QAbstractItemModel(parent),
      m_dialogParentWidget(0)
{
}

SetupCollectionModel::~SetupCollectionModel()
{
}

void SetupCollectionModel::loadCollections()
{
    m_collections.clear();
    QList<CollectionLocation> locations = CollectionManager::instance()->allLocations();

    foreach(const CollectionLocation& location, locations)
    {
        m_collections << Item(location);
    }

    beginResetModel();
    endResetModel();
    emit collectionsLoaded();
}

void SetupCollectionModel::apply()
{
    QList<int> newItems, deletedItems, renamedItems;

    for (int i = 0; i < m_collections.count(); ++i)
    {
        const Item& item = m_collections.at(i);

        if (item.deleted && !item.location.isNull())
            // if item was deleted and had a valid location, i.e. exists in DB
        {
            deletedItems << i;
        }
        else if (!item.deleted && item.location.isNull())
            // if item has no valid location, i.e. does not yet exist in db
        {
            newItems << i;
        }
        else if (!item.deleted && !item.location.isNull())
        {
            // if item has a valid location, is not deleted, and has changed its label
            if (!item.label.isNull() && item.label != item.location.label())
            {
                renamedItems << i;
            }
        }
    }

    // Delete deleted items
    foreach(int i, deletedItems)
    {
        Item& item    = m_collections[i];
        CollectionManager::instance()->removeLocation(item.location);
        item.location = CollectionLocation();
    }

    // Add added items
    QList<Item> failedItems;

    foreach(int i, newItems)
    {
        Item& item = m_collections[i];
        CollectionLocation location;

        if (item.parentId == CategoryRemote)
        {
            location = CollectionManager::instance()->addNetworkLocation(QUrl::fromLocalFile(item.path), item.label);
        }
        else
        {
            location = CollectionManager::instance()->addLocation(QUrl::fromLocalFile(item.path), item.label);
        }

        if (location.isNull())
        {
            failedItems << item;
        }
        else
        {
            item.location = location;
            item.path.clear();
            item.label.clear();
        }
    }

    // Rename collections
    foreach(int i, renamedItems)
    {
        Item& item = m_collections[i];
        CollectionManager::instance()->setLabel(item.location, item.label);
        item.label.clear();
    }

    // Handle any errors
    if (!failedItems.isEmpty())
    {
        QStringList failedPaths;

        foreach(const Item& item, failedItems)
        {
            failedPaths << QDir::toNativeSeparators(item.path);
        }

        DMessageBox::showInformationList(QMessageBox::Critical,
                                         m_dialogParentWidget,
                                         qApp->applicationName(),
                                         i18n("It was not possible to add a collection for the following paths:"),
                                         failedPaths);
    }

    // Trigger collection scan
    if (!newItems.isEmpty() || !deletedItems.isEmpty())
    {
        NewItemsFinder* const tool = new NewItemsFinder();
        tool->start();
    }
}

void SetupCollectionModel::setParentWidgetForDialogs(QWidget* widget)
{
    m_dialogParentWidget = widget;
}

void SetupCollectionModel::slotCategoryButtonPressed(int mappedId)
{
    addCollection(mappedId);
}

void SetupCollectionModel::slotButtonPressed(int mappedId)
{
    deleteCollection(mappedId);
}

void SetupCollectionModel::addCollection(int category)
{
    if (category < 0 || category >= NumberOfCategories)
    {
        return;
    }

    QString picturesPath;

    if (m_collections.count() > 0)
    {
        const Item& item = m_collections[0];
        picturesPath     = item.path;
    }
    else
    {
        picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }

    QUrl curl = DFileDialog::getExistingDirectoryUrl(m_dialogParentWidget,
                                                     i18n("Choose the folder containing your collection"),
                                                     QUrl::fromLocalFile(picturesPath));

    if (curl.isEmpty())
    {
        return;
    }

    // Check path: First check with manager
    QString messageFromManager, deviceIcon;
    QList<CollectionLocation> assumeDeleted;

    foreach(const Item& item, m_collections)
    {
        if (item.deleted && !item.location.isNull())
        {
            assumeDeleted << item.location;
        }
    }

    CollectionManager::LocationCheckResult result;

    if (category == CategoryRemote)
        result = CollectionManager::instance()->checkNetworkLocation(curl, assumeDeleted,
                                                                     &messageFromManager, &deviceIcon);
    else
        result = CollectionManager::instance()->checkLocation(curl, assumeDeleted,
                                                              &messageFromManager, &deviceIcon);

    QString path = QDir::fromNativeSeparators(curl.toDisplayString(QUrl::PreferLocalFile ));

    // If there are other added collections then CollectionManager does not know about them. Check here.
    foreach(const Item& item, m_collections)
    {
        if (!item.deleted && item.location.isNull())
        {
            if (!item.path.isEmpty() && path.startsWith(item.path))
            {
                if (path == item.path || path.startsWith(item.path + QLatin1Char('/')))
                {
                    messageFromManager = i18n("You have previously added a collection "
                                              "that contains the path \"%1\".", QDir::toNativeSeparators(path));
                    result = CollectionManager::LocationNotAllowed;
                    break;
                }
            }
        }
    }

    // If check failed, display sorry message
    QString iconName;

    switch (result)
    {
        case CollectionManager::LocationAllRight:
            iconName = QLatin1String("dialog-ok-apply");
            break;

        case CollectionManager::LocationHasProblems:
            iconName = QLatin1String("dialog-information");
            break;

        case CollectionManager::LocationNotAllowed:
        case CollectionManager::LocationInvalidCheck:
            QMessageBox::warning(m_dialogParentWidget, i18n("Problem Adding Collection"), messageFromManager);
            // fail
            return;
    }

    // Create a dialog that displays volume information and allows to change the name of the collection
    QDialog* const dialog = new QDialog(m_dialogParentWidget);
    dialog->setWindowTitle(i18n("Adding Collection"));

    QWidget* const mainWidget = new QWidget(dialog);
    QLabel* const nameLabel   = new QLabel;
    nameLabel->setText(i18n("Your new collection will be created with this name:"));
    nameLabel->setWordWrap(true);

    // lineedit for collection name
    QLineEdit* const nameEdit = new QLineEdit;
    nameEdit->setClearButtonEnabled(true);
    nameLabel->setBuddy(nameEdit);

    // label for the icon showing the type of storage (hard disk, CD, USB drive)
    QLabel* const deviceIconLabel = new QLabel;
    deviceIconLabel->setPixmap(QIcon::fromTheme(deviceIcon).pixmap(64));

    QGroupBox* const infoBox = new QGroupBox;
    //infoBox->setTitle(i18n("More Information"));

    // label either signalling everything is all right, or raising awareness to some problems
    // (like handling of CD identified by a label)
    QLabel* const iconLabel = new QLabel;
    iconLabel->setPixmap(QIcon::fromTheme(iconName).pixmap(48));
    QLabel* const infoLabel = new QLabel;
    infoLabel->setText(messageFromManager);
    infoLabel->setWordWrap(true);

    QHBoxLayout* const hbox1 = new QHBoxLayout;
    hbox1->addWidget(iconLabel);
    hbox1->addWidget(infoLabel);
    infoBox->setLayout(hbox1);

    QGridLayout* const grid1 = new QGridLayout;
    grid1->addWidget(deviceIconLabel, 0, 0, 3, 1);
    grid1->addWidget(nameLabel,       0, 1);
    grid1->addWidget(nameEdit,        1, 1);
    grid1->addWidget(infoBox,         2, 1);
    mainWidget->setLayout(grid1);

    QVBoxLayout* const vbx          = new QVBoxLayout(dialog);
    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    vbx->addWidget(mainWidget);
    vbx->addWidget(buttons);
    dialog->setLayout(vbx);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            dialog, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            dialog, SLOT(reject()));

    // default to directory name as collection name
    QDir dir(path);
    nameEdit->setText(dir.dirName());

    if (dialog->exec() == QDialog::Accepted)
    {
        // Add new item to model. Adding to CollectionManager is done in apply()!
        QModelIndex parent = indexForCategory((Category)category);
        int row            = rowCount(parent);
        QString label      = nameEdit->text();

        if (label.isEmpty())
        {
            label.clear();
        }

        beginInsertRows(parent, row, row);
        m_collections << Item(path, label, (Category)category);
        endInsertRows();

        // only workaround for bug 182753
        emit layoutChanged();
    }
}

/*
//This code works, but is currently not used. Was intended as a workaround for 219876.
void SetupCollectionModel::emitDataChangedForChildren(const QModelIndex& parent)
{
    int rows    = rowCount(parent);
    int columns = columnCount(parent);
    emit dataChanged(index(0, 0, parent), index(rows, columns, parent));

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < columns; ++c)
        {
            QModelIndex i = index(r, c, parent);

            if (i.isValid())
                emitDataChangedForChildren(i);
        }
    }
}
*/

void SetupCollectionModel::deleteCollection(int internalId)
{
    QModelIndex index       = indexForId(internalId, (int)ColumnStatus);
    QModelIndex parentIndex = parent(index);

    if (!index.isValid() || internalId >= m_collections.count())
    {
        return;
    }

    Item& item = m_collections[index.internalId()];

    // Ask for confirmation
    QString label = data(indexForId(internalId, (int)ColumnName), Qt::DisplayRole).toString();
    int result    = QMessageBox::warning(m_dialogParentWidget,
                                         i18n("Remove Collection?"),
                                         i18n("Do you want to remove the collection \"%1\" from your list of collections?", label),
                                         QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes)
    {
        // Remove from model. Removing from CollectionManager is done in apply()!
        beginRemoveRows(parentIndex, index.row(), index.row());
        item.deleted = true;
        endRemoveRows();

        // only workaround for bug 182753
        emit layoutChanged();
    }
}

QVariant SetupCollectionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.internalId() == INTERNALID)
    {
        if (index.column() == 0)
        {
            switch (role)
            {
                case Qt::DisplayRole:

                    switch (index.row())
                    {
                        case CategoryLocal:
                            return i18n("Local Collections");

                        case CategoryRemovable:
                            return i18n("Collections on Removable Media");

                        case CategoryRemote:
                            return i18n("Collections on Network Shares");
                    }

                    break;

                case Qt::DecorationRole:

                    switch (index.row())
                    {
                        case CategoryLocal:
                            return QIcon::fromTheme(QLatin1String("drive-harddisk"));

                        case CategoryRemovable:
                            return QIcon::fromTheme(QLatin1String("drive-removable-media"));

                        case CategoryRemote:
                            return QIcon::fromTheme(QLatin1String("network-wired-activated"));
                    }

                    break;

                case IsCategoryRole:
                    return true;

                case CategoryButtonDisplayRole:
                    return i18n("Add Collection");

                case CategoryButtonMapId:
                    return categoryButtonMapId(index);

                default:
                    break;
            }
        }
    }
    else
    {
        const Item& item = m_collections.at(index.internalId());

        switch (index.column())
        {
            case ColumnName:

                if (role == Qt::DisplayRole || role == Qt::EditRole)
                {
                    if (!item.label.isNull())
                    {
                        return item.label;
                    }

                    if (!item.location.label().isNull())
                    {
                        return item.location.label();
                    }

                    return i18n("Col. %1", index.row());
                }

                break;

            case ColumnPath:

                if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
                {
                    if (!item.path.isNull())
                    {
                        return QDir::toNativeSeparators(item.path);
                    }

                    //TODO: Path can be empty for items not available,
                    // query more info from CollectionManager
                    return QDir::toNativeSeparators(item.location.albumRootPath());
                }

                break;

            case ColumnStatus:

                if (role == Qt::DecorationRole)
                {
                    if (item.deleted)
                    {
                        return QIcon::fromTheme(QLatin1String("edit-delete"));
                    }

                    if (item.location.isNull())
                    {
                        return QIcon::fromTheme(QLatin1String("folder-new"));
                    }

                    switch (item.location.status())
                    {
                        case CollectionLocation::LocationAvailable:
                            return QIcon::fromTheme(QLatin1String("dialog-ok-apply"));

                        case CollectionLocation::LocationHidden:
                            return QIcon::fromTheme(QLatin1String("object-locked"));

                        case CollectionLocation::LocationUnavailable:

                            switch (item.parentId)
                            {
                                case CategoryLocal:
                                    return QIcon::fromTheme(QLatin1String("drive-harddisk")).pixmap(16, QIcon::Disabled);

                                case CategoryRemovable:
                                    return QIcon::fromTheme(QLatin1String("drive-removable-media-usb")).pixmap(16, QIcon::Disabled);

                                case CategoryRemote:
                                    return QIcon::fromTheme(QLatin1String("network-wired-activated")).pixmap(16, QIcon::Disabled);
                            }

                        case CollectionLocation::LocationNull:
                        case CollectionLocation::LocationDeleted:
                            return QIcon::fromTheme(QLatin1String("edit-delete"));
                    }
                }
                else if (role == Qt::ToolTipRole)
                {
                    switch (item.location.status())
                    {
                        case CollectionLocation::LocationUnavailable:
                            return i18n("This collection is currently not available.");

                        case CollectionLocation::LocationAvailable:
                            return i18n("No problems found, enjoy this collection.");

                        case CollectionLocation::LocationHidden:
                            return i18n("This collection is hidden.");

                        default:
                            break;
                    }
                }

                break;

            case ColumnDeleteButton:

                switch (role)
                {
                    case Qt::ToolTipRole:
                        return i18n("Remove collection");

                    case IsButtonRole:
                        return true;

                    case ButtonDecorationRole:
                        return QIcon::fromTheme(QLatin1String("edit-delete"));

                    case ButtonMapId:
                        return buttonMapId(index);
                }

                break;
        }
    }

    return QVariant();
}

QVariant SetupCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal && section < NumberOfColumns)
    {
        switch (section)
        {
            case ColumnName:
                return i18n("Name");

            case ColumnPath:
                return i18n("Path");

            case ColumnStatus:
                return i18n("Status");

            case ColumnDeleteButton:
                break;
        }
    }

    return QVariant();
}

int SetupCollectionModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return NumberOfCategories;    // Level 0: the three top level items
    }

    if (parent.column() != 0)
    {
        return 0;
    }

    if (parent.internalId() != INTERNALID)
    {
        return 0;    // Level 2: no children
    }

    // Level 1: item children count
    int parentId = parent.row();
    int rowCount = 0;

    foreach(const Item& item, m_collections)
    {
        if (!item.deleted && item.parentId == parentId)
        {
            ++rowCount;
        }
    }

    return rowCount;
}

int SetupCollectionModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 4;
}

Qt::ItemFlags SetupCollectionModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    if (index.internalId() == INTERNALID)
    {
        return Qt::ItemIsEnabled;
    }
    else
    {
        switch (index.column())
        {
            case ColumnName:
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

            default:
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
    }
}

bool SetupCollectionModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    // only editable in one case
    if (index.isValid() && index.internalId() != INTERNALID && index.column() == ColumnName && role == Qt::EditRole)
    {
        Item& item = m_collections[index.internalId()];
        item.label = value.toString();
        emit dataChanged(index, index);
    }

    return false;
}

QModelIndex SetupCollectionModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        if (row < NumberOfCategories && row >= 0 && column == 0)
        {
            return createIndex(row, 0, INTERNALID);
        }
    }
    else if (row >= 0 && column < 4)
    {
        // m_collections is a flat list with all entries, of all categories and also deleted entries.
        // The model indices contain as internal id the index to this list.
        int parentId = parent.row();
        int rowCount = 0;

        for (int i = 0; i < m_collections.count(); ++i)
        {
            const Item& item = m_collections.at(i);

            if (!item.deleted && item.parentId == parentId)
            {
                if (rowCount == row)
                {
                    return createIndex(row, column, i);
                }

                ++rowCount;
            }
        }
    }

    return QModelIndex();
}

QModelIndex SetupCollectionModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    if (index.internalId() == INTERNALID)
    {
        return QModelIndex();    // one of the three toplevel items
    }

    const Item& item = m_collections.at(index.internalId());

    return createIndex(item.parentId, 0, INTERNALID);
}

QModelIndex SetupCollectionModel::indexForCategory(Category category) const
{
    return index(category, 0, QModelIndex());
}

QList<QModelIndex> SetupCollectionModel::categoryIndexes() const
{
    QList<QModelIndex> list;

    for (int cat = 0; cat < NumberOfCategories; ++cat)
    {
        list << index(cat, 0, QModelIndex());
    }

    return list;
}

QModelIndex SetupCollectionModel::indexForId(int id, int column) const
{
    int   row             = 0;
    const Item& indexItem = m_collections.at(id);

    for (int i = 0; i < m_collections.count(); ++i)
    {
        const Item& item = m_collections.at(i);

        if (!item.deleted && item.parentId == indexItem.parentId)
        {
            if (i == id)
            {
                return createIndex(row, column, i);
            }

            ++row;
        }
    }

    return QModelIndex();
}

SetupCollectionModel::Category SetupCollectionModel::typeToCategory(CollectionLocation::Type type)
{
    switch (type)
    {
        default:
        case CollectionLocation::TypeVolumeHardWired:
            return CategoryLocal;

        case CollectionLocation::TypeVolumeRemovable:
            return CategoryRemovable;

        case CollectionLocation::TypeNetwork:
            return CategoryRemote;
    }
}

int SetupCollectionModel::categoryButtonMapId(const QModelIndex& index) const
{
    if (!index.isValid() || index.parent().isValid())
    {
        return INTERNALID;
    }

    return index.row();
}

int SetupCollectionModel::buttonMapId(const QModelIndex& index) const
{
    if (!index.isValid() || index.internalId() == INTERNALID)
    {
        return INTERNALID;
    }

    return index.internalId();
}

} // namespace Digikam
