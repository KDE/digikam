/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : low level manager for GPS bookmarks
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BOOKMARKS_MNGR_H
#define BOOKMARKS_MNGR_H

// Qt includes

#include <QObject>
#include <QAbstractItemModel>
#include <QUndoCommand>
#include <QSortFilterProxyModel>

// Local includes

#include "searchtextbar.h"

namespace Digikam
{

class BookmarksManager;
class BookmarkNode;

class RemoveBookmarksCommand : public QUndoCommand
{
public:

    explicit RemoveBookmarksCommand(BookmarksManager* const mngr,
                                    BookmarkNode* const parent,
                                    int row);
    ~RemoveBookmarksCommand();

    void undo();
    void redo();

protected:

    int               m_row;
    BookmarksManager* m_bookmarkManager;
    BookmarkNode*     m_node;
    BookmarkNode*     m_parent;
    bool              m_done;
};

//---------------------------------------------------------------------------------

class InsertBookmarksCommand : public RemoveBookmarksCommand
{
public:

    explicit InsertBookmarksCommand(BookmarksManager* const mngr,
                                    BookmarkNode* const parent,
                                    BookmarkNode* const node,
                                    int row);

    void undo();
    void redo();
};

//---------------------------------------------------------------------------------

class ChangeBookmarkCommand : public QUndoCommand
{
public:

    enum BookmarkData
    {
        Url = 0,
        Title,
        Desc
    };

public:

    explicit ChangeBookmarkCommand(BookmarksManager* const mngr,
                                   BookmarkNode* const node,
                                   const QString& newValue,
                                   BookmarkData type);
    ~ChangeBookmarkCommand();

    void undo();
    void redo();

private:

    class Private;
    Private* const d;
};

/**
 * BookmarksModel is a QAbstractItemModel wrapper around the BookmarkManager
 */
class BookmarksModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum Roles
    {
        TypeRole      = Qt::UserRole + 1,
        UrlRole       = Qt::UserRole + 2,
        UrlStringRole = Qt::UserRole + 3,
        SeparatorRole = Qt::UserRole + 4,
        DateAddedRole = Qt::UserRole + 5
    };

public:

    explicit BookmarksModel(BookmarksManager* const mngr, QObject* const parent = 0);
    ~BookmarksModel();

    BookmarksManager* bookmarksManager()                                                      const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                       const;
    int columnCount(const QModelIndex& parent = QModelIndex())                                const;
    int rowCount(const QModelIndex& parent = QModelIndex())                                   const;
    QModelIndex index(int, int, const QModelIndex& = QModelIndex())                           const;
    QModelIndex parent(const QModelIndex& index= QModelIndex())                               const;
    Qt::ItemFlags flags(const QModelIndex& index)                                             const;
    Qt::DropActions supportedDropActions ()                                                   const;
    QMimeData* mimeData(const QModelIndexList& indexes)                                       const;
    QStringList mimeTypes()                                                                   const;
    bool hasChildren(const QModelIndex& parent = QModelIndex())                               const;
    BookmarkNode* node(const QModelIndex& index)                                              const;
    QModelIndex index(BookmarkNode* node)                                                     const;

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                      int column, const QModelIndex& parent);

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

public Q_SLOTS:

    void entryAdded(BookmarkNode* item);
    void entryRemoved(BookmarkNode* parent, int row, BookmarkNode* item);
    void entryChanged(BookmarkNode* item);

private:

    class Private;
    Private* const d;
};

/**
 *  Proxy model that filters out the bookmarks so only the folders
 *  are left behind.  Used in the add bookmark dialog combobox.
 */
class AddBookmarkProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit AddBookmarkProxyModel(QObject* const parent = 0);

    int columnCount(const QModelIndex& parent = QModelIndex()) const;

protected:

    bool filterAcceptsRow(int srow, const QModelIndex& sparent) const;
};

//---------------------------------------------------------------------------------

class TreeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit TreeProxyModel(QObject* const parent = 0);

    int columnCount(const QModelIndex&) const;

Q_SIGNALS:

    void signalFilterAccepts(bool);

protected:

    bool filterAcceptsRow(int srow, const QModelIndex& sparent) const;

private:

    void emitResult(bool v);
};

/**
 *  Bookmark manager, owner of the bookmarks, loads, saves and basic tasks
 */
class BookmarksManager : public QObject
{
    Q_OBJECT

public:

    explicit BookmarksManager(const QString& bookmarksFile, QObject* const parent = 0);
    ~BookmarksManager();

    void addBookmark(BookmarkNode* const parent, BookmarkNode* const node, int row = -1);
    void removeBookmark(BookmarkNode* const node);
    void setTitle(BookmarkNode* const node, const QString& newTitle);
    void setUrl(BookmarkNode* const node, const QString& newUrl);
    void setComment(BookmarkNode* const node, const QString& newDesc);
    void changeExpanded();

    BookmarkNode*   bookmarks();
    BookmarksModel* bookmarksModel();
    QUndoStack*     undoRedoStack() const;

    void save();
    void load();

Q_SIGNALS:

    void entryAdded(BookmarkNode* item);
    void entryRemoved(BookmarkNode* parent, int row, BookmarkNode* item);
    void entryChanged(BookmarkNode* item);

public Q_SLOTS:

    void importBookmarks();
    void exportBookmarks();

private:

    class Private;
    Private* const d;

    friend class RemoveBookmarksCommand;
    friend class ChangeBookmarkCommand;
};

} // namespace Digikam

#endif // BOOKMARKS_MNGR_H
