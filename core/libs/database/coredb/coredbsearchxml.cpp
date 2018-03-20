/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-09
 * Description : Core database searches XML queries manager
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "coredbsearchxml.h"

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
                m_defaultFieldOperator = readOperator(QLatin1String("fieldoperator"), SearchXml::standardFieldOperator());
                return SearchXml::Group;
            }
            else if (isFieldElement())
            {
                return SearchXml::Field;
            }
            else if (name() == QLatin1String("search"))
            {
                // root element
                return SearchXml::Search;
            }
        }
    }

    return SearchXml::End;
}

// ---------------------------------------------------------------------------------

bool SearchXmlReader::isGroupElement() const
{
    return (name() == QLatin1String("group"));
}

bool SearchXmlReader::isFieldElement() const
{
    return (name() == QLatin1String("field"));
}

SearchXml::Operator SearchXmlReader::groupOperator() const
{
    return readOperator(QLatin1String("operator"), SearchXml::standardGroupOperator());
}

QString SearchXmlReader::groupCaption() const
{
    return attributes().value(QLatin1String("caption")).toString();
}

SearchXml::Operator SearchXmlReader::defaultFieldOperator() const
{
    return m_defaultFieldOperator;
}

SearchXml::Operator SearchXmlReader::fieldOperator() const
{
    return readOperator(QLatin1String("operator"), m_defaultFieldOperator);
}

QString SearchXmlReader::fieldName() const
{
    return attributes().value(QLatin1String("name")).toString();
}

SearchXml::Relation SearchXmlReader::fieldRelation() const
{
    return readRelation(QLatin1String("relation"), SearchXml::standardFieldRelation());
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

        if (name() != QLatin1String("listitem"))
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

        if (name() != QLatin1String("listitem"))
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

        if (name() != QLatin1String("listitem"))
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

        if (name() != QLatin1String("listitem"))
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

        if (name() != QLatin1String("listitem"))
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
        if (token != QXmlStreamReader::StartElement || name() != QLatin1String("listitem"))
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
        if (token != QXmlStreamReader::StartElement || name() != QLatin1String("listitem"))
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
        if (token != QXmlStreamReader::StartElement || name() != QLatin1String("listitem"))
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

    if (op == QLatin1String("and"))
    {
        return SearchXml::And;
    }
    else if (op == QLatin1String("or"))
    {
        return SearchXml::Or;
    }
    else if (op == QLatin1String("andnot"))
    {
        return SearchXml::AndNot;
    }
    else if (op == QLatin1String("ornot"))
    {
        return SearchXml::OrNot;
    }

    return defaultOperator;
}

SearchXml::Relation SearchXmlReader::readRelation(const QString& attributeName,
                                                  SearchXml::Relation defaultRelation) const
{
    QStringRef relation = attributes().value(attributeName);

    if (relation == QLatin1String("equal"))
    {
        return SearchXml::Equal;
    }

    if (relation == QLatin1String("unequal"))
    {
        return SearchXml::Unequal;
    }
    else if (relation == QLatin1String("like"))
    {
        return SearchXml::Like;
    }
    else if (relation == QLatin1String("notlike"))
    {
        return SearchXml::NotLike;
    }
    else if (relation == QLatin1String("lessthan"))
    {
        return SearchXml::LessThan;
    }
    else if (relation == QLatin1String("greaterthan"))
    {
        return SearchXml::GreaterThan;
    }
    else if (relation == QLatin1String("lessthanequal"))
    {
        return SearchXml::LessThanOrEqual;
    }
    else if (relation == QLatin1String("greaterthanequal"))
    {
        return SearchXml::GreaterThanOrEqual;
    }
    else if (relation == QLatin1String("interval"))
    {
        return SearchXml::Interval;
    }
    else if (relation == QLatin1String("intervalopen"))
    {
        return SearchXml::IntervalOpen;
    }
    else if (relation == QLatin1String("oneof"))
    {
        return SearchXml::OneOf;
    }
    else if (relation == QLatin1String("allof"))
    {
        return SearchXml::AllOf;
    }
    else if (relation == QLatin1String("intree"))
    {
        return SearchXml::InTree;
    }
    else if (relation == QLatin1String("notintree"))
    {
        return SearchXml::NotInTree;
    }
    else if (relation == QLatin1String("near"))
    {
        return SearchXml::Near;
    }
    else if (relation == QLatin1String("inside"))
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

// ---------------------------------------------------------------------------------

SearchXmlWriter::SearchXmlWriter()
    : QXmlStreamWriter(&m_xml)
{
    writeStartDocument();
    writeStartElement(QLatin1String("search"));
}

QString SearchXmlWriter::xml() const
{
    return m_xml;
}

void SearchXmlWriter::writeGroup()
{
    writeStartElement(QLatin1String("group"));
}

void SearchXmlWriter::setGroupOperator(SearchXml::Operator op)
{
    if (op != SearchXml::Or)
    {
        writeOperator(QLatin1String("operator"), op);
    }
}

void SearchXmlWriter::setGroupCaption(const QString& caption)
{
    if (!caption.isNull())
    {
        writeAttribute(QLatin1String("caption"), caption);
    }
}

void SearchXmlWriter::setDefaultFieldOperator(SearchXml::Operator op)
{
    if (op != SearchXml::And)
    {
        writeOperator(QLatin1String("fieldoperator"), op);
    }
}

void SearchXmlWriter::writeField(const QString& name, SearchXml::Relation relation)
{
    writeStartElement(QLatin1String("field"));
    writeAttribute(QLatin1String("name"),    name);
    writeRelation(QLatin1String("relation"), relation);
}

void SearchXmlWriter::setFieldOperator(SearchXml::Operator op)
{
    writeOperator(QLatin1String("operator"), op);
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
    QString listitem(QLatin1String("listitem"));

    foreach(int i, valueList)
    {
        writeTextElement(listitem, QString::number(i));
    }
}

void SearchXmlWriter::writeValue(const QList<qlonglong>& valueList)
{
    QString listitem(QLatin1String("listitem"));

    foreach(int i, valueList)
    {
        writeTextElement(listitem, QString::number(i));
    }
}

void SearchXmlWriter::writeValue(const QList<float>& valueList, int precision)
{
    QString listitem(QLatin1String("listitem"));

    foreach(double i, valueList)
    {
        writeTextElement(listitem, QString::number(i, 'g', precision));
    }
}

void SearchXmlWriter::writeValue(const QList<double>& valueList, int precision)
{
    QString listitem(QLatin1String("listitem"));

    foreach(double i, valueList)
    {
        writeTextElement(listitem, QString::number(i, 'g', precision));
    }
}

void SearchXmlWriter::writeValue(const QList<QDateTime>& valueList)
{
    QString listitem(QLatin1String("listitem"));

    foreach(const QDateTime& dt, valueList)
    {
        writeTextElement(listitem, dt.toString(Qt::ISODate));
    }
}

void SearchXmlWriter::writeValue(const QStringList& valueList)
{
    QString listitem(QLatin1String("listitem"));

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
            writeAttribute(attributeName, QLatin1String("and"));
            break;
        case SearchXml::Or:
            writeAttribute(attributeName, QLatin1String("or"));
            break;
        case SearchXml::AndNot:
            writeAttribute(attributeName, QLatin1String("andnot"));
            break;
        case SearchXml::OrNot:
            writeAttribute(attributeName, QLatin1String("ornot"));
            break;
    }
}

void SearchXmlWriter::writeRelation(const QString& attributeName, SearchXml::Relation op)
{
    switch (op)
    {
        default:
        case SearchXml::Equal:
            writeAttribute(attributeName, QLatin1String("equal"));
            break;
        case SearchXml::Unequal:
            writeAttribute(attributeName, QLatin1String("unequal"));
            break;
        case SearchXml::Like:
            writeAttribute(attributeName, QLatin1String("like"));
            break;
        case SearchXml::NotLike:
            writeAttribute(attributeName, QLatin1String("notlike"));
            break;
        case SearchXml::LessThan:
            writeAttribute(attributeName, QLatin1String("lessthan"));
            break;
        case SearchXml::GreaterThan:
            writeAttribute(attributeName, QLatin1String("greaterthan"));
            break;
        case SearchXml::LessThanOrEqual:
            writeAttribute(attributeName, QLatin1String("lessthanequal"));
            break;
        case SearchXml::GreaterThanOrEqual:
            writeAttribute(attributeName, QLatin1String("greaterthanequal"));
            break;
        case SearchXml::Interval:
            writeAttribute(attributeName, QLatin1String("interval"));
            break;
        case SearchXml::IntervalOpen:
            writeAttribute(attributeName, QLatin1String("intervalopen"));
            break;
        case SearchXml::OneOf:
            writeAttribute(attributeName, QLatin1String("oneof"));
            break;
        case SearchXml::AllOf:
            writeAttribute(attributeName, QLatin1String("allof"));
            break;
        case SearchXml::InTree:
            writeAttribute(attributeName, QLatin1String("intree"));
            break;
        case SearchXml::NotInTree:
            writeAttribute(attributeName, QLatin1String("notintree"));
            break;
        case SearchXml::Near:
            writeAttribute(attributeName, QLatin1String("near"));
            break;
        case SearchXml::Inside:
            writeAttribute(attributeName, QLatin1String("inside"));
            break;
    }
}

QString SearchXmlWriter::keywordSearch(const QString& keyword)
{
    SearchXmlWriter writer;
    writer.writeGroup();
    writer.writeField(QLatin1String("keyword"), SearchXml::Like);
    writer.writeValue(keyword);
    writer.finishField();
    writer.finishGroup();
    writer.finish();
    return writer.xml();
}

// ---------------------------------------------------------------------------------

QStringList KeywordSearch::split(const QString& keywords)
{
    // get groups with quotation marks
    QStringList quotationMarkList = keywords.split(QLatin1Char('"'), QString::KeepEmptyParts);

    // split down to single words
    QStringList keywordList;
    int quotationMarkCount = (keywords.startsWith(QLatin1Char('"')) ? 1 : 0);

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
            keywordList << group.split(QRegExp(QLatin1String("\\s+")), QString::SkipEmptyParts);
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
        if ((*it).contains(QLatin1Char(' ')))
        {
            *it = (*it).prepend(QLatin1Char('"')).append(QLatin1Char('"'));
        }
    }

    // join in a string
    return list.join(QLatin1String(" "));
}

QString KeywordSearch::merge(const QString& previousContent, const QString& newEntry)
{
    QString ne(newEntry);
    QString pc(previousContent);

    if (ne.contains(QLatin1Char(' ')))
    {
        ne = ne.prepend(QLatin1Char('"')).append(QLatin1Char('"'));
    }

    return pc.append(QLatin1Char(' ')).append(ne);
}

// ---------------------------------------------------------------------------------

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
    if (fieldName() == QLatin1String("keyword"))
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
            if (fieldName() != QLatin1String("keyword"))
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

// ---------------------------------------------------------------------------------

KeywordSearchWriter::KeywordSearchWriter()
    : SearchXmlWriter()
{
}

QString KeywordSearchWriter::xml(const QStringList& keywordList)
{
    writeGroup();

    foreach(const QString& keyword, keywordList)
    {
        writeField(QLatin1String("keyword"), SearchXml::Like);
        writeValue(keyword);
        finishField();
    }

    finishGroup();
    finish();

    return SearchXmlWriter::xml();
}

// ---------------------------------------------------------------------------------

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
