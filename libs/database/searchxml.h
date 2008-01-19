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

#ifndef SEARCHXML_H
#define SEARCHXML_H

// Qt includes

#include <QString>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Digikam
{

namespace SearchXml
{

    enum Operator
    {
        And,
        Or,
        AndNot
    };

    enum Element
    {
        Group,
        GroupEnd,
        Field,
        FieldEnd,
        End
    };

    enum Relation
    {
        Equal,
        Unequal,
        Like,
        NotLike,
        LessThan,
        GreaterThan,
        LessThanOrEqual,
        GreaterThanOrEqual,
        OneOf,
        InTree,
        NotInTree
    };

}

class SearchXmlReader : public QXmlStreamReader
{
public:

    SearchXmlReader(const QString &xml);

    /** Continue parsing the document. Returns the type of the current element */
    SearchXml::Element  readNext();
    /** Returns if the current element is a group element */
    bool                isGroupElement() const;
    /** Returns if the current element is a field element */
    bool                isFieldElement() const;

    /** Returns the group operator. Only valid if the current element is a group. */
    SearchXml::Operator groupOperator() const;
    /** Returns the (optional) group caption. Only valid if the current element is a group. */
    QString             groupCaption() const;

    /** Returns the field attributes. Only valid if the current element is a field.
        fieldOperator returns the default operator if the field has not specified any. */
    SearchXml::Operator fieldOperator() const;
    QString             fieldName() const;
    SearchXml::Relation fieldRelation() const;
    /** Returns the field values. Only valid if the current element is a field. */
    QString             value();
    int                 valueToInt();
    double              valueToDouble();
    QDateTime           valueToDateTime();
    QList<int>          valueToIntList();

protected:

    SearchXml::Operator readOperator(const QString &, SearchXml::Operator) const;
    SearchXml::Relation readRelation(const QString &, SearchXml::Relation) const;
    SearchXml::Operator m_defaultFieldOperator;
};


class SearchXmlWriter : public QXmlStreamWriter
{
public:

    /**
        Note that SearchXmlWriter and SearchXmlGroupWriter rely on you
        calling the methods following the restrictions set by the documentation;
        Otherwise you will not produce the desired output.
    */

    SearchXmlWriter();

    /** Adds a group. Use the returned group writer to add fields. */
    void writeGroup();

    /** Sets the operator applied to the group as a whole "AND (field1 ... fieldn)".
        Default value is AND. */
    void setGroupOperator(SearchXml::Operator op);
    /** Sets an optional caption */
    void setGroupCaption(const QString &caption);
    /** Sets the default operator for fields in this group "(field1 AND field2 AND ... fieldn)".
        The default operator can in each field be overridden. Default value is AND. */
    void setDefaultFieldOperator(SearchXml::Operator op);

    /** Adds a new field with the given name (entity) and relation, "Rating less than ...".
        Ensure that you closed the previous field with finishField(). */
    void writeField(const QString &name, SearchXml::Relation relation);
    /** Adds an optional operator overriding the default field operator of the group. */
    void setFieldOperator(SearchXml::Operator op);
    /** Adds the value, "4" in the case of "Rating less than 4" */
    void writeValue(const QString &value);
    void writeValue(int value);
    void writeValue(double value);
    void writeValue(const QDateTime &dateTime);
    void writeValue(const QList<int> valueList);

    /** Finish writing fields. You shall call this method before continuing with the SearchXmlWriter object.
        You cannot add anymore fields after calling this. */
    void finishField();

    /** Finish the current group. Note that you will want to call this before
        writing another group if you want the group on the same level.
        You can as well add nested groups and call this to close the group afterwards. */
    void finishGroup();

    /**
     *  Finish the XML. No further group can be added after calling this.
     *  You need to call this before you can get the resulting XML from xml().
     */
    void finish();

    /** Get the created XML. The value is only valid if finish() has been called. */
    QString xml();

protected:

    void writeOperator(const QString &, SearchXml::Operator);
    void writeRelation(const QString &, SearchXml::Relation);

    QString m_xml;
};

}

#endif

