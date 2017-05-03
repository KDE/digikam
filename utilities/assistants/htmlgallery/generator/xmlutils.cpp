/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "xmlutils.h"

namespace Digikam
{

bool XMLWriter::open(const QString& name)
{
    xmlTextWriterPtr ptr = xmlNewTextWriterFilename(name.toLocal8Bit().constData(), 0);

    if (!ptr)
    {
        return false;
    }

    mWriter.assign(ptr);

    int rc = xmlTextWriterStartDocument(ptr, NULL, "UTF-8", NULL);

    if (rc < 0)
    {
        mWriter.assign(0);
        return false;
    }

    xmlTextWriterSetIndent(ptr, 1);

    return true;
}

XMLWriter::operator xmlTextWriterPtr() const
{
    return mWriter;
}

void XMLWriter::writeElement(const char* element, const QString& value)
{
    xmlTextWriterWriteElement(mWriter, BAD_CAST element, BAD_CAST value.toUtf8().data());
}

void XMLWriter::writeElement(const char* element, int value)
{
    writeElement(element, QString::number(value));
}

// ------------------------------------------------------

void XMLAttributeList::write(XMLWriter& writer) const
{
    Map::const_iterator it  = mMap.begin();
    Map::const_iterator end = mMap.end();

    for (; it != end ; ++it)
    {
        xmlTextWriterWriteAttribute(writer,
                                    BAD_CAST it.key().toLatin1().data(),
                                    BAD_CAST it.value().toLatin1().data());
    }
}

void XMLAttributeList::append(const QString& key, const QString& value)
{
    mMap[key] = value;
}

void XMLAttributeList::append(const QString& key, int value)
{
    mMap[key] = QString::number(value);
}

// ------------------------------------------------------

XMLElement::XMLElement(XMLWriter& writer, const QString& element, const XMLAttributeList* attributeList)
    : mWriter(writer)
{
    xmlTextWriterStartElement(writer, BAD_CAST element.toLatin1().data());

    if (attributeList)
    {
        attributeList->write(writer);
    }
}

XMLElement::~XMLElement()
{
    xmlTextWriterEndElement(mWriter);
}

} // namespace Digikam
