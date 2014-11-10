/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-02
 * Description : validate / fix ui files
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmail dot com>
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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace
{
static const QString TOOLBARKEY("ToolBar");
static const QString TOOLBARATTRIBUTE("name");
static const QString TOOLBARVALUE("mainToolBar");
}

namespace Digikam
{

class ToolbarNameHandler : public QXmlDefaultHandler
{
public:

    virtual bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName,
                              const QXmlAttributes& atts);
};

bool ToolbarNameHandler::startElement(const QString& namespaceURI, const QString& localName, const QString& qName,
                                      const QXmlAttributes& atts)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    if ( (qName == TOOLBARKEY) && (atts.value(TOOLBARATTRIBUTE) != TOOLBARVALUE) )
    {
        return false;
    }

    return true;
}

// --------------------------------------------------------

class UiFileValidatorPriv
{
public:

    explicit UiFileValidatorPriv(const QString& _filename) :
        filename(_filename)
    {}

    const QString filename;
};

// --------------------------------------------------------

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
        // We want to return true if the file is not readable,
        // because this means that we have no custom ui file and therefore can not decide if it is
        // valid or not.
        return true;
    }

    QXmlSimpleReader   xmlReader;
    ToolbarNameHandler handler;
    xmlReader.setContentHandler(&handler);
    QXmlInputSource source(&fi);

    bool ok = xmlReader.parse(&source);
    return ok;
}

bool UiFileValidator::fixConfigFile()
{
    return fixConfigFile(d->filename);
}

bool UiFileValidator::fixConfigFile(const QString& destination)
{
    QFile fw(destination);

    if (!isWritable(fw))
    {
        return false;
    }

    QByteArray result = getFixedContent();

    if (result.isEmpty())
    {
        return false;
    }

    fw.write(result);
    fw.close();

    return true;
}

QByteArray UiFileValidator::getFixedContent()
{
    QFile fr(d->filename);

    if (!isReadable(fr))
    {
        return QByteArray();
    }

    QByteArray xmlContent = fr.readAll();

    if (xmlContent.isEmpty())
    {
        return QByteArray();
    }

    fr.close();

    QByteArray xmlDest;

    QXmlStreamReader reader(xmlContent);
    QXmlStreamWriter writer(&xmlDest);

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isStartElement())
        {
            if (reader.qualifiedName() == TOOLBARKEY)
            {
                writer.writeStartElement(TOOLBARKEY);

                QXmlStreamAttributes attrs;
                QXmlStreamAttributes _a = reader.attributes();

                for (QXmlStreamAttributes::const_iterator it = _a.constBegin(); it != _a.constEnd(); ++it)
                {
                    if (it->qualifiedName() == TOOLBARATTRIBUTE && it->value() != TOOLBARVALUE)
                    {
                        attrs.append(TOOLBARATTRIBUTE, TOOLBARVALUE);
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

    return xmlDest;
}

} // namespace Digikam
