/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-09
 * Description : Reading search XML
 * 
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "searchxml.h"

namespace Digikam
{

SearchXmlReader::SearchXmlReader(const QString &xml)
    : QXmlStreamReader(xml)
{
    m_defaultFieldOperator = SearchXml::And;
}

SearchXml::Element SearchXmlReader::readNext()
{
    while (!atEnd()) {

        QXmlStreamReader::readNext();

        if (isEndElement())
        {
            if (isGroupElement())
            {
                return SearchXml::GroupEnd;
            }
            else if (isFieldElement())
            {
                return SearchXml::FieldEnd;
            }
        }

        if (isStartElement())
        {
            if (isGroupElement())
            {
                // get possible default operator
                m_defaultFieldOperator = readOperator("fieldoperator", SearchXml::And);
                return SearchXml::Group;
            }
            else if (isFieldElement())
            {
                return SearchXml::Field;
            }
            else if (name() == "search")
            {
                // root element, skip
                continue;
            }
        }
    }

    return SearchXml::End;
}

bool SearchXmlReader::isGroupElement() const
{
    return name() == "group";
}

bool SearchXmlReader::isFieldElement() const
{
    return name() == "field";
}

SearchXml::Operator SearchXmlReader::groupOperator() const
{
    return readOperator("operator", SearchXml::Or);
}

QString SearchXmlReader::groupCaption() const
{
    return attributes().value("caption").toString();
}

SearchXml::Operator SearchXmlReader::fieldOperator() const
{
    return readOperator("operator", m_defaultFieldOperator);
}

QString SearchXmlReader::fieldName() const
{
    return attributes().value("name").toString();
}

SearchXml::Relation SearchXmlReader::fieldRelation() const
{
    return readRelation("relation", SearchXml::Equal);
}

QString SearchXmlReader::value()
{
    return readElementText();
}

int SearchXmlReader::valueToInt()
{
    return readElementText().toInt();
}

double SearchXmlReader::valueToDouble()
{
    return readElementText().toDouble();
}

QDateTime SearchXmlReader::valueToDateTime()
{
    return QDateTime::fromString(readElementText(), Qt::ISODate);
}

QList<int> SearchXmlReader::valueToIntList()
{
    QList<int> list;

    while (!atEnd())
    {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement())
            list << readElementText().toInt();
    }

    return list;
}

QStringList SearchXmlReader::valueToStringList()
{
    QStringList list;

    while (!atEnd())
    {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement())
            list << readElementText();
    }

    return list;
}

SearchXml::Operator SearchXmlReader::readOperator(const QString &attributeName,
                                                  SearchXml::Operator defaultOperator) const
{
    QStringRef op = attributes().value(attributeName);
    if (op == "and")
        return SearchXml::And;
    else if (op == "or")
        return SearchXml::Or;
    else if (op == "andnot")
        return SearchXml::AndNot;

    return defaultOperator;
}

SearchXml::Relation SearchXmlReader::readRelation(const QString &attributeName,
                                                  SearchXml::Relation defaultRelation) const
{
    QStringRef relation = attributes().value(attributeName);
    if (relation == "equal")
        return SearchXml::Equal;
    if (relation == "unequal")
        return SearchXml::Unequal;
    else if (relation == "like")
        return SearchXml::Like;
    else if (relation == "notlike")
        return SearchXml::NotLike;
    else if (relation == "lessthan")
        return SearchXml::LessThan;
    else if (relation == "greaterthan")
        return SearchXml::GreaterThan;
    else if (relation == "lessthanequal")
        return SearchXml::LessThanOrEqual;
    else if (relation == "greaterthanequal")
        return SearchXml::GreaterThanOrEqual;
    else if (relation == "oneof")
        return SearchXml::OneOf;
    else if (relation == "intree")
        return SearchXml::InTree;
    else if (relation == "notintree")
        return SearchXml::NotInTree;

    return defaultRelation;
}

void SearchXmlReader::readToEndOfElement()
{
    int stack = 1;
    if (isStartElement()) {
        QString result;
        forever {
            switch (readNext()) {
                case StartElement:
                    stack++;
                case EndElement:
                    if (!--stack)
                        return;
                case EndDocument:
                    return;
                default:
                    break;
            }
        }
    }
}


// ---------------------------------------- //

SearchXmlWriter::SearchXmlWriter()
    : QXmlStreamWriter(&m_xml)
{
    writeStartDocument();
    writeStartElement("search");
}

QString SearchXmlWriter::xml()
{
    return m_xml;
}

void SearchXmlWriter::writeGroup()
{
    writeStartElement("group");
}

void SearchXmlWriter::setGroupOperator(SearchXml::Operator op)
{
    writeOperator("operator", op);
}

void SearchXmlWriter::setGroupCaption(const QString &caption)
{
    writeAttribute("caption", caption);
}

void SearchXmlWriter::setDefaultFieldOperator(SearchXml::Operator op)
{
    writeOperator("fieldoperator", op);
}

void SearchXmlWriter::writeField(const QString &name, SearchXml::Relation relation)
{
    writeStartElement("field");
    writeAttribute("name", name);
    writeRelation("relation", relation);
}

void SearchXmlWriter::setFieldOperator(SearchXml::Operator op)
{
    writeOperator("operator", op);
}

void SearchXmlWriter::writeValue(const QString &value)
{
    writeCharacters(value);
}

void SearchXmlWriter::writeValue(int value)
{
    writeCharacters(QString::number(value));
}

void SearchXmlWriter::writeValue(double value)
{
    writeCharacters(QString::number(value));
}

void SearchXmlWriter::writeValue(const QDateTime &dateTime)
{
    writeCharacters(dateTime.toString(Qt::ISODate));
}

void SearchXmlWriter::writeValue(const QList<int> valueList)
{
    QString listitem("listitem");
    foreach(int i, valueList)
    {
        writeTextElement(listitem, QString::number(i));
    }
}

void SearchXmlWriter::writeValue(const QStringList valueList)
{
    QString listitem("listitem");
    foreach(QString str, valueList)
    {
        writeTextElement(listitem, str);
    }
}

void SearchXmlWriter::finishField()
{
    writeEndElement();
}

void SearchXmlWriter::finishGroup()
{
    writeEndElement();
}

void SearchXmlWriter::finish()
{
    writeEndDocument();
}

void SearchXmlWriter::writeOperator(const QString &attributeName, SearchXml::Operator op)
{
    switch (op)
    {
        default:
        case SearchXml::And:
            writeAttribute(attributeName, "and");
            break;
        case SearchXml::Or:
            writeAttribute(attributeName, "or");
            break;
        case SearchXml::AndNot:
            writeAttribute(attributeName, "andnot");
            break;
    }
}

void SearchXmlWriter::writeRelation(const QString &attributeName, SearchXml::Relation op)
{
    switch (op)
    {
        default:
        case SearchXml::Equal:
            writeAttribute(attributeName, "equal");
            break;
        case SearchXml::Unequal:
            writeAttribute(attributeName, "unequal");
            break;
        case SearchXml::Like:
            writeAttribute(attributeName, "like");
            break;
        case SearchXml::NotLike:
            writeAttribute(attributeName, "notlike");
            break;
        case SearchXml::LessThan:
            writeAttribute(attributeName, "lessthan");
            break;
        case SearchXml::GreaterThan:
            writeAttribute(attributeName, "greaterthan");
            break;
        case SearchXml::LessThanOrEqual:
            writeAttribute(attributeName, "lessthanequal");
            break;
        case SearchXml::GreaterThanOrEqual:
            writeAttribute(attributeName, "greaterthanequal");
            break;
        case SearchXml::OneOf:
            writeAttribute(attributeName, "oneof");
            break;
        case SearchXml::InTree:
            writeAttribute(attributeName, "intree");
            break;
        case SearchXml::NotInTree:
            writeAttribute(attributeName, "notintree");
            break;
    }
}

QString SearchXmlWriter::keywordSearch(const QString &keyword)
{
    SearchXmlWriter writer;
    writer.writeGroup();
    writer.writeField("keyword", SearchXml::Like);
    writer.writeValue(keyword);
    writer.finishField();
    writer.finishGroup();
    writer.finish();
    return writer.xml();
}



KeywordSearchReader::KeywordSearchReader(const QString &xml)
    : SearchXmlReader(xml)
{
}

QStringList KeywordSearchReader::keywords()
{
    QStringList list;

    SearchXml::Element element;
    while (!atEnd())
    {
        element = readNext();

        if (element == SearchXml::Group)
        {
            readGroup(list);
        }
    }

    return list;
}

void KeywordSearchReader::readGroup(QStringList &list)
{
    SearchXml::Element element;
    while (!atEnd())
    {
        element = readNext();

        if (element == SearchXml::Field)
        {
            QString value = readField();
            if (!value.isEmpty())
                list << value;
        }

        if (element == SearchXml::GroupEnd)
            return;
    }
}

QString KeywordSearchReader::readField()
{
    if (fieldName() == "keyword")
        return value();
    return QString();
}


KeywordSearchWriter::KeywordSearchWriter()
    : SearchXmlWriter()
{
}

QString KeywordSearchWriter::xml(const QStringList &keywordList)
{
    writeGroup();
    foreach (QString keyword, keywordList)
    {
        writeField("keyword", SearchXml::Like);
        writeValue(keyword);
        finishField();
    }
    finishGroup();
    finish();

    return SearchXmlWriter::xml();
}



}

