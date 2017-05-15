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

#include "bookmarknode.h"

// Qt includes

#include <QFile>

namespace Digikam
{

BookmarkNode::BookmarkNode(BookmarkNode::Type type, BookmarkNode *parent) :
     expanded(false),
     m_parent(parent),
     m_type(type)
{
    if (parent)
        parent->add(this);
}

BookmarkNode::~BookmarkNode()
{
    if (m_parent)
        m_parent->remove(this);

    qDeleteAll(m_children);
    m_parent = 0;
    m_type   = BookmarkNode::Root;
}

bool BookmarkNode::operator==(const BookmarkNode &other)
{
    if (url != other.url
        || title != other.title
        || desc != other.desc
        || expanded != other.expanded
        || m_type != other.m_type
        || m_children.count() != other.m_children.count())
        return false;

    for (int i = 0; i < m_children.count(); ++i)
        if (!((*(m_children[i])) == (*(other.m_children[i]))))
            return false;
    return true;
}

BookmarkNode::Type BookmarkNode::type() const
{
    return m_type;
}

void BookmarkNode::setType(Type type)
{
    m_type = type;
}

QList<BookmarkNode *> BookmarkNode::children() const
{
    return m_children;
}

BookmarkNode *BookmarkNode::parent() const
{
    return m_parent;
}

void BookmarkNode::add(BookmarkNode *child, int offset)
{
    Q_ASSERT(child->m_type != Root);
    if (child->m_parent)
        child->m_parent->remove(child);
    child->m_parent = this;
    if (-1 == offset)
        offset = m_children.size();
    m_children.insert(offset, child);
}

void BookmarkNode::remove(BookmarkNode *child)
{
    child->m_parent = 0;
    m_children.removeAll(child);
}


XbelReader::XbelReader()
{
}

BookmarkNode *XbelReader::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists()) {
        return new BookmarkNode(BookmarkNode::Root);
    }
    file.open(QFile::ReadOnly);
    return read(&file);
}

BookmarkNode *XbelReader::read(QIODevice *device)
{
    BookmarkNode *root = new BookmarkNode(BookmarkNode::Root);
    setDevice(device);
    if (readNextStartElement()) {
        QString version = attributes().value(QLatin1String("version")).toString();
        if (name() == QLatin1String("xbel")
            && (version.isEmpty() || version == QLatin1String("1.0"))) {
            readXBEL(root);
        } else {
            raiseError(QObject::tr("The file is not an XBEL version 1.0 file."));
        }
    }
    return root;
}

void XbelReader::readXBEL(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("xbel"));

    while (readNextStartElement()) {
        if (name() == QLatin1String("folder"))
            readFolder(parent);
        else if (name() == QLatin1String("bookmark"))
            readBookmarkNode(parent);
        else if (name() == QLatin1String("separator"))
            readSeparator(parent);
        else
            skipCurrentElement();
    }
}

void XbelReader::readFolder(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("folder"));

    BookmarkNode *folder = new BookmarkNode(BookmarkNode::Folder, parent);
    folder->expanded = (attributes().value(QLatin1String("folded")) == QLatin1String("no"));

    while (readNextStartElement()) {
        if (name() == QLatin1String("title"))
            readTitle(folder);
        else if (name() == QLatin1String("desc"))
            readDescription(folder);
        else if (name() == QLatin1String("folder"))
            readFolder(folder);
        else if (name() == QLatin1String("bookmark"))
            readBookmarkNode(folder);
        else if (name() == QLatin1String("separator"))
            readSeparator(folder);
        else
            skipCurrentElement();
    }
}

void XbelReader::readTitle(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("title"));
    parent->title = readElementText();
}

void XbelReader::readDescription(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("desc"));
    parent->desc = readElementText();
}

void XbelReader::readSeparator(BookmarkNode *parent)
{
    new BookmarkNode(BookmarkNode::Separator, parent);
    // empty elements have a start and end element
    readNext();
}

void XbelReader::readBookmarkNode(BookmarkNode *parent)
{
    Q_ASSERT(isStartElement() && name() == QLatin1String("bookmark"));
    BookmarkNode *bookmark = new BookmarkNode(BookmarkNode::Bookmark, parent);
    bookmark->url = attributes().value(QLatin1String("href")).toString();
    while (readNextStartElement()) {
        if (name() == QLatin1String("title"))
            readTitle(bookmark);
        else if (name() == QLatin1String("desc"))
            readDescription(bookmark);
        else
            skipCurrentElement();
    }
    if (bookmark->title.isEmpty())
        bookmark->title = QObject::tr("Unknown title");
}


XbelWriter::XbelWriter()
{
    setAutoFormatting(true);
}

bool XbelWriter::write(const QString &fileName, const BookmarkNode *root)
{
    QFile file(fileName);
    if (!root || !file.open(QFile::WriteOnly))
        return false;
    return write(&file, root);
}

bool XbelWriter::write(QIODevice *device, const BookmarkNode *root)
{
    setDevice(device);

    writeStartDocument();
    writeDTD(QLatin1String("<!DOCTYPE xbel>"));
    writeStartElement(QLatin1String("xbel"));
    writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
    if (root->type() == BookmarkNode::Root) {
        for (int i = 0; i < root->children().count(); ++i)
            writeItem(root->children().at(i));
    } else {
        writeItem(root);
    }

    writeEndDocument();
    return true;
}

void XbelWriter::writeItem(const BookmarkNode *parent)
{
    switch (parent->type()) {
    case BookmarkNode::Folder:
        writeStartElement(QLatin1String("folder"));
        writeAttribute(QLatin1String("folded"), parent->expanded ? QLatin1String("no") : QLatin1String("yes"));
        writeTextElement(QLatin1String("title"), parent->title);
        for (int i = 0; i < parent->children().count(); ++i)
            writeItem(parent->children().at(i));
        writeEndElement();
        break;
    case BookmarkNode::Bookmark:
        writeStartElement(QLatin1String("bookmark"));
        if (!parent->url.isEmpty())
            writeAttribute(QLatin1String("href"), parent->url);
        writeTextElement(QLatin1String("title"), parent->title);
        if (!parent->desc.isEmpty())
            writeAttribute(QLatin1String("desc"), parent->desc);
        writeEndElement();
        break;
    case BookmarkNode::Separator:
        writeEmptyElement(QLatin1String("separator"));
        break;
    default:
        break;
    }
}

} // namespace Digikam

