/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-09
 * Description : Reading search XML
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "searchxml.h"

namespace Digikam
{

SearchXmlReader::SearchXmlReader(const QString& xml)
    : QXmlStreamReader(xml)
{
    m_defaultFieldOperator = SearchXml::And;

    // read in root element "search"
    readNext();
}

SearchXml::Element SearchXmlReader::readNext()
{
    while (!atEnd())
    {
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
                m_defaultFieldOperator = readOperator("fieldoperator", SearchXml::standardFieldOperator());
                return SearchXml::Group;
            }
            else if (isFieldElement())
            {
                return SearchXml::Field;
            }
            else if (name() == "search")
            {
                // root element
                return SearchXml::Search;
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
    return readOperator("operator", SearchXml::standardGroupOperator());
}

QString SearchXmlReader::groupCaption() const
{
    return attributes().value("caption").toString();
}

SearchXml::Operator SearchXmlReader::defaultFieldOperator() const
{
    return m_defaultFieldOperator;
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
    return readRelation("relation", SearchXml::standardFieldRelation());
}

QString SearchXmlReader::value()
{
    return readElementText();
}

int SearchXmlReader::valueToInt()
{
    return readElementText().toInt();
}

qlonglong SearchXmlReader::valueToLongLong()
{
    return readElementText().toLongLong();
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
        QXmlStreamReader::readNext();

        if (name() != "listitem")
        {
            break;
        }

        if (isStartElement())
        {
            list << readElementText().toInt();
        }
    }

    return list;
}

QList<qlonglong> SearchXmlReader::valueToLongLongList()
{
    QList<qlonglong> list;

    while (!atEnd())
    {
        QXmlStreamReader::readNext();

        if (name() != "listitem")
        {
            break;
        }

        if (isStartElement())
        {
            list << readElementText().toLongLong();
        }
    }

    return list;
}

QList<double> SearchXmlReader::valueToDoubleList()
{
    QList<double> list;

    while (!atEnd())
    {
        QXmlStreamReader::readNext();

        if (name() != "listitem")
        {
            break;
        }

        if (isStartElement())
        {
            list << readElementText().toDouble();
        }
    }

    return list;
}

QStringList SearchXmlReader::valueToStringList()
{
    QStringList list;

    while (!atEnd())
    {
        QXmlStreamReader::readNext();

        if (name() != "listitem")
        {
            break;
        }

        if (isStartElement())
        {
            list << readElementText();
        }
    }

    return list;
}

QList<QDateTime> SearchXmlReader::valueToDateTimeList()
{
    QList<QDateTime> list;

    while (!atEnd())
    {
        QXmlStreamReader::readNext();

        if (name() != "listitem")
        {
            break;
        }

        if (isStartElement())
        {
            list << QDateTime::fromString(readElementText(), Qt::ISODate);
        }
    }

    return list;
}

QList<int> SearchXmlReader::valueToIntOrIntList()
{
    QList<int> list;

    // poke at next token
    QXmlStreamReader::TokenType token = QXmlStreamReader::readNext();

    // Found text? Treat text as with valueToInt(), return single element list
    if (token == QXmlStreamReader::Characters)
    {
        list << text().toString().toInt();
        readNext();
        return list;
    }

    // treat as with valueToIntList()
    while (!atEnd())
    {
        if (token != QXmlStreamReader::StartElement || name() != "listitem")
        {
            break;
        }

        list << readElementText().toInt();

        token = QXmlStreamReader::readNext();
    }

    return list;
}

QList<double> SearchXmlReader::valueToDoubleOrDoubleList()
{
    QList<double> list;

    // poke at next token
    QXmlStreamReader::TokenType token = QXmlStreamReader::readNext();

    // Found text? Treat text as with valueToInt(), return single element list
    if (token == QXmlStreamReader::Characters)
    {
        list << text().toString().toDouble();
        readNext();
        return list;
    }

    // treat as with valueToIntList()
    while (!atEnd())
    {
        if (token != QXmlStreamReader::StartElement || name() != "listitem")
        {
            break;
        }

        list << readElementText().toDouble();

        token = QXmlStreamReader::readNext();
    }

    return list;
}

QList<QString> SearchXmlReader::valueToStringOrStringList()
{
    QList<QString> list;

    // poke at next token
    QXmlStreamReader::TokenType token = QXmlStreamReader::readNext();

    // Found text? Treat text as with valueToString(), return single element list
    if (token == QXmlStreamReader::Characters)
    {
        list << text().toString();
        readNext();
        return list;
    }

    // treat as with valueToStringList()
    while (!atEnd())
    {
        if (token != QXmlStreamReader::StartElement || name() != "listitem")
        {
            break;
        }

        list << readElementText();

        token = QXmlStreamReader::readNext();
    }

    return list;
}

SearchXml::Operator SearchXmlReader::readOperator(const QString& attributeName,
                                                  SearchXml::Operator defaultOperator) const
{
    QStringRef op = attributes().value(attributeName);

    if (op == "and")
    {
        return SearchXml::And;
    }
    else if (op == "or")
    {
        return SearchXml::Or;
    }
    else if (op == "andnot")
    {
        return SearchXml::AndNot;
    }
    else if (op == "ornot")
    {
        return SearchXml::OrNot;
    }

    return defaultOperator;
}

SearchXml::Relation SearchXmlReader::readRelation(const QString& attributeName,
                                                  SearchXml::Relation defaultRelation) const
{
    QStringRef relation = attributes().value(attributeName);

    if (relation == "equal")
    {
        return SearchXml::Equal;
    }

    if (relation == "unequal")
    {
        return SearchXml::Unequal;
    }
    else if (relation == "like")
    {
        return SearchXml::Like;
    }
    else if (relation == "notlike")
    {
        return SearchXml::NotLike;
    }
    else if (relation == "lessthan")
    {
        return SearchXml::LessThan;
    }
    else if (relation == "greaterthan")
    {
        return SearchXml::GreaterThan;
    }
    else if (relation == "lessthanequal")
    {
        return SearchXml::LessThanOrEqual;
    }
    else if (relation == "greaterthanequal")
    {
        return SearchXml::GreaterThanOrEqual;
    }
    else if (relation == "interval")
    {
        return SearchXml::Interval;
    }
    else if (relation == "intervalopen")
    {
        return SearchXml::IntervalOpen;
    }
    else if (relation == "oneof")
    {
        return SearchXml::OneOf;
    }
    else if (relation == "intree")
    {
        return SearchXml::InTree;
    }
    else if (relation == "notintree")
    {
        return SearchXml::NotInTree;
    }
    else if (relation == "near")
    {
        return SearchXml::Near;
    }
    else if (relation == "inside")
    {
        return SearchXml::Inside;
    }

    return defaultRelation;
}

bool SearchXmlReader::readToStartOfElement(const QString& elementName)
{
    // go to next start element
    forever
    {
        bool atStart = isStartElement();

        if (atStart)
        {
            break;
        }

        switch (QXmlStreamReader::readNext())
        {
            case StartElement:
                atStart = true;
                break;
            default:
                break;
            case EndDocument:
                return false;
        }
    }

    int stack = 1;

    forever
    {
        switch (QXmlStreamReader::readNext())
        {
            case StartElement:
            {
                if (name() == elementName)
                {
                    return true;
                }

                ++stack;
                break;
            }
            case EndElement:
            {
                if (!--stack)
                {
                    return false;
                }
                break;
            }
            case EndDocument:
                return false;
                break;
            default:
                break;
        }
    }

    return false;
}

void SearchXmlReader::readToEndOfElement()
{
    if (isStartElement())
    {
        int stack = 1;

        forever
        {
            switch (QXmlStreamReader::readNext())
            {
                case StartElement:
                    ++stack;
                    break;
                case EndElement:
                {
                    if (!--stack)
                    {
                        return;
                    }
                    break;
                }
                case EndDocument:
                    return;
                    break;
                default:
                    break;
            }
        }
    }
}

void SearchXmlReader::readToFirstField()
{
    SearchXml::Element element;
    bool hasGroup = false;

    while (!atEnd())
    {
        element = readNext();

        if (element == SearchXml::Group)
        {
            hasGroup = true;
        }
        else if (hasGroup && element == SearchXml::Field)
        {
            return;
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

QString SearchXmlWriter::xml() const
{
    return m_xml;
}

void SearchXmlWriter::writeGroup()
{
    writeStartElement("group");
}

void SearchXmlWriter::setGroupOperator(SearchXml::Operator op)
{
    if (op != SearchXml::Or)
    {
        writeOperator("operator", op);
    }
}

void SearchXmlWriter::setGroupCaption(const QString& caption)
{
    if (!caption.isNull())
    {
        writeAttribute("caption", caption);
    }
}

void SearchXmlWriter::setDefaultFieldOperator(SearchXml::Operator op)
{
    if (op != SearchXml::And)
    {
        writeOperator("fieldoperator", op);
    }
}

void SearchXmlWriter::writeField(const QString& name, SearchXml::Relation relation)
{
    writeStartElement("field");
    writeAttribute("name", name);
    writeRelation("relation", relation);
}

void SearchXmlWriter::setFieldOperator(SearchXml::Operator op)
{
    writeOperator("operator", op);
}

void SearchXmlWriter::writeValue(const QString& value)
{
    writeCharacters(value);
}

void SearchXmlWriter::writeValue(int value)
{
    writeCharacters(QString::number(value));
}

void SearchXmlWriter::writeValue(qlonglong value)
{
    writeCharacters(QString::number(value));
}

void SearchXmlWriter::writeValue(float value, int precision)
{
    writeCharacters(QString::number(value, 'g', precision));
}

void SearchXmlWriter::writeValue(double value, int precision)
{
    writeCharacters(QString::number(value, 'g', precision));
}

void SearchXmlWriter::writeValue(const QDateTime& dateTime)
{
    writeCharacters(dateTime.toString(Qt::ISODate));
}

void SearchXmlWriter::writeValue(const QList<int>& valueList)
{
    QString listitem("listitem");

    foreach(int i, valueList)
    {
        writeTextElement(listitem, QString::number(i));
    }
}

void SearchXmlWriter::writeValue(const QList<qlonglong>& valueList)
{
    QString listitem("listitem");

    foreach(int i, valueList)
    {
        writeTextElement(listitem, QString::number(i));
    }
}

void SearchXmlWriter::writeValue(const QList<float>& valueList, int precision)
{
    QString listitem("listitem");

    foreach(double i, valueList)
    {
        writeTextElement(listitem, QString::number(i, 'g', precision));
    }
}

void SearchXmlWriter::writeValue(const QList<double>& valueList, int precision)
{
    QString listitem("listitem");

    foreach(double i, valueList)
    {
        writeTextElement(listitem, QString::number(i, 'g', precision));
    }
}

void SearchXmlWriter::writeValue(const QList<QDateTime>& valueList)
{
    QString listitem("listitem");

    foreach(const QDateTime& dt, valueList)
    {
        writeTextElement(listitem, dt.toString(Qt::ISODate));
    }
}

void SearchXmlWriter::writeValue(const QStringList& valueList)
{
    QString listitem("listitem");

    foreach(const QString& str, valueList)
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

void SearchXmlWriter::writeOperator(const QString& attributeName, SearchXml::Operator op)
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
        case SearchXml::OrNot:
            writeAttribute(attributeName, "ornot");
            break;
    }
}

void SearchXmlWriter::writeRelation(const QString& attributeName, SearchXml::Relation op)
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
        case SearchXml::Interval:
            writeAttribute(attributeName, "interval");
            break;
        case SearchXml::IntervalOpen:
            writeAttribute(attributeName, "intervalopen");
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
        case SearchXml::Near:
            writeAttribute(attributeName, "near");
            break;
        case SearchXml::Inside:
            writeAttribute(attributeName, "inside");
            break;
    }
}

QString SearchXmlWriter::keywordSearch(const QString& keyword)
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

// ---------------------------------------- //

QStringList KeywordSearch::split(const QString& keywords)
{
    // get groups with quotation marks
    QStringList quotationMarkList = keywords.split('"', QString::KeepEmptyParts);

    // split down to single words
    QStringList keywordList;
    int quotationMarkCount = (keywords.startsWith('"') ? 1 : 0);
    foreach(const QString& group, quotationMarkList)
    {
        if (quotationMarkCount % 2)
        {
            // inside marks: leave as is
            if (!group.isEmpty())
            {
                keywordList << group;
            }
        }
        else
        {
            // not in quotation marks: split by whitespace
            keywordList << group.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        }

        ++quotationMarkCount;
    }
    return keywordList;
}

QString KeywordSearch::merge(const QStringList& keywordList)
{
    QStringList list(keywordList);

    // group keyword with spaces in quotation marks
    for (QStringList::iterator it = list.begin(); it != list.end(); ++it)
    {
        if ((*it).contains(' '))
        {
            *it = (*it).prepend('"').append('"');
        }
    }

    // join in a string
    return list.join(" ");
}

QString KeywordSearch::merge(const QString& previousContent, const QString& newEntry)
{
    QString ne(newEntry);
    QString pc(previousContent);

    if (ne.contains(' '))
    {
        ne = ne.prepend('"').append('"');
    }

    return pc.append(' ').append(ne);
}

// ---------------------------------------- //

KeywordSearchReader::KeywordSearchReader(const QString& xml)
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

void KeywordSearchReader::readGroup(QStringList& list)
{
    SearchXml::Element element;

    while (!atEnd())
    {
        element = readNext();

        if (element == SearchXml::Field)
        {
            QString value = readField();

            if (!value.isEmpty())
            {
                list << value;
            }
        }

        if (element == SearchXml::GroupEnd)
        {
            return;
        }
    }
}

QString KeywordSearchReader::readField()
{
    if (fieldName() == "keyword")
    {
        return value();
    }

    return QString();
}

bool KeywordSearchReader::isSimpleKeywordSearch()
{
    // Find out if this XML conforms to a simple keyword search,
    // as created with KeywordSearchWriter
    SearchXml::Element element;
    int groupCount = 0;

    while (!atEnd())
    {
        element = readNext();

        if (element == SearchXml::Group)
        {
            // only one group please
            if (++groupCount > 1)
            {
                return false;
            }

            if (!isSimpleKeywordSearchGroup())
            {
                return false;
            }
        }
    }

    return true;
}

bool KeywordSearchReader::isSimpleKeywordSearchGroup()
{
    // Find out if the current group conforms to a simple keyword search,
    // as created with KeywordSearchWriter

    if (groupOperator() != SearchXml::standardGroupOperator())
    {
        return false;
    }

    if (defaultFieldOperator() != SearchXml::standardFieldOperator())
    {
        return false;
    }

    SearchXml::Element element;

    while (!atEnd())
    {
        element = readNext();

        // subgroups not allowed
        if (element == SearchXml::Group)
        {
            return false;
        }

        // only "keyword" fields allowed
        if (element == SearchXml::Field)
        {
            if (fieldName() != "keyword")
            {
                return false;
            }

            if (fieldRelation() != SearchXml::Like)
            {
                return false;
            }

            if (fieldOperator() != SearchXml::standardFieldOperator())
            {
                return false;
            }
        }

        if (element == SearchXml::GroupEnd)
        {
            return true;
        }
    }

    return true;
}

// ---------------------------------------- //

KeywordSearchWriter::KeywordSearchWriter()
    : SearchXmlWriter()
{
}

QString KeywordSearchWriter::xml(const QStringList& keywordList)
{
    writeGroup();

    foreach(const QString& keyword, keywordList)
    {
        writeField("keyword", SearchXml::Like);
        writeValue(keyword);
        finishField();
    }

    finishGroup();
    finish();

    return SearchXmlWriter::xml();
}

// ---------------------------------------- //

SearchXmlCachingReader::SearchXmlCachingReader(const QString& xml)
    : SearchXmlReader(xml),
      m_groupOperator(SearchXml::And),
      m_fieldOperator(SearchXml::And),
      m_fieldRelation(SearchXml::Equal),
      m_readValue(false)
{
}

SearchXml::Element SearchXmlCachingReader::readNext()
{
    SearchXml::Element element = SearchXmlReader::readNext();

    if (element == SearchXml::Group)
    {
        m_groupOperator = SearchXmlReader::groupOperator();
        m_groupCaption  = SearchXmlReader::groupCaption();
    }
    else if (element == SearchXml::Field)
    {
        m_fieldOperator = SearchXmlReader::fieldOperator();
        m_fieldName     = SearchXmlReader::fieldName();
        m_fieldRelation = SearchXmlReader::fieldRelation();
        m_readValue     = false;
    }

    return element;
}

SearchXml::Operator SearchXmlCachingReader::groupOperator() const
{
    return m_groupOperator;
}

QString SearchXmlCachingReader::groupCaption() const
{
    return m_groupCaption;
}

SearchXml::Operator SearchXmlCachingReader::fieldOperator() const
{
    return m_fieldOperator;
}

QString SearchXmlCachingReader::fieldName() const
{
    return m_fieldName;
}

SearchXml::Relation SearchXmlCachingReader::fieldRelation() const
{
    return m_fieldRelation;
}

QString SearchXmlCachingReader::value()
{
    if (!m_readValue)
    {
        m_value     = SearchXmlReader::value();
        m_readValue = true;
    }

    return m_value.toString();
}

int SearchXmlCachingReader::valueToInt()
{
    if (!m_readValue)
    {
        m_value     = SearchXmlReader::valueToInt();
        m_readValue = true;
    }

    return m_value.toInt();
}

qlonglong SearchXmlCachingReader::valueToLongLong()
{
    if (!m_readValue)
    {
        m_value     = SearchXmlReader::valueToLongLong();
        m_readValue = true;
    }

    return m_value.toLongLong();
}

double SearchXmlCachingReader::valueToDouble()
{
    if (!m_readValue)
    {
        m_value     = SearchXmlReader::valueToDouble();
        m_readValue = true;
    }

    return m_value.toDouble();
}

QDateTime SearchXmlCachingReader::valueToDateTime()
{
    if (!m_readValue)
    {
        m_value     = SearchXmlReader::valueToDateTime();
        m_readValue = true;
    }

    return m_value.toDateTime();
}

QList<int> SearchXmlCachingReader::valueToIntList()
{
    // with no QVariant support for QList<int>,
    // we convert here from string list (equivalent result)
    QStringList list = valueToStringList();
    QList<int> intList;

    foreach(const QString& s, list)
    {
        intList << s.toInt();
    }

    return intList;
}

QList<qlonglong> SearchXmlCachingReader::valueToLongLongList()
{
    // with no QVariant support for QList<qlonglong>,
    // we convert here from string list (equivalent result)
    QStringList list = valueToStringList();
    QList<qlonglong> qlonglongList;

    foreach(const QString& s, list)
    {
        qlonglongList << s.toLongLong();
    }

    return qlonglongList;
}

QList<double> SearchXmlCachingReader::valueToDoubleList()
{
    // with no QVariant support for QList<double>,
    // we convert here from string list (equivalent result)
    QStringList list = valueToStringList();
    QList<double> doubleList;

    foreach(const QString& s, list)
    {
        doubleList << s.toDouble();
    }

    return doubleList;
}

QList<QDateTime> SearchXmlCachingReader::valueToDateTimeList()
{
    // with no QVariant support for QList<QDateTime>,
    // we convert here from string list (equivalent result)
    QStringList list = valueToStringList();
    QList<QDateTime> doubleList;

    foreach(const QString& s, list)
    {
        doubleList << QDateTime::fromString(s, Qt::ISODate);
    }

    return doubleList;
}

QStringList SearchXmlCachingReader::valueToStringList()
{
    if (!m_readValue)
    {
        m_value     = SearchXmlReader::valueToStringList();
        m_readValue = true;
    }

    return m_value.toStringList();
}

QList<int> SearchXmlCachingReader::valueToIntOrIntList()
{
    if (!m_readValue)
    {
        QList<int> intList = SearchXmlReader::valueToIntOrIntList();
        QList<QVariant> varList;

        foreach(int v, intList)
        {
            varList << v;
        }

        m_value     = varList;
        m_readValue = true;

        return intList;
    }

    QList<int> intList;
    QList<QVariant> varList = m_value.toList();

    foreach(const QVariant& var, varList)
    {
        intList << var.toInt();
    }

    return intList;
}

QList<double> SearchXmlCachingReader::valueToDoubleOrDoubleList()
{
    if (!m_readValue)
    {
        QList<double> doubleList = SearchXmlReader::valueToDoubleOrDoubleList();
        QList<QVariant> varList;

        foreach(double v, doubleList)
        {
            varList << v;
        }

        m_value     = varList;
        m_readValue = true;
        return doubleList;
    }

    QList<double> doubleList;
    QList<QVariant> varList = m_value.toList();

    foreach(const QVariant& var, varList)
    {
        doubleList << var.toDouble();
    }

    return doubleList;
}

QList<QString> SearchXmlCachingReader::valueToStringOrStringList()
{
    if (!m_readValue)
    {
        QList<QString> QStringList = SearchXmlReader::valueToStringOrStringList();
        QList<QVariant> varList;

        foreach(const QString& v, QStringList)
        {
            varList << v;
        }

        m_value     = varList;
        m_readValue = true;
        return QStringList;
    }

    QList<QString> QStringList;
    QList<QVariant> varList = m_value.toList();

    foreach(const QVariant& var, varList)
    {
        QStringList << var.toString();
    }

    return QStringList;
}

} // namespace Digikam
