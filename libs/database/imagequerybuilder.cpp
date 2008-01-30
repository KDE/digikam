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
#include "databaseaccess.h"
#include "databasebackend.h"
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
    SearchXmlReader reader(xml);
    QString sql;
    bool firstGroup = true;

    while (reader.readNext() != SearchXml::End)
    {
        if (reader.isGroupElement())
        {
            addSqlOperator(sql, reader.groupOperator(), firstGroup);
            if (firstGroup)
                firstGroup = false;

            sql += " (";
            buildGroup(sql, reader, boundValues);
            sql += ") ";
        }
    }

    return sql;
}

void ImageQueryBuilder::buildGroup(QString &sql, SearchXmlReader &reader, QList<QVariant> *boundValues) const
{
    bool firstField = true;
    while (reader.readNext() != SearchXml::GroupEnd)
    {
        if (reader.isFieldElement())
        {
            addSqlOperator(sql, reader.fieldOperator(), firstField);
            if (firstField)
                firstField = false;

            buildField(sql, reader, reader.fieldName(), boundValues);
        }
    }
}

class FieldQueryBuilder
{
public:

    FieldQueryBuilder(QString &sql, SearchXmlReader &reader,
                      QList<QVariant> *boundValues, SearchXml::Relation relation)
       : sql(sql), reader(reader), boundValues(boundValues), relation(relation)
    {
    }

    QString &sql;
    SearchXmlReader &reader;
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
        sql += " (" + name + ' ';
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?) ";
        *boundValues << reader.value();
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


void ImageQueryBuilder::buildField(QString &sql, SearchXmlReader &reader, const QString &name, QList<QVariant> *boundValues) const
{
    SearchXml::Relation relation = reader.fieldRelation();
    FieldQueryBuilder fieldQuery(sql, reader, boundValues, relation);
    if (name == "albumid")
    {
        fieldQuery.addIntField("Images.id");
    }
    else if (name == "albumname")
    {
        fieldQuery.addStringField("Album.relativePath");
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
        if (relation == SearchXml::Equal)
        {
            sql += " (Images.id IN "
                   "   (SELECT imageid FROM ImageTags "
                   "    WHERE tagid IN "
                   "   (SELECT id FROM Tags WHERE name LIKE ?))) ";
            *boundValues << tagname;
        }
        else if (relation == SearchXml::Unequal)
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
               " (SELECT id FROM ImageComments "
               "  WHERE type=? AND comment ";
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?) ";
        *boundValues << DatabaseComment::Comment << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == "commentauthor")
    {
        sql += " (Images.id IN "
               " (SELECT id FROM ImageComments "
               "  WHERE type=? AND author ";
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?)) ";
        *boundValues << DatabaseComment::Comment << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == "headline")
    {
        sql += " (Images.id IN "
               " (SELECT id FROM ImageComments "
               "  WHERE type=? AND comment ";
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?)) ";
        *boundValues << DatabaseComment::Headline << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == "title")
    {
        sql += " (Images.id IN "
               " (SELECT id FROM ImageComments "
               "  WHERE type=? AND comment ";
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?)) ";
        *boundValues << DatabaseComment::Comment << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == "keyword")
    {
        // keyword is the common search in the text fields
        buildField(sql, reader, "albumname", boundValues);
        buildField(sql, reader, "filename", boundValues);
        buildField(sql, reader, "tagname", boundValues);
        buildField(sql, reader, "albumcaption", boundValues);
        buildField(sql, reader, "albumcollection", boundValues);
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
        case SearchXml::Or:
            sql += "OR";
        case SearchXml::AndNot:
            sql += "AND NOT";
    }
}

void ImageQueryBuilder::addSqlRelation(QString &sql, SearchXml::Relation rel)
{
    switch (rel)
    {
        default:
        case SearchXml::Equal:
            sql += "=";
        case SearchXml::Unequal:
            sql += "<>";
        case SearchXml::Like:
            sql += "LIKE";
        case SearchXml::NotLike:
            sql += "NOT LIKE";
        case SearchXml::LessThan:
            sql += "<";
        case SearchXml::GreaterThan:
            sql += ">";
        case SearchXml::LessThanOrEqual:
            sql += "<=";
        case SearchXml::GreaterThanOrEqual:
            sql += ">=";
        case SearchXml::OneOf:
            sql += "IN";
    }
}


// ----------- Legacy query description handling -------------- //

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

                    sqlQuery += subQuery(rule.key, rule.op, rule.val, boundValues);
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
                        sqlQuery += subQuery(*it, rule.op, rule.val, boundValues);
                        ++it;
                        if ( it != todo.end() )
                            sqlQuery += " OR ";
                    }
                    sqlQuery += ')';
                }
            }
            else
            {
                sqlQuery += subQuery(rule.key, rule.op, rule.val, boundValues);
            }
        }
        else
        {
            sqlQuery += ' ' + *it + ' ';
        }
    }

    return sqlQuery;
}

QString ImageQueryBuilder::subQuery(enum ImageQueryBuilder::SKey key,
                                    enum ImageQueryBuilder::SOperator op,
                                    const QString& passedVal,
                                    QList<QVariant> *boundValues) const
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
