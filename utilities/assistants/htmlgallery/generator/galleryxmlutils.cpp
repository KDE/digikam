/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "galleryxmlutils.h"

namespace Digikam
{

bool XMLWriter::open(const QString& name)
{
    xmlTextWriterPtr ptr = xmlNewTextWriterFilename(name.toUtf8().constData(), 0);

    if (!ptr)
    {
        return false;
    }

    m_writer.assign(ptr);

    int rc = xmlTextWriterStartDocument(ptr, NULL, "UTF-8", NULL);

    if (rc < 0)
    {
        m_writer.assign(0);
        return false;
    }

    xmlTextWriterSetIndent(ptr, 1);

    return true;
}

XMLWriter::operator xmlTextWriterPtr() const
{
    return m_writer;
}

void XMLWriter::writeElement(const char* element, const QString& value)
{
    xmlTextWriterWriteElement(m_writer, BAD_CAST element, BAD_CAST value.toUtf8().data());
}

void XMLWriter::writeElement(const char* element, int value)
{
    writeElement(element, QString::number(value));
}

// ------------------------------------------------------

void XMLAttributeList::write(XMLWriter& writer) const
{
    Map::const_iterator it  = m_map.begin();
    Map::const_iterator end = m_map.end();

    for (; it != end ; ++it)
    {
        xmlTextWriterWriteAttribute(writer,
                                    BAD_CAST it.key().toLatin1().data(),
                                    BAD_CAST it.value().toLatin1().data());
    }
}

void XMLAttributeList::append(const QString& key, const QString& value)
{
    m_map[key] = value;
}

void XMLAttributeList::append(const QString& key, int value)
{
    m_map[key] = QString::number(value);
}

// ------------------------------------------------------

XMLElement::XMLElement(XMLWriter& writer, const QString& element, const XMLAttributeList* attributeList)
    : m_writer(writer)
{
    xmlTextWriterStartElement(writer, BAD_CAST element.toLatin1().data());

    if (attributeList)
    {
        attributeList->write(writer);
    }
}

XMLElement::~XMLElement()
{
    xmlTextWriterEndElement(m_writer);
}

} // namespace Digikam
