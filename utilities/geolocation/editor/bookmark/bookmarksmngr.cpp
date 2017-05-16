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
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QIcon>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QDebug>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "bookmarknode.h"

#define BOOKMARKBAR "Bookmarks Bar"
#define BOOKMARKMENU "Bookmarks Menu"
#define MIMETYPE QLatin1String("application/bookmarks.xbel")

namespace Digikam
{

RemoveBookmarksCommand::RemoveBookmarksCommand(BookmarksManager* mngr, BookmarkNode* parent, int row)
    : QUndoCommand(i18n("Remove Bookmark"))
    , m_row(row)
    , m_bookmarkManager(mngr)
    , m_node(parent->children().value(row))
    , m_parent(parent)
    , m_done(false)
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

InsertBookmarksCommand::InsertBookmarksCommand(BookmarksManager *m_bookmarkManagaer,
                BookmarkNode *parent, BookmarkNode *node, int row)
    : RemoveBookmarksCommand(m_bookmarkManagaer, parent, row)
{
    setText(i18n("Insert Bookmark"));
    m_node = node;
}

// --------------------------------------------------------------

ChangeBookmarkCommand::ChangeBookmarkCommand(BookmarksManager* mngr, BookmarkNode* node,
                        const QString &newValue, bool title)
    : QUndoCommand()
    , m_bookmarkManager(mngr)
    , m_title(title)
    , m_newValue(newValue)
    , m_node(node)
{
    if (m_title)
    {
        m_oldValue = m_node->title;
        setText(i18n("Name Change"));
    }
    else
    {
        m_oldValue = m_node->url;
        setText(i18n("Address Change"));
    }
}

void ChangeBookmarkCommand::undo()
{
    if (m_title)
        m_node->title = m_oldValue;
    else
        m_node->url = m_oldValue;

    emit m_bookmarkManager->entryChanged(m_node);
}

void ChangeBookmarkCommand::redo()
{
    if (m_title)
        m_node->title = m_newValue;
    else
        m_node->url = m_newValue;

    emit m_bookmarkManager->entryChanged(m_node);
}

// --------------------------------------------------------------

BookmarksModel::BookmarksModel(BookmarksManager* mngr, QObject *parent)
    : QAbstractItemModel(parent)
    , m_endMacro(false)
    , m_bookmarksManager(mngr)
{
    connect(m_bookmarksManager, SIGNAL(entryAdded(BookmarkNode*)),
            this, SLOT(entryAdded(BookmarkNode*)));

    connect(m_bookmarksManager, SIGNAL(entryRemoved(BookmarkNode*,int,BookmarkNode*)),
            this, SLOT(entryRemoved(BookmarkNode*,int,BookmarkNode*)));

    connect(m_bookmarksManager, SIGNAL(entryChanged(BookmarkNode*)),
            this, SLOT(entryChanged(BookmarkNode*)));
}

QModelIndex BookmarksModel::index(BookmarkNode *node) const
{
    BookmarkNode *parent = node->parent();
    if (!parent)
        return QModelIndex();
    return createIndex(parent->children().indexOf(node), 0, node);
}

void BookmarksModel::entryAdded(BookmarkNode *item)
{
    Q_ASSERT(item && item->parent());
    int row = item->parent()->children().indexOf(item);
    BookmarkNode *parent = item->parent();
    // item was already added so remove beore beginInsertRows is called
    parent->remove(item);
    beginInsertRows(index(parent), row, row);
    parent->add(item, row);
    endInsertRows();
}

void BookmarksModel::entryRemoved(BookmarkNode *parent, int row, BookmarkNode *item)
{
    // item was already removed, re-add so beginRemoveRows works
    parent->add(item, row);
    beginRemoveRows(index(parent), row, row);
    parent->remove(item);
    endRemoveRows();
}

void BookmarksModel::entryChanged(BookmarkNode *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

bool BookmarksModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count <= 0 || row + count > rowCount(parent))
        return false;

    BookmarkNode *bookmarkNode = node(parent);
    for (int i = row + count - 1; i >= row; --i) {
        BookmarkNode *node = bookmarkNode->children().at(i);
        if (node == m_bookmarksManager->menu()
            || node == m_bookmarksManager->toolbar())
            continue;

        m_bookmarksManager->removeBookmark(node);
    }
    if (m_endMacro) {
        m_bookmarksManager->undoRedoStack()->endMacro();
        m_endMacro = false;
    }
    return true;
}

QVariant BookmarksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return i18n("Title");
            case 1: return i18n("Address");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant BookmarksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    const BookmarkNode *bookmarkNode = node(index);
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (bookmarkNode->type() == BookmarkNode::Separator) {
            switch (index.column()) {
            case 0: return QString(50, 0xB7);
            case 1: return QString();
            }
        }

        switch (index.column()) {
        case 0: return bookmarkNode->title;
        case 1: return bookmarkNode->url;
        }
        break;
    case BookmarksModel::UrlRole:
        return QUrl(bookmarkNode->url);
        break;
    case BookmarksModel::UrlStringRole:
        return bookmarkNode->url;
        break;
    case BookmarksModel::TypeRole:
        return bookmarkNode->type();
        break;
    case BookmarksModel::SeparatorRole:
        return (bookmarkNode->type() == BookmarkNode::Separator);
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            //if (bookmarkNode->type() == BookmarkNode::Folder)
                return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
            //return BrowserApplication::instance()->icon(bookmarkNode->url);
        }
    }

    return QVariant();
}

int BookmarksModel::columnCount(const QModelIndex &parent) const
{
    return (parent.column() > 0) ? 0 : 2;
}

int BookmarksModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return m_bookmarksManager->bookmarks()->children().count();

    const BookmarkNode *item = static_cast<BookmarkNode*>(parent.internalPointer());
    return item->children().count();
}

QModelIndex BookmarksModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    // get the parent node
    BookmarkNode *parentNode = node(parent);
    return createIndex(row, column, parentNode->children().at(row));
}

QModelIndex BookmarksModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    BookmarkNode *itemNode = node(index);
    BookmarkNode *parentNode = (itemNode ? itemNode->parent() : 0);
    if (!parentNode || parentNode == m_bookmarksManager->bookmarks())
        return QModelIndex();

    // get the parent's row
    BookmarkNode *grandParentNode = parentNode->parent();
    int parentRow = grandParentNode->children().indexOf(parentNode);
    Q_ASSERT(parentRow >= 0);
    return createIndex(parentRow, 0, parentNode);
}

bool BookmarksModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;
    const BookmarkNode *parentNode = node(parent);
    return (parentNode->type() == BookmarkNode::Folder);
}

Qt::ItemFlags BookmarksModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    BookmarkNode *bookmarkNode = node(index);

    if (bookmarkNode != m_bookmarksManager->menu()
        && bookmarkNode != m_bookmarksManager->toolbar()) {
        flags |= Qt::ItemIsDragEnabled;
        if (bookmarkNode->type() != BookmarkNode::Separator)
            flags |= Qt::ItemIsEditable;
    }
    if (hasChildren(index))
        flags |= Qt::ItemIsDropEnabled;
    return flags;
}

Qt::DropActions BookmarksModel::supportedDropActions () const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList BookmarksModel::mimeTypes() const
{
    QStringList types;
    types << MIMETYPE;
    return types;
}

QMimeData *BookmarksModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    foreach (QModelIndex index, indexes) {
        if (index.column() != 0 || !index.isValid())
            continue;
        QByteArray encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadWrite);
        XbelWriter writer;
        const BookmarkNode *parentNode = node(index);
        writer.write(&buffer, parentNode);
        stream << encodedData;
    }
    mimeData->setData(MIMETYPE, data);
    return mimeData;
}

bool BookmarksModel::dropMimeData(const QMimeData *data,
     Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(MIMETYPE)
        || column > 0)
        return false;

    QByteArray ba = data->data(MIMETYPE);
    QDataStream stream(&ba, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    QUndoStack *undoStack = m_bookmarksManager->undoRedoStack();
    undoStack->beginMacro(QLatin1String("Move Bookmarks"));

    while (!stream.atEnd()) {
        QByteArray encodedData;
        stream >> encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadOnly);

        XbelReader reader;
        BookmarkNode *rootNode = reader.read(&buffer);
        QList<BookmarkNode*> children = rootNode->children();
        for (int i = 0; i < children.count(); ++i) {
            BookmarkNode *bookmarkNode = children.at(i);
            rootNode->remove(bookmarkNode);
            row = qMax(0, row);
            BookmarkNode *parentNode = node(parent);
            m_bookmarksManager->addBookmark(parentNode, bookmarkNode, row);
            m_endMacro = true;
        }
        delete rootNode;
    }
    return true;
}

bool BookmarksModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || (flags(index) & Qt::ItemIsEditable) == 0)
        return false;

    BookmarkNode *item = node(index);

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (index.column() == 0) {
            m_bookmarksManager->setTitle(item, value.toString());
            break;
        }
        if (index.column() == 1) {
            m_bookmarksManager->setUrl(item, value.toString());
            break;
        }
        return false;
    case BookmarksModel::UrlRole:
        m_bookmarksManager->setUrl(item, value.toUrl().toString());
        break;
    case BookmarksModel::UrlStringRole:
        m_bookmarksManager->setUrl(item, value.toString());
        break;
    default:
        break;
        return false;
    }

    return true;
}

BookmarkNode *BookmarksModel::node(const QModelIndex &index) const
{
    BookmarkNode *itemNode = static_cast<BookmarkNode*>(index.internalPointer());
    if (!itemNode)
        return m_bookmarksManager->bookmarks();
    return itemNode;
}

// --------------------------------------------------------------

AddBookmarkProxyModel::AddBookmarkProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

int AddBookmarkProxyModel::columnCount(const QModelIndex &parent) const
{
    return qMin(1, QSortFilterProxyModel::columnCount(parent));
}

bool AddBookmarkProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    return sourceModel()->hasChildren(idx);
}

// --------------------------------------------------------------

TreeProxyModel::TreeProxyModel(QObject* const parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool TreeProxyModel::filterAcceptsRow(int srow, const QModelIndex& source_parent) const
{
    bool ret = QSortFilterProxyModel::filterAcceptsRow(srow, source_parent);

    return ret;
}

void TreeProxyModel::emitResult(bool v)
{
    emit signalFilterAccepts(v);
}

// --------------------------------------------------------------

BookmarksManager::BookmarksManager(const QString& bookmarksFile, QObject* const parent)
    : QObject(parent),
      m_loaded(false),
      m_bookmarkRootNode(0),
      m_bookmarkModel(0),
      m_bookmarksFile(bookmarksFile)
{
    load();
}

BookmarksManager::~BookmarksManager()
{
}

void BookmarksManager::changeExpanded()
{
}

void BookmarksManager::load()
{
    if (m_loaded)
        return;

    qDebug() << "Loading GPS bookmarks from" << m_bookmarksFile;
    m_loaded = true;

    XbelReader reader;
    m_bookmarkRootNode = reader.read(m_bookmarksFile);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(0, QLatin1String("Loading Bookmark"),
            i18n("Error when loading bookmarks on line %1, column %2:\n"
               "%3").arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.errorString()));
    }

    BookmarkNode* toolbar = 0;
    BookmarkNode* menu    = 0;
    QList<BookmarkNode*> others;

    for (int i = m_bookmarkRootNode->children().count() - 1; i >= 0; --i)
    {
        BookmarkNode *node = m_bookmarkRootNode->children().at(i);

        if (node->type() == BookmarkNode::Folder)
        {
            // Automatically convert
            if (node->title == i18n("Toolbar Bookmarks") && !toolbar)
            {
                node->title = i18n(BOOKMARKBAR);
            }

            if (node->title == i18n(BOOKMARKBAR) && !toolbar)
            {
                toolbar = node;
            }

            // Automatically convert
            if (node->title == i18n("Menu") && !menu)
            {
                node->title = i18n(BOOKMARKMENU);
            }

            if (node->title == i18n(BOOKMARKMENU) && !menu)
            {
                menu = node;
            }
        }
        else
        {
            others.append(node);
        }

        m_bookmarkRootNode->remove(node);
    }

    Q_ASSERT(m_bookmarkRootNode->children().count() == 0);

    if (!toolbar)
    {
        toolbar = new BookmarkNode(BookmarkNode::Folder, m_bookmarkRootNode);
        toolbar->title = i18n(BOOKMARKBAR);
    }
    else
    {
        m_bookmarkRootNode->add(toolbar);
    }

    if (!menu)
    {
        menu = new BookmarkNode(BookmarkNode::Folder, m_bookmarkRootNode);
        menu->title = i18n(BOOKMARKMENU);
    }
    else
    {
        m_bookmarkRootNode->add(menu);
    }

    for (int i = 0; i < others.count(); ++i)
        menu->add(others.at(i));
}

void BookmarksManager::save()
{
    if (!m_loaded)
        return;

    qDebug() << "Saving GPS bookmarks to" << m_bookmarksFile;

    XbelWriter writer;

    if (!writer.write(m_bookmarksFile, m_bookmarkRootNode))
        qWarning() << "BookmarkManager: error saving to" << m_bookmarksFile;
}

void BookmarksManager::addBookmark(BookmarkNode* parent, BookmarkNode* node, int row)
{
    if (!m_loaded)
        return;

    Q_ASSERT(parent);
    InsertBookmarksCommand* const command = new InsertBookmarksCommand(this, parent, node, row);
    m_commands.push(command);
}

void BookmarksManager::removeBookmark(BookmarkNode *node)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
    BookmarkNode* const parent            = node->parent();
    int row                               = parent->children().indexOf(node);
    RemoveBookmarksCommand* const command = new RemoveBookmarksCommand(this, parent, row);
    m_commands.push(command);
}

void BookmarksManager::setTitle(BookmarkNode *node, const QString &newTitle)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
    ChangeBookmarkCommand *command = new ChangeBookmarkCommand(this, node, newTitle, true);
    m_commands.push(command);
}

void BookmarksManager::setUrl(BookmarkNode *node, const QString &newUrl)
{
    if (!m_loaded)
        return;

    Q_ASSERT(node);
    ChangeBookmarkCommand *command = new ChangeBookmarkCommand(this, node, newUrl, false);
    m_commands.push(command);
}

BookmarkNode *BookmarksManager::bookmarks()
{
    if (!m_loaded)
        load();

    return m_bookmarkRootNode;
}

BookmarkNode *BookmarksManager::menu()
{
    if (!m_loaded)
        load();

    for (int i = m_bookmarkRootNode->children().count() - 1; i >= 0; --i)
    {
        BookmarkNode *node = m_bookmarkRootNode->children().at(i);

        if (node->title == i18n(BOOKMARKMENU))
            return node;
    }

    Q_ASSERT(false);

    return 0;
}

BookmarkNode *BookmarksManager::toolbar()
{
    if (!m_loaded)
        load();

    for (int i = m_bookmarkRootNode->children().count() - 1; i >= 0; --i)
    {
        BookmarkNode *node = m_bookmarkRootNode->children().at(i);

        if (node->title == i18n(BOOKMARKBAR))
            return node;
    }

    Q_ASSERT(false);

    return 0;
}

BookmarksModel *BookmarksManager::bookmarksModel()
{
    if (!m_bookmarkModel)
        m_bookmarkModel = new BookmarksModel(this, this);

    return m_bookmarkModel;
}

void BookmarksManager::importBookmarks()
{
    QString fileName = QFileDialog::getOpenFileName(0, i18n("Open File"),
                                                     QString(),
                                                     i18n("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
        return;

    XbelReader reader;
    BookmarkNode *importRootNode = reader.read(fileName);
    if (reader.error() != QXmlStreamReader::NoError) {
        QMessageBox::warning(0, QLatin1String("Loading Bookmark"),
            i18n("Error when loading bookmarks on line %1, column %2:\n"
               "%3").arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.errorString()));
    }

    importRootNode->setType(BookmarkNode::Folder);
    importRootNode->title = (i18n("Imported %1").arg(QDate::currentDate().toString(Qt::SystemLocaleShortDate)));
    addBookmark(menu(), importRootNode);
}

void BookmarksManager::exportBookmarks()
{
    QString fileName = QFileDialog::getSaveFileName(0, i18n("Save File"),
                                i18n("%1 Bookmarks.xbel").arg(QCoreApplication::applicationName()),
                                i18n("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
        return;

    XbelWriter writer;
    if (!writer.write(fileName, m_bookmarkRootNode))
        QMessageBox::critical(0, i18n("Export error"), i18n("error saving bookmarks"));
}

} // namespace Digikam
