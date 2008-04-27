/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : Building complex database SQL queries from search descriptions
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
 * ============================================================ */

// Qt includes.

#include <QFile>
#include <QDir>
#include <QMap>

// KDE includes.

#include <kglobal.h>
#include <klocale.h>
#include <kcalendarsystem.h>
#include <kcomponentdata.h>
#include <kmimetype.h>

// Local includes.

#include "albumdb.h"
#include "ddebug.h"
#include "imagequerybuilder.h"

namespace Digikam
{

ImageQueryBuilder::ImageQueryBuilder()
{
    // build a lookup table for month names
    const KCalendarSystem* cal = KGlobal::locale()->calendar();
    for (int i=1; i<=12; ++i)
    {
        m_shortMonths[i-1] = cal->monthName(i, 2000, KCalendarSystem::ShortName).toLower();
        m_longMonths[i-1]  = cal->monthName(i, 2000, KCalendarSystem::LongName).toLower();
    }
}

QString ImageQueryBuilder::buildQuery(const QString &q, QList<QVariant> *boundValues) const
{
    // Handle legacy query descriptions
    if (q.startsWith("digikamsearch:"))
        return buildQueryFromUrl(KUrl(q), boundValues);
    else
        return buildQueryFromXml(q, boundValues);
}

QString ImageQueryBuilder::buildQueryFromXml(const QString &xml, QList<QVariant> *boundValues) const
{
    SearchXmlCachingReader reader(xml);
    QString sql;
    bool firstGroup = true;

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isEndElement())
            continue;

        if (reader.isGroupElement())
        {
            addSqlOperator(sql, reader.groupOperator(), firstGroup);
            if (firstGroup)
                firstGroup = false;

            buildGroup(sql, reader, boundValues);
        }
    }

    return sql;
}

void ImageQueryBuilder::buildGroup(QString &sql, SearchXmlCachingReader &reader, QList<QVariant> *boundValues) const
{
    sql += " (";

    bool firstField = true;
    bool hasContent = false;
    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isEndElement())
            break;

        // subgroup
        if (reader.isGroupElement())
        {
            hasContent = true;
            addSqlOperator(sql, reader.groupOperator(), firstField);
            if (firstField)
                firstField = false;

            buildGroup(sql, reader, boundValues);
        }

        if (reader.isFieldElement())
        {
            hasContent = true;
            addSqlOperator(sql, reader.fieldOperator(), firstField);
            if (firstField)
                firstField = false;

            buildField(sql, reader, reader.fieldName(), boundValues);
        }
    }

    if (!hasContent)
        sql += " 0 ";

    sql += ") ";
}

class FieldQueryBuilder
{
public:

    FieldQueryBuilder(QString &sql, SearchXmlCachingReader &reader,
                      QList<QVariant> *boundValues, SearchXml::Relation relation)
       : sql(sql), reader(reader), boundValues(boundValues), relation(relation)
    {
    }

    QString &sql;
    SearchXmlCachingReader &reader;
    QList<QVariant> *boundValues;
    SearchXml::Relation relation;

    inline QString prepareForLike(const QString &str)
    {
        if (relation == SearchXml::Like || relation == SearchXml::NotLike)
            return "%" + str + "%";
        else return str;
    }

    void addIntField(const QString &name)
    {
        sql += " (" + name + ' ';
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?) ";
        *boundValues << reader.valueToInt();
    }

    void addDoubleField(const QString &name)
    {
        sql += " (" + name + ' ';
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?) ";
        *boundValues << reader.valueToDouble();
    }

    void addStringField(const QString &name)
    {
        sql += " (" + name + ' ';
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?) ";
        *boundValues << prepareForLike(reader.value());
    }

    void addDateField(const QString &name)
    {
        if (relation == SearchXml::Equal)
        {
            // special case: split in < and >
            QDateTime date = QDateTime::fromString(reader.value(), Qt::ISODate);
            if (!date.isValid())
                return;
            QDateTime startDate, endDate;
            if (date.time() == QTime(0,0,0,0))
            {
                // day precision
                QDate startDate, endDate;
                startDate = date.date().addDays(-1);
                endDate = date.date().addDays(1);
                *boundValues << startDate.toString(Qt::ISODate)
                             << endDate.toString(Qt::ISODate);
            }
            else
            {
                // sub-day precision
                QDateTime startDate, endDate;
                int diff;
                if (date.time().hour() == 0)
                    diff = 3600;
                else if (date.time().minute() == 0)
                    diff = 60;
                else
                    diff = 1;
                // we spare microseconds for the future

                startDate = date.addSecs(-diff);
                endDate = date.addSecs(diff);
                *boundValues << startDate.toString(Qt::ISODate)
                             << endDate.toString(Qt::ISODate);
            }

            sql += " (" + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql, SearchXml::GreaterThan);
            sql += " ? AND " + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql, SearchXml::LessThan);
            sql += " ?) ";
        }
        else
        {
            sql += " (" + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql, relation);
            sql += " ?) ";
            *boundValues << reader.value();
        }
    }

    void addChoiceIntField(const QString &name)
    {
        if (relation == SearchXml::OneOf)
        {
            QList<int> values = reader.valueToIntList();
            bool searchForNull = values.removeAll(-1);
            sql += " (" + name + " IN (";
            AlbumDB::addBoundValuePlaceholders(sql, values.size());
            if (searchForNull)
                sql += ") OR " + name + " IS NULL";
            else
                sql += ") ";
            foreach (int v, values)
                *boundValues << v;
        }
        else
        {
            addIntField(name);
        }
    }

    void addIntBitmaskField(const QString &name)
    {
        if (relation == SearchXml::OneOf)
        {
            QList<int> values = reader.valueToIntList();
            bool searchForNull = values.removeAll(-1);
            sql += "( ";
            bool first = true;
            for (int i=0; i<values.size(); i++)
            {
                if (!first)
                    sql += "OR ";
                first = false;
                sql += name + " & ? ";
            }
            if (searchForNull)
                sql += "OR " + name + " IS NULL ";

            foreach (int v, values)
                *boundValues << v;
        }
        else
        {
            if (relation == SearchXml::Equal)
                sql += " (" + name + " & " + " ?) ";
            else
                sql += " (NOT " + name + " & " + " ?) ";
            *boundValues << reader.valueToDouble();
        }
    }

    void addChoiceStringField(const QString &name)
    {
        if (relation == SearchXml::OneOf)
        {
            QStringList values = reader.valueToStringList();
            sql += " (" + name + " IN (";
            AlbumDB::addBoundValuePlaceholders(sql, values.size());
            sql += ") ";
            *boundValues << values;
        }
        else
        {
            addStringField(name);
        }
    }
};


void ImageQueryBuilder::buildField(QString &sql, SearchXmlCachingReader &reader, const QString &name, QList<QVariant> *boundValues) const
{
    SearchXml::Relation relation = reader.fieldRelation();
    FieldQueryBuilder fieldQuery(sql, reader, boundValues, relation);
    if (name == "albumid")
    {
        fieldQuery.addIntField("Images.id");
    }
    else if (name == "albumname")
    {
        fieldQuery.addStringField("Albums.relativePath");
    }
    else if (name == "albumcaption")
    {
        fieldQuery.addStringField("Albums.caption");
    }
    else if (name == "albumcollection")
    {
        fieldQuery.addStringField("Albums.collection");
    }
    else if (name == "tagid")
    {
        if (relation == SearchXml::Equal)
        {
            sql += " (Images.id IN "
                    "   (SELECT imageid FROM ImageTags "
                    "    WHERE tagid = ?)) ";
            *boundValues << reader.valueToInt();
        }
        else if (relation == SearchXml::Unequal)
        {
            sql += " (Images.id NOT IN "
                   "   (SELECT imageid FROM ImageTags "
                   "    WHERE tagid = ?)) ";
            *boundValues << reader.valueToInt();
        }
        else if (relation == SearchXml::InTree)
        {
            sql += " (Images.id IN "
                   "   (SELECT ImageTags.imageid FROM ImageTags JOIN TagsTree on ImageTags.tagid = TagsTree.id "
                   "    WHERE TagsTree.pid = ? OR ImageTags.tagid = ? )) ";
            *boundValues << reader.valueToInt() << reader.valueToInt();
        }
        else if (relation == SearchXml::NotInTree)
        {
            sql += " (Images.id NOT IN "
                   "   (SELECT ImageTags.imageid FROM ImageTags JOIN TagsTree on ImageTags.tagid = TagsTree.id "
                   "    WHERE TagsTree.pid = ? OR ImageTags.tagid = ? )) ";
            *boundValues << reader.valueToInt() << reader.valueToInt();
        }
    }
    else if (name == "tagname")
    {
        QString tagname = "%" + reader.value() + "%";
        if (relation == SearchXml::Equal || relation == SearchXml::Like)
        {
            sql += " (Images.id IN "
                   "   (SELECT imageid FROM ImageTags "
                   "    WHERE tagid IN "
                   "   (SELECT id FROM Tags WHERE name LIKE ?))) ";
            *boundValues << tagname;
        }
        else if (relation == SearchXml::Unequal || relation == SearchXml::NotLike)
        {
            sql += " (Images.id NOT IN "
                   "   (SELECT imageid FROM ImageTags "
                   "    WHERE tagid IN "
                   "   (SELECT id FROM Tags WHERE name LIKE ?))) ";
            *boundValues << tagname;
        }
        else if (relation == SearchXml::InTree)
        {
            sql += " (Images.id IN "
                   "   (SELECT ImageTags.imageid FROM ImageTags JOIN TagsTree on ImageTags.tagid = TagsTree.id "
                   "    WHERE TagsTree.pid = (SELECT id FROM Tags WHERE name LIKE ?) "
                   "    or ImageTags.tagid = (SELECT id FROM Tags WHERE name LIKE ?) )) ";
            *boundValues << tagname << tagname;
        }
        else if (relation == SearchXml::NotInTree)
        {
            sql += " (Images.id NOT IN "
                   "   (SELECT ImageTags.imageid FROM ImageTags JOIN TagsTree on ImageTags.tagid = TagsTree.id "
                   "    WHERE TagsTree.pid = (SELECT id FROM Tags WHERE name LIKE ?) "
                   "    or ImageTags.tagid = (SELECT id FROM Tags WHERE name LIKE ?) )) ";
            *boundValues << tagname << tagname;
        }
    }
    else if (name == "notag")
    {
        sql += " (Images.id NOT IN "
               "   (SELECT imageid FROM ImageTags)) ";
    }
    else if (name == "filename")
    {
        fieldQuery.addStringField("Images.name");
    }
    else if (name == "modificationdate")
    {
        fieldQuery.addDateField("Images.modificationDate");
    }
    else if (name == "filesize")
    {
        fieldQuery.addIntField("Images.fileSize");
    }

    else if (name == "rating")
    {
        fieldQuery.addIntField("ImageInformation.rating");
    }
    else if (name == "creationdate")
    {
        fieldQuery.addDateField("ImageInformation.creationDate");
    }
    else if (name == "digitizationdate")
    {
        fieldQuery.addDateField("ImageInformation.digitizationDate");
    }
    else if (name == "orientation")
    {
        fieldQuery.addChoiceIntField("ImageInformation.orientation");
    }
    else if (name == "width")
    {
        fieldQuery.addIntField("ImageInformation.width");
    }
    else if (name == "height")
    {
        fieldQuery.addIntField("ImageInformation.height");
    }
    else if (name == "pixels")
    {
        fieldQuery.addIntField("(ImageInformation.width * ImageInformation.height)");
    }
    else if (name == "format")
    {
        fieldQuery.addChoiceStringField("ImageInformation.format");
    }
    else if (name == "colordepth")
    {
        fieldQuery.addIntField("ImageInformation.colorDepth");
    }
    else if (name == "colormodel")
    {
        fieldQuery.addIntField("ImageInformation.colorModel");
    }

    else if (name == "make")
    {
        fieldQuery.addStringField("ImageMetadata.make");
    }
    else if (name == "model")
    {
        fieldQuery.addStringField("ImageMetadata.model");
    }
    else if (name == "aperture")
    {
        fieldQuery.addDoubleField("ImageMetadata.aperture");
    }
    else if (name == "focallength")
    {
        fieldQuery.addDoubleField("ImageMetadata.focalLength");
    }
    else if (name == "focallength35")
    {
        fieldQuery.addDoubleField("ImageMetadata.focalLength35");
    }
    else if (name == "exposuretime")
    {
        fieldQuery.addDoubleField("ImageMetadata.exposureTime");
    }
    else if (name == "exposureprogram")
    {
        fieldQuery.addChoiceIntField("ImageMetadata.exposureProgram");
    }
    else if (name == "exposuremode")
    {
        fieldQuery.addChoiceIntField("ImageMetadata.exposureMode");
    }
    else if (name == "sensitivity")
    {
        fieldQuery.addIntField("ImageMetadata.sensitivity");
    }
    else if (name == "flashmode")
    {
        fieldQuery.addIntBitmaskField("ImageMetadata.flash");
    }
    else if (name == "whitebalance")
    {
        fieldQuery.addChoiceIntField("ImageMetadata.whiteBalance");
    }
    else if (name == "whitebalancecolortemperature")
    {
        fieldQuery.addIntField("ImageMetadata.whiteBalanceColorTemperature");
    }
    else if (name == "meteringmode")
    {
        fieldQuery.addChoiceIntField("ImageMetadata.meteringMode");
    }
    else if (name == "subjectdistance")
    {
        fieldQuery.addDoubleField("ImageMetadata.subjectDistance");
    }
    else if (name == "subjectdistancecategory")
    {
        fieldQuery.addChoiceIntField("ImageMetadata.subjectDistanceCategory");
    }

    else if (name == "latitude")
    {
        fieldQuery.addDoubleField("ImagePositions.latitudeNumber");
    }
    else if (name == "longitude")
    {
        fieldQuery.addDoubleField("ImagePositions.longitudeNumber");
    }
    else if (name == "altitude")
    {
        fieldQuery.addDoubleField("ImagePositions.altitude");
    }
    else if (name == "positionorientation")
    {
        fieldQuery.addDoubleField("ImagePositions.orientation");
    }
    else if (name == "positiontilt")
    {
        fieldQuery.addDoubleField("ImagePositions.tilt");
    }
    else if (name == "positionroll")
    {
        fieldQuery.addDoubleField("ImagePositions.roll");
    }
    else if (name == "positiondescription")
    {
        fieldQuery.addStringField("ImagePositions.description");
    }
    else if (name == "nogps")
    {
        sql += " (ImagePositions.latitudeNumber IS NULL AND ImagePositions.longitudeNumber IS NULL) ";
    }

    else if (name == "comment")
    {
        sql += " (Images.id IN "
               " (SELECT imageid FROM ImageComments "
               "  WHERE type=? AND comment ";
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?)) ";
        *boundValues << DatabaseComment::Comment << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == "commentauthor")
    {
        sql += " (Images.id IN "
               " (SELECT imageid FROM ImageComments "
               "  WHERE type=? AND author ";
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?)) ";
        *boundValues << DatabaseComment::Comment << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == "headline")
    {
        sql += " (Images.id IN "
               " (SELECT imageid FROM ImageComments "
               "  WHERE type=? AND comment ";
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?)) ";
        *boundValues << DatabaseComment::Headline << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == "title")
    {
        sql += " (Images.id IN "
               " (SELECT imageid FROM ImageComments "
               "  WHERE type=? AND comment ";
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?)) ";
        *boundValues << DatabaseComment::Comment << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == "keyword")
    {
        // keyword is the common search in the text fields

        addSqlOperator(sql, SearchXml::Or, true);
        buildField(sql, reader, "albumname", boundValues);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "filename", boundValues);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "tagname", boundValues);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "albumcaption", boundValues);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "albumcollection", boundValues);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "comment", boundValues);
    }
}

void ImageQueryBuilder::addSqlOperator(QString &sql, SearchXml::Operator op, bool isFirst)
{
    if (isFirst)
    {
        if (op == SearchXml::AndNot)
            sql += "NOT";
        return;
    }

    switch (op)
    {
        default:
        case SearchXml::And:
            sql += "AND";
            break;
        case SearchXml::Or:
            sql += "OR";
            break;
        case SearchXml::AndNot:
            sql += "AND NOT";
            break;
    }
}

void ImageQueryBuilder::addSqlRelation(QString &sql, SearchXml::Relation rel)
{
    switch (rel)
    {
        default:
        case SearchXml::Equal:
            sql += "=";
            break;
        case SearchXml::Unequal:
            sql += "<>";
            break;
        case SearchXml::Like:
            sql += "LIKE";
            break;
        case SearchXml::NotLike:
            sql += "NOT LIKE";
            break;
        case SearchXml::LessThan:
            sql += "<";
            break;
        case SearchXml::GreaterThan:
            sql += ">";
            break;
        case SearchXml::LessThanOrEqual:
            sql += "<=";
            break;
        case SearchXml::GreaterThanOrEqual:
            sql += ">=";
            break;
        case SearchXml::OneOf:
            sql += "IN";
            break;
    }
}


// ----------- Legacy query description handling -------------- //

class RuleTypeForConversion
{
    public:

        RuleTypeForConversion()
            : op(SearchXml::Equal) {}

        QString             key;
        SearchXml::Relation op;
        QString             val;
};

QString ImageQueryBuilder::convertFromUrlToXml(const KUrl& url) const
{
    int  count = url.queryItem("count").toInt();
    if (count <= 0)
        return QString();

    QMap<int, RuleTypeForConversion> rulesMap;

    for (int i=1; i<=count; i++)
    {
        RuleTypeForConversion rule;

        QString key = url.queryItem(QString::number(i) + ".key").toLower();
        QString op  = url.queryItem(QString::number(i) + ".op").toLower();

        if (key == "album")
        {
            rule.key = "albumid";
        }
        else if (key == "imagename")
        {
            rule.key = "filename";
        }
        else if (key == "imagecaption")
        {
            rule.key = "comment";
        }
        else if (key == "imagedate")
        {
            rule.key = "creationdate";
        }
        else if (key == "tag")
        {
            rule.key = "tagid";
        }
        else
        {
            // other field names did not change:
            // albumname, albumcaption, albumcollection, tagname, keyword, rating
            rule.key = key;
        }

        if (op == "eq")
            rule.op = SearchXml::Equal;
        else if (op == "ne")
            rule.op = SearchXml::Unequal;
        else if (op == "lt")
            rule.op = SearchXml::LessThan;
        else if (op == "lte")
            rule.op = SearchXml::LessThanOrEqual;
        else if (op == "gt")
            rule.op = SearchXml::GreaterThan;
        else if (op == "gte")
            rule.op = SearchXml::GreaterThanOrEqual;
        else if (op == "like")
        {
            if (key == "tag")
                rule.op = SearchXml::InTree;
            else
                rule.op = SearchXml::Like;
        }
        else if (op == "nlike")
        {
            if (key == "tag")
                rule.op = SearchXml::NotInTree;
            else
                rule.op = SearchXml::NotLike;
        }

        rule.val = url.queryItem(QString::number(i) + ".val");

        rulesMap.insert(i, rule);
    }

    SearchXmlWriter writer;

    writer.writeGroup();

    QStringList strList = url.path().split(' ', QString::SkipEmptyParts);
    for ( QStringList::Iterator it = strList.begin(); it != strList.end(); ++it )
    {
        bool ok;
        int  num = (*it).toInt(&ok);
        if (ok)
        {
            RuleTypeForConversion rule = rulesMap[num];
            writer.writeField(rule.key, rule.op);
            writer.writeValue(rule.val);
            writer.finishField();
        }
        else
        {
            QString expr = (*it).trimmed();
            if (expr == "AND")
            {
                // add another field
            }
            else if (expr == "OR")
            {
                // open a new group
                writer.finishGroup();
                writer.writeGroup();
                writer.setGroupOperator(SearchXml::Or);
            }
            else if (expr == "(")
            {
                // open a subgroup
                writer.writeGroup();
            }
            else if (expr == ")")
            {
                writer.finishGroup();
            }
        }
    }
    writer.finishGroup();
    writer.finish();

    return writer.xml();
}


enum SKey
{
    ALBUM = 0,
    ALBUMNAME,
    ALBUMCAPTION,
    ALBUMCOLLECTION,
    TAG,
    TAGNAME,
    IMAGENAME,
    IMAGECAPTION,
    IMAGEDATE,
    KEYWORD,
    RATING
};

enum SOperator
{
    EQ = 0,
    NE,
    LT,
    GT,
    LIKE,
    NLIKE,
    LTE,
    GTE
};

class RuleType
{
    public:

        SKey      key;
        SOperator op;
        QString   val;
};

class SubQueryBuilder
{
public:

    QString build(enum SKey key, enum SOperator op,
                  const QString& passedVal, QList<QVariant> *boundValues) const;
};

QString ImageQueryBuilder::buildQueryFromUrl(const KUrl& url, QList<QVariant> *boundValues) const
{
    int  count = url.queryItem("count").toInt();
    if (count <= 0)
        return QString();

    QMap<int, RuleType> rulesMap;

    for (int i=1; i<=count; i++)
    {
        RuleType rule;

        QString key = url.queryItem(QString::number(i) + ".key").toLower();
        QString op  = url.queryItem(QString::number(i) + ".op").toLower();

        if (key == "album")
        {
            rule.key = ALBUM;
        }
        else if (key == "albumname")
        {
            rule.key = ALBUMNAME;
        }
        else if (key == "albumcaption")
        {
            rule.key = ALBUMCAPTION;
        }
        else if (key == "albumcollection")
        {
            rule.key = ALBUMCOLLECTION;
        }
        else if (key == "imagename")
        {
            rule.key = IMAGENAME;
        }
        else if (key == "imagecaption")
        {
            rule.key = IMAGECAPTION;
        }
        else if (key == "imagedate")
        {
            rule.key = IMAGEDATE;
        }
        else if (key == "tag")
        {
            rule.key = TAG;
        }
        else if (key == "tagname")
        {
            rule.key = TAGNAME;
        }
        else if (key == "keyword")
        {
            rule.key = KEYWORD;
        }
        else if (key == "rating")
        {
            rule.key = RATING;
        }
        else
        {
            DWarning() << "Unknown rule type: " << key << " passed to kioslave"
                        << endl;
            continue;
        }

        if (op == "eq")
            rule.op = EQ;
        else if (op == "ne")
            rule.op = NE;
        else if (op == "lt")
            rule.op = LT;
        else if (op == "lte")
            rule.op = LTE;
        else if (op == "gt")
            rule.op = GT;
        else if (op == "gte")
            rule.op = GTE;
        else if (op == "like")
            rule.op = LIKE;
        else if (op == "nlike")
            rule.op = NLIKE;
        else
        {
            DWarning() << "Unknown op type: " << op << " passed to kioslave"
                        << endl;
            continue;
        }

        rule.val = url.queryItem(QString::number(i) + ".val");

        rulesMap.insert(i, rule);
    }

    QString sqlQuery;
    SubQueryBuilder subQuery;

    QStringList strList = url.path().split(' ', QString::SkipEmptyParts);
    for ( QStringList::Iterator it = strList.begin(); it != strList.end(); ++it )
    {
        bool ok;
        int  num = (*it).toInt(&ok);
        if (ok)
        {
            RuleType rule = rulesMap[num];
            if (rule.key == KEYWORD)
            {
                bool exact;
                QString possDate = possibleDate(rule.val, exact);
                if (!possDate.isEmpty())
                {
                    rule.key = IMAGEDATE;
                    rule.val = possDate;
                    if (exact)
                    {
                        rule.op = EQ;
                    }
                    else
                    {
                        rule.op = LIKE;
                    }

                    sqlQuery += subQuery.build(rule.key, rule.op, rule.val, boundValues);
                }
                else
                {
                    QList<SKey> todo;
                    todo.append( ALBUMNAME );
                    todo.append( IMAGENAME );
                    todo.append( TAGNAME );
                    todo.append( ALBUMCAPTION );
                    todo.append( ALBUMCOLLECTION );
                    todo.append( IMAGECAPTION );
                    todo.append( RATING );

                    sqlQuery += '(';
                    QList<SKey>::const_iterator it = todo.constBegin();
                    while ( it != todo.constEnd() )
                    {
                        sqlQuery += subQuery.build(*it, rule.op, rule.val, boundValues);
                        ++it;
                        if ( it != todo.end() )
                            sqlQuery += " OR ";
                    }
                    sqlQuery += ')';
                }
            }
            else
            {
                sqlQuery += subQuery.build(rule.key, rule.op, rule.val, boundValues);
            }
        }
        else
        {
            sqlQuery += ' ' + *it + ' ';
        }
    }

    return sqlQuery;
}

QString SubQueryBuilder::build(enum SKey key, enum SOperator op,
                               const QString& passedVal, QList<QVariant> *boundValues) const
{
    QString query;
    QString val = passedVal;

    if (op == LIKE || op == NLIKE)
        val = "%" + val + "%";

    switch (key)
    {
        case(ALBUM):
        {
            query = " (Images.dirid $$##$$ ?) ";
            *boundValues << val;
            break;
        }
        case(ALBUMNAME):
        {
            query = " (Images.dirid IN "
                    "  (SELECT id FROM Albums WHERE url $$##$$ ?)) ";
            *boundValues << val;
            break;
        }
        case(ALBUMCAPTION):
        {
            query = " (Images.dirid IN "
                    "  (SELECT id FROM Albums WHERE caption $$##$$ ?)) ";
            *boundValues << val;
            break;
        }
        case(ALBUMCOLLECTION):
        {
            query = " (Images.dirid IN "
                    "  (SELECT id FROM Albums WHERE collection $$##$$ ?)) ";
            *boundValues << val;
            break;
        }
        case(TAG):
        {
            if (op == EQ)
            {
                query = " (Images.id IN "
                        "   (SELECT imageid FROM ImageTags "
                        "    WHERE tagid = ?)) ";
                *boundValues << val.toInt();
            }
            else if (op == NE)
            {
                query = " (Images.id NOT IN "
                        "   (SELECT imageid FROM ImageTags "
                        "    WHERE tagid = ?)) ";
                *boundValues << val.toInt();
            }
            else if (op == LIKE)
            {
                query = " (Images.id IN "
                        "   (SELECT ImageTags.imageid FROM ImageTags JOIN TagsTree on ImageTags.tagid = TagsTree.id "
                        "    WHERE TagsTree.pid = ? or ImageTags.tagid = ? )) ";
                *boundValues << val.toInt() << val.toInt();
            }
            else // op == NLIKE
            {
                query = " (Images.id NOT IN "
                        "   (SELECT ImageTags.imageid FROM ImageTags JOIN TagsTree on ImageTags.tagid = TagsTree.id "
                        "    WHERE TagsTree.pid = ? or ImageTags.tagid = ? )) ";
                *boundValues << val.toInt() << val.toInt();
            }

    //         query = " (Images.id IN "
    //                 "   (SELECT imageid FROM ImageTags "
    //                 "    WHERE tagid $$##$$ ?)) ";

            break;
        }
        case(TAGNAME):
        {
            query = " (Images.id IN "
                    "  (SELECT imageid FROM ImageTags "
                    "   WHERE tagid IN "
                    "   (SELECT id FROM Tags WHERE name $$##$$ ?))) ";
            *boundValues << val;
            break;
        }
        case(IMAGENAME):
        {
            query = " (Images.name $$##$$ ?) ";
            *boundValues << val;
            break;
        }
        case(IMAGECAPTION):
        {
            query = " (Images.caption $$##$$ ?) ";
            *boundValues << val;
            break;
        }
        case(IMAGEDATE):
        {
            query = " (Images.datetime $$##$$ ?) ";
            *boundValues << val;
            break;
        }
        case (KEYWORD):
        {
            DWarning() << "KEYWORD Detected which is not possible" << endl;
            break;
        }
        case(RATING):
        {
            query = " (ImageProperties.value $$##$$ ? and ImageProperties.property='Rating') ";
            *boundValues << val;
            break;
        }
    }

    if (key != TAG)
    {
        switch (op)
        {
            case(EQ):
            {
                query.replace("$$##$$", "=");
                break;
            }
            case(NE):
            {
                query.replace("$$##$$", "<>");
                break;
            }
            case(LT):
            {
                query.replace("$$##$$", "<");
                break;
            }
            case(GT):
            {
                query.replace("$$##$$", ">");
                break;
            }
            case(LTE):
            {
                query.replace("$$##$$", "<=");
                break;
            }
            case(GTE):
            {
                query.replace("$$##$$", ">=");
                break;
            }
            case(LIKE):
            {
                query.replace("$$##$$", "LIKE");
                break;
            }
            case(NLIKE):
            {
                query.replace("$$##$$", "NOT LIKE");
                break;
            }
        }
    }

    // special case for imagedate. If the key is imagedate and the operator is EQ,
    // we need to split it into two rules
    if (key == IMAGEDATE && op == EQ)
    {
        QDate date = QDate::fromString(val, Qt::ISODate);
        if (!date.isValid())
            return query;

        query = QString(" (Images.datetime > ? AND Images.datetime < ?) ");
        *boundValues << date.addDays(-1).toString(Qt::ISODate)
                   << date.addDays( 1).toString(Qt::ISODate);
    }

    return query;
}

QString ImageQueryBuilder::possibleDate(const QString& str, bool& exact) const
{
    QDate date = QDate::fromString(str, Qt::ISODate);
    if (date.isValid())
    {
        exact = true;
        return date.toString(Qt::ISODate);
    }

    exact = false;

    bool ok;
    int num = str.toInt(&ok);
    if (ok)
    {
        // ok. its an int, does it look like a year?
        if (1970 <= num && num <= QDate::currentDate().year())
        {
            // very sure its a year
            return QString("%1-%-%").arg(num);
        }
    }
    else
    {
        // hmm... not a year. is it a particular month?
        for (int i=1; i<=12; i++)
        {
            if (str.toLower() == m_shortMonths[i-1] ||
                str.toLower() == m_longMonths[i-1])
            {
                QString monGlob;
                monGlob.sprintf("%.2d", i);
                monGlob = "%-" + monGlob + "-%";
                return monGlob;
            }
        }
    }

    return QString();
}

}  // namespace Digikam
