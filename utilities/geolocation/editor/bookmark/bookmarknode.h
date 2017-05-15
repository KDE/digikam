/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : a node container for GPS bookmarks
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

#ifndef BOOKMARK_NODE_H
#define BOOKMARK_NODE_H

// Qt includes

#include <QXmlStreamReader>
#include <QDateTime>
#include <QXmlStreamWriter>

namespace Digikam
{

class BookmarkNode
{
public:

    enum Type
    {
        Root,
        Folder,
        Bookmark,
        Separator
    };

public:

    explicit BookmarkNode(Type type = Root, BookmarkNode* parent = 0);
    ~BookmarkNode();

    bool operator==(const BookmarkNode& other);

    Type type() const;
    void setType(Type type);
    
    QList<BookmarkNode*> children() const;
    BookmarkNode* parent()          const;

    void add(BookmarkNode* child, int offset = -1);
    void remove(BookmarkNode* child);

public:

    QString url;
    QString title;
    QString desc;
    bool    expanded;

private:

    BookmarkNode*        m_parent;
    Type                 m_type;
    QList<BookmarkNode*> m_children;
};

// -----------------------------------------------------------

class XbelReader : public QXmlStreamReader
{
public:

    explicit XbelReader();

    BookmarkNode* read(const QString& fileName);
    BookmarkNode* read(QIODevice* device);

private:

    void readXBEL(BookmarkNode* parent);
    void readTitle(BookmarkNode* parent);
    void readDescription(BookmarkNode* parent);
    void readSeparator(BookmarkNode* parent);
    void readFolder(BookmarkNode* parent);
    void readBookmarkNode(BookmarkNode* parent);
};

// -----------------------------------------------------------

class XbelWriter : public QXmlStreamWriter
{
public:

    explicit XbelWriter();

    bool write(const QString& fileName, const BookmarkNode* root);
    bool write(QIODevice* device, const BookmarkNode* root);

private:

    void writeItem(const BookmarkNode* parent);
};

} // namespace Digikam

#endif // BOOKMARK_NODE_H
