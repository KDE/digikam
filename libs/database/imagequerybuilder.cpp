/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : Building complex database SQL queries from search descriptions
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagequerybuilder.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QFile>
#include <QDir>
#include <QMap>
#include <QRectF>

// KDE includes

#include <kglobal.h>
#include <klocale.h>
#include <kcalendarsystem.h>
#include <kcomponentdata.h>
#include <kmimetype.h>
#include <kdebug.h>

// KExiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "databaseaccess.h"
#include "albumdb.h"
#include "geodetictools.h"

namespace Digikam
{

class ImageQueryPostHook
{
public:

    // This is the single hook, ImageQueryPostHookS is the container
    virtual ~ImageQueryPostHook()
    {
    };

    virtual bool checkPosition(double /*latitudeNumber*/, double /*longitudeNumber*/)
    {
        return true;
    };
};

ImageQueryPostHooks::~ImageQueryPostHooks()
{
    foreach(ImageQueryPostHook* hook, m_postHooks)
    {
        delete hook;
    }
}

void ImageQueryPostHooks::addHook(ImageQueryPostHook* const hook)
{
    m_postHooks << hook;
}

bool ImageQueryPostHooks::checkPosition(double latitudeNumber, double longitudeNumber)
{
    foreach(ImageQueryPostHook* hook, m_postHooks)
    {
        if (!hook->checkPosition(latitudeNumber, longitudeNumber))
        {
            return false;
        }
    }
    return true;
}

// -----------------------------------------------------------------------------------------------

ImageQueryBuilder::ImageQueryBuilder()
{
    // build a lookup table for month names
    const KCalendarSystem* cal = KGlobal::locale()->calendar();

    for (int i=1; i<=12; ++i)
    {
        m_shortMonths[i-1] = cal->monthName(i, 2000, KCalendarSystem::ShortName).toLower();
        m_longMonths[i-1]  = cal->monthName(i, 2000, KCalendarSystem::LongName).toLower();
    }

    m_imageTagPropertiesJoined = false;
}

void ImageQueryBuilder::setImageTagPropertiesJoined(bool isJoined)
{
    m_imageTagPropertiesJoined = isJoined;
}

QString ImageQueryBuilder::buildQuery(const QString& q, QList<QVariant> *boundValues, ImageQueryPostHooks* const hooks) const
{
    // Handle legacy query descriptions
    if (q.startsWith(QLatin1String("digikamsearch:")))
    {
        return buildQueryFromUrl(KUrl(q), boundValues);
    }
    else
    {
        return buildQueryFromXml(q, boundValues, hooks);
    }
}

QString ImageQueryBuilder::buildQueryFromXml(const QString& xml, QList<QVariant> *boundValues, ImageQueryPostHooks* const hooks) const
{
    SearchXmlCachingReader reader(xml);
    QString                sql;
    bool                   firstGroup = true;

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isEndElement())
        {
            continue;
        }

        if (reader.isGroupElement())
        {
            addSqlOperator(sql, reader.groupOperator(), firstGroup);

            if (firstGroup)
            {
                firstGroup = false;
            }

            buildGroup(sql, reader, boundValues, hooks);
        }
    }

    kDebug() << sql;

    return sql;
}

void ImageQueryBuilder::buildGroup(QString& sql, SearchXmlCachingReader& reader,
                                   QList<QVariant> *boundValues, ImageQueryPostHooks* const hooks) const
{
    sql += " (";

    SearchXml::Operator mainGroupOp = reader.groupOperator();

    bool firstField = true;
    bool hasContent = false;

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isEndElement())
        {
            break;
        }

        // subgroup
        if (reader.isGroupElement())
        {
            hasContent = true;
            addSqlOperator(sql, reader.groupOperator(), firstField);

            if (firstField)
            {
                firstField = false;
            }

            buildGroup(sql, reader, boundValues, hooks);
        }

        if (reader.isFieldElement())
        {
            hasContent                        = true;
            SearchXml::Operator fieldOperator = reader.fieldOperator();
            addSqlOperator(sql, fieldOperator, firstField);

            if (firstField)
            {
                firstField = false;
            }

            if (!buildField(sql, reader, reader.fieldName(), boundValues, hooks))
            {
                addNoEffectContent(sql, fieldOperator);
            }
        }
    }

    if (!hasContent)
    {
        addNoEffectContent(sql, mainGroupOp);
    }

    sql += ") ";
}

// ---------------------------------------------------------------------------------------

class FieldQueryBuilder
{
public:

    FieldQueryBuilder(QString& sql, SearchXmlCachingReader& reader,
                      QList<QVariant>* boundValues, ImageQueryPostHooks* const hooks, SearchXml::Relation relation)
        : sql(sql),
          reader(reader),
          boundValues(boundValues),
          hooks(hooks),
          relation(relation)
    {
    }

public:

    QString&                sql;
    SearchXmlCachingReader& reader;
    QList<QVariant>*        boundValues;
    ImageQueryPostHooks*    hooks;
    SearchXml::Relation     relation;

public:

    inline QString prepareForLike(const QString& str) const
    {
        if (relation == SearchXml::Like || relation == SearchXml::NotLike)
        {
            return '%' + str + '%';
        }
        else
        {
            return str;
        }
    }

    void addIntField(const QString& name)
    {
        if (relation == SearchXml::Interval || relation == SearchXml::IntervalOpen)
        {
            QList<int> values = reader.valueToIntList();

            if (values.size() != 2)
            {
                kWarning() << "Relation Interval requires a list of two values";
                return;
            }

            sql += " (" + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
            sql += " ? AND " + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
            sql += " ?) ";

            *boundValues << values.first() << values.last();
        }
        else
        {
            sql += " (" + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql, relation);
            sql += " ?) ";
            *boundValues << reader.valueToInt();
        }
    }

    void addDoubleField(const QString& name)
    {
        if (relation == SearchXml::Interval || relation == SearchXml::IntervalOpen)
        {
            QList<double> values = reader.valueToDoubleList();

            if (values.size() != 2)
            {
                kWarning() << "Relation Interval requires a list of two values";
                return;
            }

            sql += " (" + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
            sql += " ? AND " + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
            sql += " ?) ";

            *boundValues << values.first() << values.last();
        }
        else
        {
            sql += " (" + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql, relation);
            sql += " ?) ";
            *boundValues << reader.valueToDouble();
        }
    }

    void addStringField(const QString& name)
    {
        sql += " (" + name + ' ';
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += " ?) ";
        *boundValues << prepareForLike(reader.value());
    }

    void addDateField(const QString& name)
    {
        if (relation == SearchXml::Equal)
        {
            // special case: split in < and >
            QDateTime date = QDateTime::fromString(reader.value(), Qt::ISODate);

            if (!date.isValid())
            {
                kWarning() << "Date" << reader.value() << "is invalid";
                return;
            }

            QDateTime startDate, endDate;

            if (date.time() == QTime(0, 0, 0, 0))
            {
                // day precision
                QDate startDate, endDate;
                startDate = date.date().addDays(-1);
                endDate   = date.date().addDays(1);
                *boundValues << startDate.toString(Qt::ISODate)
                             << endDate.toString(Qt::ISODate);
            }
            else
            {
                // sub-day precision
                QDateTime startDate, endDate;
                int diff;

                if (date.time().hour() == 0)
                {
                    diff = 3600;
                }
                else if (date.time().minute() == 0)
                {
                    diff = 60;
                }
                else
                {
                    diff = 1;
                }

                // we spare microseconds for the future

                startDate = date.addSecs(-diff);
                endDate   = date.addSecs(diff);
                *boundValues << startDate.toString(Qt::ISODate)
                             << endDate.toString(Qt::ISODate);
            }

            sql += " (" + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql, SearchXml::GreaterThan);
            sql += " ? AND " + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql, SearchXml::LessThan);
            sql += " ?) ";
        }
        else if (relation == SearchXml::Interval || relation == SearchXml::IntervalOpen)
        {
            QList<QString> values = reader.valueToStringList();

            if (values.size() != 2)
            {
                kWarning() << "Relation Interval requires a list of two values";
                return;
            }

            sql += " (" + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
            sql += " ? AND " + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
            sql += " ?) ";

            *boundValues << values.first() << values.last();
        }
        else
        {
            sql += " (" + name + ' ';
            ImageQueryBuilder::addSqlRelation(sql, relation);
            sql += " ?) ";
            *boundValues << reader.value();
        }
    }

    void addChoiceIntField(const QString& name)
    {
        if (relation == SearchXml::OneOf)
        {
            QList<int> values = reader.valueToIntList();
            bool searchForNull = values.removeAll(-1);
            sql += " (" + name + " IN (";
            AlbumDB::addBoundValuePlaceholders(sql, values.size());

            if (searchForNull)
            {
                sql += ") OR " + name + " IS NULL";
            }
            else
            {
                sql += ") ";
            }

            foreach(int v, values)
            {
                *boundValues << v;
            }

            sql += " ) ";
        }
        else
        {
            addIntField(name);
        }
    }

    void addLongListField(const QString& name)
    {
        if (relation == SearchXml::OneOf)
        {
            QList<qlonglong> values = reader.valueToLongLongList();
            sql += " (" + name + " IN (";
            AlbumDB::addBoundValuePlaceholders(sql, values.size());
            sql += ") ";

            foreach(const qlonglong& v, values)
            {
                *boundValues << v;
            }

            sql += " ) ";
        }
        else
        {
            addIntField(name);
        }
    }

    void addIntBitmaskField(const QString& name)
    {
        if (relation == SearchXml::OneOf)
        {
            QList<int> values = reader.valueToIntList();
            bool searchForNull = values.removeAll(-1);
            sql += "( ";
            bool first = true;

            for (int i=0; i<values.size(); ++i)
            {
                if (!first)
                {
                    sql += "OR ";
                }

                first = false;
                sql += name + " & ? ";
            }

            if (searchForNull)
            {
                sql += "OR " + name + " IS NULL ";
            }

            foreach(int v, values)
            {
                *boundValues << v;
            }

            sql += " ) ";
        }
        else
        {
            if (relation == SearchXml::Equal)
            {
                sql += " (" + name + " & " + " ?) ";
            }
            else
            {
                sql += " (NOT " + name + " & " + " ?) ";
            }

            *boundValues << reader.valueToDouble();
        }
    }

    void addChoiceStringField(const QString& name)
    {
        if (relation == SearchXml::OneOf)
        {
            QStringList values = reader.valueToStringList();

            if (values.isEmpty())
            {
                kDebug() << "List for OneOf is empty";
                return;
            }

            QStringList simpleValues, wildcards;

            foreach(const QString& value, values)
            {
                if (value.contains("*"))
                {
                    wildcards << value;
                }
                else
                {
                    simpleValues << value;
                }
            }

            bool firstCondition =  true;
            sql                 += " (";

            if (!simpleValues.isEmpty())
            {
                firstCondition =  false;
                sql            += name + " IN (";
                AlbumDB::addBoundValuePlaceholders(sql, simpleValues.size());

                foreach(const QString& value, simpleValues)
                {
                    *boundValues << value;
                }

                sql += " ) ";
            }

            if (!wildcards.isEmpty())
            {
                foreach(QString wildcard, wildcards) // krazy:exclude=foreach
                {
                    ImageQueryBuilder::addSqlOperator(sql, SearchXml::Or, firstCondition);
                    firstCondition = false;
                    wildcard.replace('*', '%');
                    sql += ' ' + name + ' ';
                    ImageQueryBuilder::addSqlRelation(sql, SearchXml::Like);
                    sql += " ? ";
                    *boundValues << wildcard;
                }
            }

            sql += ") ";
        }
        else
        {
            QString value = reader.value();

            if (relation == SearchXml::Like && value.contains("*"))
            {
                // Handle special case: * denotes the place if the wildcard,
                // Don't automatically prepend and append %
                sql += " (" + name + ' ';
                ImageQueryBuilder::addSqlRelation(sql, SearchXml::Like);
                sql += " ?) ";
                QString wildcard = reader.value();
                wildcard.replace('*', '%');
                *boundValues << wildcard;
            }
            else
            {
                addStringField(name);
            }
        }
    }

    void addPosition()
    {
        if (relation == SearchXml::Near)
        {
            // First read attributes
            QStringRef type           = reader.attributes().value("type");
            QStringRef distanceString = reader.attributes().value("distance");
            // Distance in meters
            double distance           = 100;

            if (!distanceString.isEmpty())
            {
                distance = distanceString.toString().toDouble();
            }

            // Search type, "radius" or "rectangle"
            bool radiusSearch = true;

            if (type == "radius")
            {
                radiusSearch = true;
            }
            else if (type == "rectangle")
            {
                radiusSearch = false;
            }

            // Get a list of doubles:
            // Longitude and Latitude in (decimal) degrees
            QList<double> list = reader.valueToDoubleList();

            if (list.size() != 2)
            {
                kWarning() << "Relation 'Near' requires a list of two values";
                return;
            }

            double lon = list.at(0);
            double lat = list.at(1);

            sql += " ( ";

            // Part 1: Rectangle search.
            // Get the coordinates of the (spherical) rectangle enclosing
            // the (spherical) circle given by our coordinates and the distance.
            // For this one-time computation we use the advanced code
            // which assumes the earth is a ellipsoid.

            // From the point (lon,lat) we go East, North, West, South,
            // and get the coordinates in degrees of a rectangle
            // of width and height 2*distance enclosing (lon,lat)
            QRectF rect;
            GeodeticCalculator calc;
            calc.setStartingGeographicPoint(lon, lat);
            // go west
            calc.setDirection(-90, distance);
            rect.setLeft(calc.destinationGeographicPoint().x());
            // go north (from first starting point!)
            calc.setDirection(0, distance);
            rect.setTop(calc.destinationGeographicPoint().y());
            // go east
            calc.setDirection(90, distance);
            rect.setRight(calc.destinationGeographicPoint().x());
            // go south
            calc.setDirection(180, distance);
            rect.setBottom(calc.destinationGeographicPoint().y());

            addRectanglePositionSearch(rect.x(), rect.y(), rect.right(), rect.bottom());

            if (radiusSearch)
            {
                // Part 2: Use the Haversine formula to filter out from
                // the matching pictures those that lie inside the
                // actual (spherical) circle.
                // This code only assumes that the earth is a sphere.
                // But this needs to be computed n times, so it's expensive.
                // We refrain from putting this into SQL, but use a post hook.

                /*
                Reference: http://www.usenet-replayer.com/faq/comp.infosystems.gis.html
                Pseudo code of the formula:
                    Position 1 (lon1, lat1), position 2 (lon2, lat2), in Radians
                    d: distance; R: radius of earth. Same unit (assume: meters)
                dlon = lon2 - lon1;
                dlat = lat2 - lat1;
                a = (sin(dlat/2))^2 + cos(lat1) * cos(lat2) * (sin(dlon/2))^2;
                c = 2 * arcsin(min(1,sqrt(a)));
                d = R * c;
                // We precompute c.
                */

                class HaversinePostHook : public ImageQueryPostHook
                {
                public:

                    HaversinePostHook(double lat1Deg, double lon1Deg, double radiusOfCurvature, double distance)
                    {
                        lat1              = Coordinates::toRadians(lat1Deg);
                        lon1              = Coordinates::toRadians(lon1Deg);
                        distanceInRadians = distance / radiusOfCurvature;
                        cosLat1           = cos(lat1);
                    }

                    virtual bool checkPosition(double lat2Deg, double lon2Deg)
                    {
                        double lat2 = Coordinates::toRadians(lat2Deg);
                        double lon2 = Coordinates::toRadians(lon2Deg);
                        double dlon = lon2 - lon1;
                        double dlat = lat2 - lat1;
                        double a    = pow(sin(dlat/2), 2) + cosLat1 * cos(lat2) * pow(sin(dlon/2),2);
                        double c    = 2 * asin(qMin(1.0, sqrt(a)));

                        return (c < distanceInRadians);
                    }

                    double lat1, lon1;
                    double distanceInRadians;
                    double cosLat1;
                };

                // get radius (of the ellipsoid) in dependence of the latitude.
                double R = calc.ellipsoid().radiusOfCurvature(lat);
                hooks->addHook(new HaversinePostHook(lat, lon, R, distance));
            }

            sql += " ) ";
        }
        else if (relation == SearchXml::Inside)
        {
            // First read attributes
            QStringRef type = reader.attributes().value("type");

            // Search type, currently only "rectangle"
            if (type != "rectangle")
            {
                kWarning() << "Relation 'Inside' supports no other type than 'rectangle'";
                return;
            }

            // Get a list of doubles:
            // Longitude and Latitude in (decimal) degrees
            QList<double> list = reader.valueToDoubleList();

            if (list.size() != 4)
            {
                kWarning() << "Relation 'Inside' requires a list of four values";
                return;
            }

            // the list contains (lon1,lat1), (lon2,lat2) in this order,
            // like (x,y), (right,bottom) of a rectangle,
            // or like (West,North), (East,South),
            // where the searched region contains any lon,lat
            //  where lon1 < lon < lon2 and lat1 < lat < lat2.
            double lon1,lat1,lon2,lat2;
            lon1 = list.at(0);
            lat1 = list.at(1);
            lon2 = list.at(2);
            lat2 = list.at(3);

            sql += " ( ";
            addRectanglePositionSearch(lon1, lat1, lon2, lat2);
            sql += " ) ";
        }
    }

    void addRectanglePositionSearch(double lon1, double lat1, double lon2, double lat2) const
    {
        // lon1 is always West of lon2. If the rectangle crosses 180 longitude, we have to treat a special case.
        if (lon1 <= lon2)
        {
            sql += " ImagePositions.LongitudeNumber > ? AND ImagePositions.LatitudeNumber < ? "
                   " AND ImagePositions.LongitudeNumber < ? AND ImagePositions.LatitudeNumber > ? ";
            *boundValues << lon1 << lat1 << lon2 << lat2;
        }
        else
        {
            // this effectively means splitting the rectangle is two parts, one East, one West
            // to the 180 line. But no need to check for less/greater than -180/180.
            sql += " (ImagePositions.LongitudeNumber > ? OR ImagePositions.LongitudeNumber < ?) "
                   " AND ImagePositions.LatitudeNumber < ? AND ImagePositions.LatitudeNumber > ? ";
            *boundValues << lon1 << lon2 << lat1 << lat2;
        }
    }
};

// ----------------------------------------------------------------------------------------------------

bool ImageQueryBuilder::buildField(QString& sql, SearchXmlCachingReader& reader, const QString& name,
                                   QList<QVariant>* boundValues, ImageQueryPostHooks* const hooks) const
{
    SearchXml::Relation relation = reader.fieldRelation();
    FieldQueryBuilder fieldQuery(sql, reader, boundValues, hooks, relation);

    if (name == "albumid")
    {
        if (relation == SearchXml::Equal || relation == SearchXml::Unequal)
        {
            fieldQuery.addIntField("Images.album");
        }
        else if (relation == SearchXml::InTree)
        {
            // see also: AlbumDB::getItemNamesInAlbum
            QList<int> ids = reader.valueToIntOrIntList();

            if (ids.isEmpty())
            {
                kDebug() << "Relation 'InTree', name 'albumid': No values given";
                return false;
            }

            sql += "(Images.album IN "
                   "   (SELECT DISTINCT id "
                   "    FROM Albums WHERE ";
            bool firstCondition = true;

            foreach(int albumID, ids)
            {
                addSqlOperator(sql, SearchXml::Or, firstCondition);
                firstCondition = false;

                DatabaseAccess access;
                int rootId           = access.db()->getAlbumRootId(albumID);
                QString relativePath = access.db()->getAlbumRelativePath(albumID);

                QString childrenWildcard;

                if (relativePath == "/")
                {
                    childrenWildcard = "/%";
                }
                else
                {
                    childrenWildcard = relativePath + "/%";
                }

                sql += " ( albumRoot=? AND (relativePath=? OR relativePath LIKE ?) ) ";
                *boundValues << rootId << relativePath << childrenWildcard;
            }

            sql += " ))";
        }
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
        fieldQuery.addChoiceStringField("Albums.collection");
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
        else if (relation == SearchXml::InTree || relation == SearchXml::NotInTree)
        {
            QList<int> ids = reader.valueToIntOrIntList();

            if (ids.isEmpty())
            {
                kDebug() << "Relation 'InTree', name 'tagid': No values given";
                return false;
            }

            if (relation == SearchXml::InTree)
            {
                sql += " (Images.id IN ";
            }
            else
            {
                sql += " (Images.id NOT IN ";
            }

            sql += "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                   "    WHERE ";

            bool firstCondition = true;

            foreach(int tagID, ids)
            {
                addSqlOperator(sql, SearchXml::Or, firstCondition);
                firstCondition = false;
                sql += " (TagsTree.pid = ? OR ImageTags.tagid = ? ) ";
                *boundValues << tagID << tagID;
            }

            sql += " )) ";
        }
    }
    else if (name == "tagname")
    {
        QString tagname = '%' + reader.value() + '%';

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
                   "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                   "    WHERE TagsTree.pid = (SELECT id FROM Tags WHERE name LIKE ?) "
                   "    or ImageTags.tagid = (SELECT id FROM Tags WHERE name LIKE ?) )) ";
            *boundValues << tagname << tagname;
        }
        else if (relation == SearchXml::NotInTree)
        {
            sql += " (Images.id NOT IN "
                   "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                   "    WHERE TagsTree.pid = (SELECT id FROM Tags WHERE name LIKE ?) "
                   "    or ImageTags.tagid = (SELECT id FROM Tags WHERE name LIKE ?) )) ";
            *boundValues << tagname << tagname;
        }
    }
    else if (name == "notag")
    {
        reader.readToEndOfElement();
        sql += " (Images.id NOT IN "
               "   (SELECT imageid FROM ImageTags)) ";
    }
    else if (name == "imageid")
    {
        fieldQuery.addLongListField("Images.id");
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
    else if (name == "pageorientation")
    {
        if (relation == SearchXml::Equal)
        {
            int pageOrientation = reader.valueToInt();

            // "1" is landscape, "2" is portrait, "3" is landscape regardless of Exif, "4" is portrait regardless of Exif
            if (pageOrientation == 1)
            {
                sql += " ( (ImageInformation.orientation <= ? AND ImageInformation.width >= ImageInformation.height) "
                       "  OR (ImageInformation.orientation >= ? AND ImageInformation.width <= ImageInformation.height) ) ";
                *boundValues << KExiv2Iface::KExiv2::ORIENTATION_VFLIP << KExiv2Iface::KExiv2::ORIENTATION_ROT_90_HFLIP;
            }
            else if (pageOrientation == 2)
            {
                sql += " ( (ImageInformation.orientation <= ? AND ImageInformation.width < ImageInformation.height) "
                       "  OR (ImageInformation.orientation >= ? AND ImageInformation.width > ImageInformation.height) ) ";
                *boundValues << KExiv2Iface::KExiv2::ORIENTATION_VFLIP << KExiv2Iface::KExiv2::ORIENTATION_ROT_90_HFLIP;
            }
            else if (pageOrientation == 3 || pageOrientation == 4)
            {
                // ignoring Exif orientation
                sql += " ( ImageInformation.width ";
                ImageQueryBuilder::addSqlRelation(sql, pageOrientation == 3 ? SearchXml::GreaterThanOrEqual : SearchXml::LessThanOrEqual);
                sql += " ImageInformation.height) ";
            }
        }
    }
    else if (name == "width")
    {
        sql += " ( (ImageInformation.orientation <= ? AND ";
        *boundValues << KExiv2Iface::KExiv2::ORIENTATION_VFLIP;
        fieldQuery.addIntField("ImageInformation.width");
        sql += ") OR (ImageInformation.orientation >= ? AND ";
        *boundValues << KExiv2Iface::KExiv2::ORIENTATION_ROT_90_HFLIP;
        fieldQuery.addIntField("ImageInformation.height");
        sql += " ) ) ";
    }
    else if (name == "height")
    {
        sql += " ( (ImageInformation.orientation <= ? AND ";
        *boundValues << KExiv2Iface::KExiv2::ORIENTATION_VFLIP;
        fieldQuery.addIntField("ImageInformation.height");
        sql += ") OR (ImageInformation.orientation >= ? AND ";
        *boundValues << KExiv2Iface::KExiv2::ORIENTATION_ROT_90_HFLIP;
        fieldQuery.addIntField("ImageInformation.width");
        sql += " ) ) ";
    }
    else if (name == "aspectratioimg")
    {
        QString query;
        QString readerString = (reader.valueToStringOrStringList()).at(0);
        if(readerString.contains(QRegExp("^\\d+:\\d+$")))
        {
            QStringList ratioNum = readerString.split(":", QString::SkipEmptyParts);
            int num = ratioNum.at(0).toInt();
            int denominator = ratioNum.at(1).toInt();
            query = "abs((ImageInformation.width/CAST(ImageInformation.height as REAL)) - ?)  < 0.1";
            sql += " (" + query + ") ";
            *boundValues << (double)num/denominator;
        }
        else if(readerString.contains(QRegExp("^\\d+(.\\d+)?$")))
        {
            query = "abs((ImageInformation.width/CAST(ImageInformation.height as REAL)) - ?)  < 0.1";
            sql += " (" + query + ") ";
            *boundValues << readerString.toDouble();
        }
    }
    else if (name == "pixelsize")
    {
        fieldQuery.addIntField("(ImageInformation.width * ImageInformation.height)");
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
    else if (name == "videoaspectratio")
    {
        if (relation == SearchXml::OneOf)
        {
            QStringList values = reader.valueToStringList();

            if (values.isEmpty())
            {
                kDebug() << "List for OneOf is empty";
            }

            QList<double> ratioValues;
            foreach(const QString& value, values)
            {
                 *boundValues << value;

                 if (value.contains(":"))
                 {
                     QStringList ratioNum = value.split(":", QString::SkipEmptyParts);
                     int num = ratioNum.at(0).toInt();
                     int denominator = ratioNum.at(1).toInt();
                     ratioValues << (double)num/denominator;
                 }
            }

            sql += "(VideoMetadata.aspectRatio IN (";
            AlbumDB::addBoundValuePlaceholders(sql, values.size());
            sql += ") ";
            QString query = "abs((CAST(VideoMetadata.aspectRatio as REAL) - ?)  < 0.1) ";

            foreach(double value, ratioValues)
            {
                *boundValues << value;
                sql +=  "OR " + query;
            }

            sql += ") ";
        }
        else
        {
            QString value = reader.value();
            *boundValues << value;

            if (value.contains(":"))
            {
                QStringList ratioNum = value.split(":", QString::SkipEmptyParts);
                int num = ratioNum.at(0).toInt();
                int denominator = ratioNum.at(1).toInt();
                *boundValues << (double)num/denominator;
            }

            sql += "(VideoMetadata.aspectRatio=? OR abs((CAST(VideoMetadata.aspectRatio as REAL) - ?)  < 0.1 )) ";
        }
    }
    else if (name == "videoaudiobitrate")
    {
        //fieldQuery.addIntField("VideoMetadata.audioBitRate");
        QList<int> values = reader.valueToIntList();

        if (values.size() != 2)
        {
            kWarning() << "Relation Interval requires a list of two values";
        }

        sql += " ( CAST(VideoMetadata.audioBitRate AS INTEGER)";
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
        sql += " ? AND CAST(VideoMetadata.audioBitRate AS INTEGER)";
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
        sql += " ?) ";

        *boundValues << values.first() << values.last();
    }
    else if (name == "videoaudiochanneltype")
    {
        if (relation == SearchXml::OneOf)
        {
            QStringList values = reader.valueToStringList();

            if (values.isEmpty())
            {
                kDebug() << "List for OneOf is empty";
            }

            foreach(const QString& value, values)
            {
                 *boundValues << value;

                 if (value=="1")
                 {
                     *boundValues << "Mono";
                 }
                 else if (value == "2")
                 {
                     *boundValues << "Stereo";
                 }
            }

            sql += "(VideoMetadata.audioChannelType IN (";
            AlbumDB::addBoundValuePlaceholders(sql, boundValues->size());
            sql += ")) ";
        }
        else
        {
            QString value = reader.value();
            *boundValues << value;

            if (value=="1")
            {
                *boundValues << "Mono";
            }
            else if (value == "2")
            {
                *boundValues << "Stereo";
            }

            sql += "(VideoMetadata.audioChannelType IN (";
            AlbumDB::addBoundValuePlaceholders(sql, boundValues->size());
            sql += ")) ";
        }
    }
    else if (name == "videoaudiocompressor")
    {
        fieldQuery.addChoiceStringField("VideoMetadata.audioCompressor");
    }
    else if (name == "videoduration")
    {
        QList<int> values = reader.valueToIntList();

        if (values.size() != 2)
        {
            kWarning() << "Relation Interval requires a list of two values";
        }

        sql += " ( CAST(VideoMetadata.duration AS INTEGER)";
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
        sql += " ? AND CAST(VideoMetadata.duration AS INTEGER)";
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
        sql += " ?) ";

        *boundValues << values.first()*1000 << values.last()*1000;
    }
    else if (name == "videoframerate")
    {
        //fieldQuery.addChoiceStringField("VideoMetadata.frameRate");
        QList<double> values = reader.valueToDoubleList();

        if (values.size() != 2)
        {
            kWarning() << "Relation Interval requires a list of two values";
        }

        sql += " ( CAST(VideoMetadata.frameRate AS REAL)";
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
        sql += " ? AND CAST(VideoMetadata.frameRate AS REAL)";
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
        sql += " ?) ";

        *boundValues << values.first() << values.last();
    }
    else if (name == "videocodec")
    {
        if (relation == SearchXml::OneOf)
        {
            QStringList values = reader.valueToStringList();

            if (values.isEmpty())
            {
                kDebug() << "List for OneOf is empty";
            }

            foreach(const QString& value, values)
            {
                sql += "( Upper(VideoMetadata.videoCodec) LIKE '%" + value.toUpper() + "%' ";
                if (value!=values.last())
                {
                    sql += "OR ";
                }
            }
            sql += ") ";
        }
        else
        {
            QString value = reader.value();
            sql += "(Upper(VideoMetadata.videoCodec) LIKE '%" + value.toUpper() + "%') ";
        }
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
    else if (name == "position")
    {
        fieldQuery.addPosition();
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
        *boundValues << DatabaseComment::Title << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == "imagetagproperty")
    {
        if (relation == SearchXml::Equal || relation == SearchXml::InTree)
        {
            // First, read attributes
            QStringRef tagAttribute = reader.attributes().value("tagid");
            int tagId = 0;

            if (!tagAttribute.isEmpty())
            {
                tagId = tagAttribute.toString().toInt();
            }

            // read values: one or two strings
            QStringList values = reader.valueToStringOrStringList();

            if (values.size() < 1 || values.size() > 2)
            {
                kDebug() << "The imagetagproperty field requires one value (property) or two values (property, value).";
                return false;
            }

            QString selectQuery;
            // %1 is resolved to either "ImageTagProperties." or the empty string
            if (tagId)
            {
                if (relation == SearchXml::Equal)
                {
                    selectQuery += "%1tagid=? AND ";
                    *boundValues << tagId;
                }
                else // InTree
                {
                    selectQuery += "(%1tagid=? OR %1tagid IN (SELECT id FROM TagsTree WHERE pid=?)) AND ";
                    *boundValues << tagId << tagId;
                }
            }

            if (values.size() == 1)
            {
                selectQuery += "%1property=? ";
                *boundValues << values.first();
            }
            else if (values.size() == 2)
            {
                selectQuery += "%1property=? AND %1value ";
                ImageQueryBuilder::addSqlRelation(selectQuery, relation);
                selectQuery += " ? ";
                *boundValues << values.at(0) << fieldQuery.prepareForLike(values.at(1));
            }

            // This indicates that the ImageTagProperties is joined in the SELECT query,
            // so one entry is listed for each property entry (not for each image id)
            if (m_imageTagPropertiesJoined)
            {
                sql += " ( ";
                sql += selectQuery.arg("ImageTagProperties.");
                sql += " ) ";
            }
            else
            {
                sql += " (Images.id IN "
                       " (SELECT imageid FROM ImageTagProperties WHERE ";
                sql += selectQuery.arg(QString());
                sql += " )) ";
            }
        }
    }
    else if (name == "keyword")
    {
        // keyword is the common search in the text fields

        sql += " ( ";

        addSqlOperator(sql, SearchXml::Or, true);
        buildField(sql, reader, "albumname", boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "filename", boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "tagname", boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "albumcaption", boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "albumcollection", boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "comment", boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, "title", boundValues, hooks);

        sql += " ) ";
    }
    else if (name == "similarity")
    {
        kWarning() << "Search field \"similarity\" is not supported by ImageQueryBuilder";
    }
    else
    {
        kDebug() << "Search field" << name << "not known by this version of ImageQueryBuilder";
        return false;
    }

    return true;
}

void ImageQueryBuilder::addSqlOperator(QString& sql, SearchXml::Operator op, bool isFirst)
{
    if (isFirst)
    {
        if (op == SearchXml::AndNot || op == SearchXml::OrNot)
        {
            sql += "NOT";
        }

        return;
    }

    switch (op)
    {
        case SearchXml::And:
            sql += "AND";
            break;
        case SearchXml::Or:
            sql += "OR";
            break;
        case SearchXml::AndNot:
            sql += "AND NOT";
            break;
        case SearchXml::OrNot:
            sql += "OR NOT";
            break;
    }
}

void ImageQueryBuilder::addSqlRelation(QString& sql, SearchXml::Relation rel)
{
    switch (rel)
    {
        default:
        case SearchXml::Equal:
            sql += '=';
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
            sql += '<';
            break;
        case SearchXml::GreaterThan:
            sql += '>';
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

void ImageQueryBuilder::addNoEffectContent(QString& sql, SearchXml::Operator op)
{
    // add a condition statement with no effect
    switch (op)
    {
        case SearchXml::And:
        case SearchXml::Or:
            sql += " 1 ";
            break;
        case SearchXml::AndNot:
        case SearchXml::OrNot:
            sql += " 0 ";
            break;
    }
}

// ----------- Legacy query description handling -------------- //

class RuleTypeForConversion
{
public:

    RuleTypeForConversion()
        : op(SearchXml::Equal)
    {
    }

    QString             key;
    SearchXml::Relation op;
    QString             val;
};

QString ImageQueryBuilder::convertFromUrlToXml(const KUrl& url) const
{
    int  count = url.queryItem("count").toInt();

    if (count <= 0)
    {
        return QString();
    }

    QMap<int, RuleTypeForConversion> rulesMap;

    for (int i=1; i<=count; ++i)
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
        {
            rule.op = SearchXml::Equal;
        }
        else if (op == "ne")
        {
            rule.op = SearchXml::Unequal;
        }
        else if (op == "lt")
        {
            rule.op = SearchXml::LessThan;
        }
        else if (op == "lte")
        {
            rule.op = SearchXml::LessThanOrEqual;
        }
        else if (op == "gt")
        {
            rule.op = SearchXml::GreaterThan;
        }
        else if (op == "gte")
        {
            rule.op = SearchXml::GreaterThanOrEqual;
        }
        else if (op == "like")
        {
            if (key == "tag")
            {
                rule.op = SearchXml::InTree;
            }
            else
            {
                rule.op = SearchXml::Like;
            }
        }
        else if (op == "nlike")
        {
            if (key == "tag")
            {
                rule.op = SearchXml::NotInTree;
            }
            else
            {
                rule.op = SearchXml::NotLike;
            }
        }

        rule.val = url.queryItem(QString::number(i) + ".val");

        rulesMap.insert(i, rule);
    }

    SearchXmlWriter writer;

    // set an attribute marking this search as converted from 0.9 style search
    writer.writeAttribute("convertedFrom09Url", "true");
    writer.writeGroup();

    QStringList strList = url.path().split(' ', QString::SkipEmptyParts);

    for ( QStringList::const_iterator it = strList.constBegin(); it != strList.constEnd(); ++it )
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

// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------

class RuleType
{
public:

    SKey      key;
    SOperator op;
    QString   val;
};

// -------------------------------------------------------------------------

class SubQueryBuilder
{
public:

    QString build(enum SKey key, enum SOperator op,
                  const QString& passedVal, QList<QVariant>* boundValues) const;
};

// -------------------------------------------------------------------------

QString ImageQueryBuilder::buildQueryFromUrl(const KUrl& url, QList<QVariant>* boundValues) const
{
    int count = url.queryItem("count").toInt();

    if (count <= 0)
    {
        return QString();
    }

    QMap<int, RuleType> rulesMap;

    for (int i=1; i<=count; ++i)
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
            kWarning() << "Unknown rule type: " << key << " passed to kioslave";
            continue;
        }

        if (op == "eq")
        {
            rule.op = EQ;
        }
        else if (op == "ne")
        {
            rule.op = NE;
        }
        else if (op == "lt")
        {
            rule.op = LT;
        }
        else if (op == "lte")
        {
            rule.op = LTE;
        }
        else if (op == "gt")
        {
            rule.op = GT;
        }
        else if (op == "gte")
        {
            rule.op = GTE;
        }
        else if (op == "like")
        {
            rule.op = LIKE;
        }
        else if (op == "nlike")
        {
            rule.op = NLIKE;
        }
        else
        {
            kWarning() << "Unknown op type: " << op << " passed to kioslave";
            continue;
        }

        rule.val = url.queryItem(QString::number(i) + ".val");

        rulesMap.insert(i, rule);
    }

    QString         sqlQuery;
    SubQueryBuilder subQuery;
    QStringList     strList = url.path().split(' ', QString::SkipEmptyParts);

    for ( QStringList::const_iterator it = strList.constBegin(); it != strList.constEnd(); ++it )
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

                        if ( it != todo.constEnd() )
                        {
                            sqlQuery += " OR ";
                        }
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
                               const QString& passedVal, QList<QVariant>* boundValues) const
{
    QString query;
    QString val = passedVal;

    if (op == LIKE || op == NLIKE)
    {
        val = '%' + val + '%';
    }

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
                        "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                        "    WHERE TagsTree.pid = ? or ImageTags.tagid = ? )) ";
                *boundValues << val.toInt() << val.toInt();
            }
            else // op == NLIKE
            {
                query = " (Images.id NOT IN "
                        "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
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
            kWarning() << "KEYWORD Detected which is not possible";
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
        {
            return query;
        }

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

    exact    = false;
    bool ok;
    int  num = str.toInt(&ok);

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
        for (int i=1; i<=12; ++i)
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
