/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-02
 * Description : validate / fix ui files
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "uifilevalidator.h"

// Qt includes

#include <QFile>
#include <QXmlSimpleReader>
#include <QXmlStreamWriter>

namespace Digikam
{

const QString toolbarKey("ToolBar");
const QString toolbarAttribute("name");
const QString toolbarGoodValue("mainToolBar");
const QString toolbarBadValue("ToolBar");

bool ToolbarHandler::startElement(const QString & namespaceURI, const QString & localName, const QString & qName,
                                  const QXmlAttributes & atts)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    if ( (qName == toolbarKey) && (atts.value(toolbarAttribute) != toolbarGoodValue) )
    {
        return false;
    }
    return true;
}

// --------------------------------------------------------

class UiFileValidatorPriv
{
public:

    UiFileValidatorPriv(const QString& _filename) :
        filename(_filename)
    {}

    const QString filename;
};

UiFileValidator::UiFileValidator(const QString& filename)
                   : d(new UiFileValidatorPriv(filename))
{
}

UiFileValidator::~UiFileValidator()
{
    delete d;
}

bool UiFileValidator::isReadable(QFile& file) const
{
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    return true;
}

bool UiFileValidator::isWritable(QFile& file) const
{
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }
    return true;
}

bool UiFileValidator::isValid() const
{
    QFile fi(d->filename);
    if (!isReadable(fi))
    {
        return false;
    }

    QXmlSimpleReader xmlReader;
    ToolbarHandler handler;
    xmlReader.setContentHandler(&handler);
    QXmlInputSource source(&fi);

    bool ok = xmlReader.parse(&source);
    if (!ok)
    {
        return false;
    }
    return true;
}

bool UiFileValidator::fixConfigFile()
{
    QFile fr(d->filename);
    if (!isReadable(fr))
    {
        return false;
    }
    QByteArray xmlContent = fr.readAll();

    if (xmlContent.isEmpty())
    {
        return false;
    }
    fr.close();

    QFile fw(d->filename);
    if (!isWritable(fw))
    {
        return false;
    }

    QXmlStreamReader reader(xmlContent);
    QXmlStreamWriter writer(&fw);
    while (!reader.atEnd())
    {
        reader.readNext();
        if (reader.isStartElement())
        {
            if (reader.qualifiedName() == toolbarKey)
            {
                writer.writeStartElement(toolbarKey);

                QXmlStreamAttributes attrs;
                QXmlStreamAttributes _a = reader.attributes();
                for (QXmlStreamAttributes::iterator it = _a.begin(); it != _a.end(); ++it)
                {
                    if (it->qualifiedName() == toolbarAttribute && it->value() == toolbarBadValue)
                    {
                        attrs.append(toolbarAttribute, toolbarGoodValue);
                    }
                    else
                    {
                        attrs.append((*it));
                    }
                }
                writer.writeAttributes(attrs);
                continue;
            }
        }
        writer.writeCurrentToken(reader);
    }
    fw.close();

    return true;
}

} // namespace Digikam
