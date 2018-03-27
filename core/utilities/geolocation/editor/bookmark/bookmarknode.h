/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-15
 * Description : a node container for GPS bookmarks
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

#ifndef BOOKMARK_NODE_H
#define BOOKMARK_NODE_H

// Qt includes

#include <QString>
#include <QList>
#include <QDateTime>
#include <QIODevice>
#include <QXmlStreamReader>
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

    explicit BookmarkNode(Type type = Root, BookmarkNode* const parent = 0);
    ~BookmarkNode();

    bool operator==(const BookmarkNode& other) const;

    Type type() const;
    void setType(Type type);

    QList<BookmarkNode*> children() const;
    BookmarkNode*        parent()   const;

    void add(BookmarkNode* const child, int offset = -1);
    void remove(BookmarkNode* const child);

public:

    QString   url;
    QString   title;
    QString   desc;
    QDateTime dateAdded;
    bool      expanded;

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------

class XbelReader : public QXmlStreamReader
{
public:

    explicit XbelReader();

    BookmarkNode* read(const QString& fileName);
    BookmarkNode* read(QIODevice* const device);

private:

    void readXBEL(BookmarkNode* const parent);
    void readTitle(BookmarkNode* const parent);
    void readDescription(BookmarkNode* const parent);
    void readSeparator(BookmarkNode* const parent);
    void readFolder(BookmarkNode* const parent);
    void readBookmarkNode(BookmarkNode* const parent);
};

// -----------------------------------------------------------

class XbelWriter : public QXmlStreamWriter
{
public:

    explicit XbelWriter();

    bool write(const QString& fileName, const BookmarkNode* const root);
    bool write(QIODevice* const device, const BookmarkNode* const root);

private:

    void writeItem(const BookmarkNode* const parent);
};

} // namespace Digikam

#endif // BOOKMARK_NODE_H
