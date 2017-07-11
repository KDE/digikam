/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : low level manager for GPS bookmarks
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bookmarksmngr.h"

// Qt includes

#include <QBuffer>
#include <QFile>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QIcon>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QDebug>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include <dfiledialog.h>
#include "bookmarknode.h"
#include "digikam_debug.h"

namespace Digikam
{

RemoveBookmarksCommand::RemoveBookmarksCommand(BookmarksManager* const mngr,
                                               BookmarkNode* const parent,
                                               int row)
    : QUndoCommand(i18n("Remove Bookmark")),
      m_row(row),
      m_bookmarkManager(mngr),
      m_node(parent->children().value(row)),
      m_parent(parent),
      m_done(false)
{
}

RemoveBookmarksCommand::~RemoveBookmarksCommand()
{
    if (m_done && !m_node->parent())
    {
        delete m_node;
    }
}

void RemoveBookmarksCommand::undo()
{
    m_parent->add(m_node, m_row);
    emit m_bookmarkManager->entryAdded(m_node);
    m_done = false;
}

void RemoveBookmarksCommand::redo()
{
    m_parent->remove(m_node);
    emit m_bookmarkManager->entryRemoved(m_parent, m_row, m_node);
    m_done = true;
}

// --------------------------------------------------------------

InsertBookmarksCommand::InsertBookmarksCommand(BookmarksManager* const mngr,
                                               BookmarkNode* const parent,
                                               BookmarkNode* const node,
                                               int row)
    : RemoveBookmarksCommand(mngr, parent, row)
{
    setText(i18n("Insert Bookmark"));
    m_node = node;
}

void InsertBookmarksCommand::undo()
{
    RemoveBookmarksCommand::redo();
}

void InsertBookmarksCommand::redo()
{
    RemoveBookmarksCommand::undo();
}

// --------------------------------------------------------------

class ChangeBookmarkCommand::Private
{
public:

    Private() :
        manager(0),
        type(Url),
        node(0)
    {
    }

    BookmarksManager* manager;
    BookmarkData      type;
    QString           oldValue;
    QString           newValue;
    BookmarkNode*     node;
};

ChangeBookmarkCommand::ChangeBookmarkCommand(BookmarksManager* const mngr,
                                             BookmarkNode* const node,
                                             const QString& newValue,
                                             BookmarkData type)
    : QUndoCommand(),
      d(new Private)
{
    d->manager  = mngr;
    d->type     = type;
    d->newValue = newValue;
    d->node     = node;

    switch (d->type)
    {
        case Title:
            d->oldValue = d->node->title;
            setText(i18n("Title Change"));
            break;
        case Desc:
            d->oldValue = d->node->desc;
            setText(i18n("Comment Change"));
            break;
        default:    // Url
            d->oldValue = d->node->url;
            setText(i18n("Address Change"));
            break;
    }
}

ChangeBookmarkCommand::~ChangeBookmarkCommand()
{
    delete d;
}

void ChangeBookmarkCommand::undo()
{
    switch (d->type)
    {
        case Title:
            d->node->title = d->oldValue;
            break;
        case Desc:
            d->node->desc  = d->oldValue;
            break;
        default:    // Url
            d->node->url   = d->oldValue;
            break;
    }

    emit d->manager->entryChanged(d->node);
}

void ChangeBookmarkCommand::redo()
{
    switch (d->type)
    {
        case Title:
            d->node->title = d->newValue;
            break;
        case Desc:
            d->node->desc  = d->newValue;
            break;
        default:    // Url
            d->node->url   = d->newValue;
            break;
    }

    emit d->manager->entryChanged(d->node);
}

// --------------------------------------------------------------

class BookmarksModel::Private
{
public:

    Private() :
        manager(0),
        endMacro(false)
    {
    }

    BookmarksManager* manager;
    bool              endMacro;
};

BookmarksModel::BookmarksModel(BookmarksManager* const mngr, QObject* const parent)
    : QAbstractItemModel(parent),
      d(new Private)
{
    d->manager = mngr;

    connect(d->manager, SIGNAL(entryAdded(BookmarkNode*)),
            this, SLOT(entryAdded(BookmarkNode*)));

    connect(d->manager, SIGNAL(entryRemoved(BookmarkNode*,int,BookmarkNode*)),
            this, SLOT(entryRemoved(BookmarkNode*,int,BookmarkNode*)));

    connect(d->manager, SIGNAL(entryChanged(BookmarkNode*)),
            this, SLOT(entryChanged(BookmarkNode*)));
}

BookmarksModel::~BookmarksModel()
{
    delete d;
}

BookmarksManager* BookmarksModel::bookmarksManager() const
{
    return d->manager;
}

QModelIndex BookmarksModel::index(BookmarkNode* node) const
{
    BookmarkNode* const parent = node->parent();

    if (!parent)
        return QModelIndex();

    return createIndex(parent->children().indexOf(node), 0, node);
}

void BookmarksModel::entryAdded(BookmarkNode* item)
{
    Q_ASSERT(item && item->parent());

    int row                    = item->parent()->children().indexOf(item);
    BookmarkNode* const parent = item->parent();

    // item was already added so remove beore beginInsertRows is called
    parent->remove(item);
    beginInsertRows(index(parent), row, row);
    parent->add(item, row);
    endInsertRows();
}

void BookmarksModel::entryRemoved(BookmarkNode* parent, int row, BookmarkNode* item)
{
    // item was already removed, re-add so beginRemoveRows works
    parent->add(item, row);
    beginRemoveRows(index(parent), row, row);
    parent->remove(item);
    endRemoveRows();
}

void BookmarksModel::entryChanged(BookmarkNode* item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

bool BookmarksModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || count <= 0 || (row + count) > rowCount(parent))
        return false;

    BookmarkNode* const bookmarkNode = node(parent);

    for (int i = (row + count - 1) ; i >= row ; i--)
    {
        BookmarkNode* const node = bookmarkNode->children().at(i);
        d->manager->removeBookmark(node);
    }

    if (d->endMacro)
    {
        d->manager->undoRedoStack()->endMacro();
        d->endMacro = false;
    }

    return true;
}

QVariant BookmarksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case 0:
                return i18n("Title");
            case 1:
                return i18n("Comment");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant BookmarksModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    const BookmarkNode* const bookmarkNode = node(index);

    switch (role)
    {
        case Qt::EditRole:
        case Qt::DisplayRole:
            if (bookmarkNode->type() == BookmarkNode::Separator)
            {
                switch (index.column())
                {
                    case 0:
                        return QString(50, 0xB7);
                    case 1:
                        return QString();
                }
            }

            switch (index.column())
            {
                case 0:
                    return bookmarkNode->title;
                case 1:
                    return bookmarkNode->desc;
            }
            break;
        case BookmarksModel::UrlRole:
            return QUrl(bookmarkNode->url);
            break;
        case BookmarksModel::UrlStringRole:
            return bookmarkNode->url;
            break;
        case BookmarksModel::DateAddedRole:
            return bookmarkNode->dateAdded;
            break;
        case BookmarksModel::TypeRole:
            return bookmarkNode->type();
            break;
        case BookmarksModel::SeparatorRole:
            return (bookmarkNode->type() == BookmarkNode::Separator);
            break;
        case Qt::DecorationRole:
            if (index.column() == 0)
            {
                if (bookmarkNode->type() == BookmarkNode::Bookmark)
                {
                    return QIcon::fromTheme(QLatin1String("globe"));
                }
                else
                {
                    return QIcon::fromTheme(QLatin1String("folder"));
                }
            }
    }

    return QVariant();
}

int BookmarksModel::columnCount(const QModelIndex& parent) const
{
    return (parent.column() > 0) ? 0 : 2;
}

int BookmarksModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return d->manager->bookmarks()->children().count();

    const BookmarkNode* const item = static_cast<BookmarkNode*>(parent.internalPointer());

    return item->children().count();
}

QModelIndex BookmarksModel::index(int row, int column, const QModelIndex& parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    // get the parent node
    BookmarkNode* const parentNode = node(parent);

    return createIndex(row, column, parentNode->children().at(row));
}

QModelIndex BookmarksModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    BookmarkNode* const itemNode   = node(index);
    BookmarkNode* const parentNode = (itemNode ? itemNode->parent() : 0);

    if (!parentNode || parentNode == d->manager->bookmarks())
        return QModelIndex();

    // get the parent's row
    BookmarkNode* const grandParentNode = parentNode->parent();
    int parentRow                       = grandParentNode->children().indexOf(parentNode);

    Q_ASSERT(parentRow >= 0);

    return createIndex(parentRow, 0, parentNode);
}

bool BookmarksModel::hasChildren(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return true;

    const BookmarkNode* const parentNode = node(parent);

    return (parentNode->type() == BookmarkNode::Folder);
}

Qt::ItemFlags BookmarksModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags              = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    BookmarkNode* const bookmarkNode = node(index);

    flags |= Qt::ItemIsDragEnabled;

    if (bookmarkNode->type() != BookmarkNode::Separator)
        flags |= Qt::ItemIsEditable;

    if (hasChildren(index))
        flags |= Qt::ItemIsDropEnabled;

    return flags;
}

Qt::DropActions BookmarksModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList BookmarksModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/bookmarks.xbel");
    return types;
}

QMimeData* BookmarksModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* const mimeData = new QMimeData();
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes)
    {
        if (index.column() != 0 || !index.isValid())
            continue;

        QByteArray encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadWrite);
        XbelWriter writer;
        const BookmarkNode* const parentNode = node(index);
        writer.write(&buffer, parentNode);
        stream << encodedData;
    }

    mimeData->setData(QLatin1String("application/bookmarks.xbel"), data);
    return mimeData;
}

bool BookmarksModel::dropMimeData(const QMimeData* data,
                                  Qt::DropAction action,
                                  int row, int column,
                                  const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(QLatin1String("application/bookmarks.xbel")) || column > 0)
        return false;

    QByteArray ba = data->data(QLatin1String("application/bookmarks.xbel"));
    QDataStream stream(&ba, QIODevice::ReadOnly);

    if (stream.atEnd())
        return false;

    QUndoStack* const undoStack = d->manager->undoRedoStack();
    undoStack->beginMacro(QLatin1String("Move Bookmarks"));

    while (!stream.atEnd())
    {
        QByteArray encodedData;
        stream >> encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadOnly);

        XbelReader reader;
        BookmarkNode* const rootNode  = reader.read(&buffer);
        QList<BookmarkNode*> children = rootNode->children();

        for (int i = 0 ; i < children.count() ; i++)
        {
            BookmarkNode* const bookmarkNode = children.at(i);
            rootNode->remove(bookmarkNode);
            row                              = qMax(0, row);
            BookmarkNode* const parentNode   = node(parent);
            d->manager->addBookmark(parentNode, bookmarkNode, row);
            d->endMacro                      = true;
        }

        delete rootNode;
    }
    return true;
}

bool BookmarksModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || (flags(index) & Qt::ItemIsEditable) == 0)
        return false;

    BookmarkNode* const item = node(index);

    switch (role)
    {
        case Qt::EditRole:
        case Qt::DisplayRole:
            if (index.column() == 0)
            {
                d->manager->setTitle(item, value.toString());
                break;
            }

            if (index.column() == 1)
            {
                d->manager->setComment(item, value.toString());
                break;
            }

            return false;
        case BookmarksModel::UrlRole:
            d->manager->setUrl(item, value.toUrl().toString());
            break;
        case BookmarksModel::UrlStringRole:
            d->manager->setUrl(item, value.toString());
            break;
        default:
            return false;
    }

    return true;
}

BookmarkNode* BookmarksModel::node(const QModelIndex& index) const
{
    BookmarkNode* const itemNode = static_cast<BookmarkNode*>(index.internalPointer());

    if (!itemNode)
        return d->manager->bookmarks();

    return itemNode;
}

// --------------------------------------------------------------

AddBookmarkProxyModel::AddBookmarkProxyModel(QObject* const parent)
    : QSortFilterProxyModel(parent)
{
}

int AddBookmarkProxyModel::columnCount(const QModelIndex& parent) const
{
    return qMin(1, QSortFilterProxyModel::columnCount(parent));
}

bool AddBookmarkProxyModel::filterAcceptsRow(int srow, const QModelIndex& sparent) const
{
    QModelIndex idx = sourceModel()->index(srow, 0, sparent);

    return sourceModel()->hasChildren(idx);
}

// --------------------------------------------------------------

TreeProxyModel::TreeProxyModel(QObject* const parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

int TreeProxyModel::columnCount(const QModelIndex&) const
{
    // 1th column : Title
    // 2th column : Comment
    return 2;
}

bool TreeProxyModel::filterAcceptsRow(int srow, const QModelIndex& sparent) const
{
    QModelIndex index = sourceModel()->index(srow, 0, sparent);

    if (!index.isValid())
    {
        return false;
    }

    if (index.data().toString().contains(filterRegExp()))
    {
        return true;
    }

    for (int i = 0 ; i < sourceModel()->rowCount(index) ; i++)
    {
        if (filterAcceptsRow(i, index))
        {
            return true;
        }
    }

    return false;
}

void TreeProxyModel::emitResult(bool v)
{
    emit signalFilterAccepts(v);
}

// --------------------------------------------------------------

class BookmarksManager::Private
{
public:

    Private() :
        loaded(false),
        bookmarkRootNode(0),
        bookmarkModel(0)
    {
    }

    bool            loaded;
    BookmarkNode*   bookmarkRootNode;
    BookmarksModel* bookmarkModel;
    QUndoStack      commands;
    QString         bookmarksFile;
};

BookmarksManager::BookmarksManager(const QString& bookmarksFile, QObject* const parent)
    : QObject(parent),
      d(new Private)
{
    d->bookmarksFile = bookmarksFile;
    load();
}

BookmarksManager::~BookmarksManager()
{
    delete d;
}

void BookmarksManager::changeExpanded()
{
}

void BookmarksManager::load()
{
    if (d->loaded)
        return;

    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Loading GPS bookmarks from" << d->bookmarksFile;
    d->loaded = true;

    XbelReader reader;
    d->bookmarkRootNode = reader.read(d->bookmarksFile);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(0, i18n("Loading Bookmark"),
                             i18n("Error when loading bookmarks on line %1, column %2:\n%3")
                             .arg(reader.lineNumber())
                             .arg(reader.columnNumber())
                             .arg(reader.errorString()));
    }
}

void BookmarksManager::save()
{
    if (!d->loaded)
        return;

    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Saving GPS bookmarks to" << d->bookmarksFile;

    XbelWriter writer;

    if (!writer.write(d->bookmarksFile, d->bookmarkRootNode))
    {
        qCWarning(DIGIKAM_GEOIFACE_LOG) << "BookmarkManager: error saving to" << d->bookmarksFile;
    }
}

void BookmarksManager::addBookmark(BookmarkNode* const parent, BookmarkNode* const node, int row)
{
    if (!d->loaded)
        return;

    Q_ASSERT(parent);

    InsertBookmarksCommand* const command = new InsertBookmarksCommand(this, parent, node, row);
    d->commands.push(command);
}

void BookmarksManager::removeBookmark(BookmarkNode* const node)
{
    if (!d->loaded)
        return;

    Q_ASSERT(node);

    BookmarkNode* const parent            = node->parent();
    int row                               = parent->children().indexOf(node);
    RemoveBookmarksCommand* const command = new RemoveBookmarksCommand(this, parent, row);
    d->commands.push(command);
}

void BookmarksManager::setTitle(BookmarkNode* const node, const QString& newTitle)
{
    if (!d->loaded)
        return;

    Q_ASSERT(node);

    ChangeBookmarkCommand* const command = new ChangeBookmarkCommand(this, node, newTitle,
                                                                     ChangeBookmarkCommand::Title);
    d->commands.push(command);
}

void BookmarksManager::setUrl(BookmarkNode* const node, const QString& newUrl)
{
    if (!d->loaded)
        return;

    Q_ASSERT(node);

    ChangeBookmarkCommand* const command = new ChangeBookmarkCommand(this, node, newUrl,
                                                                     ChangeBookmarkCommand::Url);
    d->commands.push(command);
}

void BookmarksManager::setComment(BookmarkNode* const node, const QString& newDesc)
{
    if (!d->loaded)
        return;

    Q_ASSERT(node);

    ChangeBookmarkCommand* const command = new ChangeBookmarkCommand(this, node, newDesc,
                                                                     ChangeBookmarkCommand::Desc);
    d->commands.push(command);
}

BookmarkNode* BookmarksManager::bookmarks()
{
    if (!d->loaded)
        load();

    return d->bookmarkRootNode;
}

BookmarksModel* BookmarksManager::bookmarksModel()
{
    if (!d->bookmarkModel)
        d->bookmarkModel = new BookmarksModel(this, this);

    return d->bookmarkModel;
}

QUndoStack* BookmarksManager::undoRedoStack() const
{
    return &d->commands;
}

void BookmarksManager::importBookmarks()
{
    QString fileName = DFileDialog::getOpenFileName(0, i18n("Open File"),
                                                    QString(),
                                                    i18n("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
        return;

    XbelReader reader;
    BookmarkNode* const importRootNode = reader.read(fileName);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(0, i18n("Loading Bookmark"),
                             i18n("Error when loading bookmarks on line %1, column %2:\n%3")
                             .arg(reader.lineNumber())
                             .arg(reader.columnNumber())
                             .arg(reader.errorString()));
    }

    importRootNode->setType(BookmarkNode::Folder);
    importRootNode->title = i18n("Imported %1")
                            .arg(QDate::currentDate().toString(Qt::SystemLocaleShortDate));
    addBookmark(bookmarks(), importRootNode);
}

void BookmarksManager::exportBookmarks()
{
    QString fileName = DFileDialog::getSaveFileName(0, i18n("Save File"),
                                                    i18n("%1 Bookmarks.xbel")
                                                    .arg(QCoreApplication::applicationName()),
                                                    i18n("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
        return;

    XbelWriter writer;

    if (!writer.write(fileName, d->bookmarkRootNode))
        QMessageBox::critical(0, i18n("Export error"), i18n("error saving bookmarks"));
}

} // namespace Digikam
