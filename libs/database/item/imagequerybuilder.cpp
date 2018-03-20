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
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagequerybuilder.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QFile>
#include <QDir>
#include <QMap>
#include <QRectF>
#include <QUrl>
#include <QLocale>
#include <QUrlQuery>

// Local includes

#include "metaengine.h"
#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredb.h"
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
    foreach(ImageQueryPostHook* const hook, m_postHooks)
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
    foreach(ImageQueryPostHook* const hook, m_postHooks)
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

    for (int i=1; i<=12; ++i)
    {
        m_shortMonths[i-1] = QLocale().monthName(i, QLocale::ShortFormat).toLower();
        m_longMonths[i-1]  = QLocale().monthName(i, QLocale::LongFormat).toLower();
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
        return buildQueryFromUrl(QUrl(q), boundValues);
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

    qCDebug(DIGIKAM_DATABASE_LOG) << sql;

    return sql;
}

void ImageQueryBuilder::buildGroup(QString& sql, SearchXmlCachingReader& reader,
                                   QList<QVariant> *boundValues, ImageQueryPostHooks* const hooks) const
{
    sql += QLatin1String(" (");

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

    sql += QLatin1String(") ");
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
            return QLatin1Char('%') + str + QLatin1Char('%');
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
                qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";
                return;
            }

            sql += QLatin1String(" (") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
            sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
            sql += QLatin1String(" ?) ");

            *boundValues << values.first() << values.last();
        }
        else
        {
            sql += QLatin1String(" (") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql, relation);
            sql += QLatin1String(" ?) ");
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
                qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";
                return;
            }

            sql += QLatin1String(" (") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
            sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
            sql += QLatin1String(" ?) ");

            *boundValues << values.first() << values.last();
        }
        else
        {
            sql += QLatin1String(" (") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql, relation);
            sql += QLatin1String(" ?) ");
            *boundValues << reader.valueToDouble();
        }
    }

    void addStringField(const QString& name)
    {
        sql += QLatin1String(" (") + name + QLatin1Char(' ');
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += QLatin1String(" ?) ");
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
                qCWarning(DIGIKAM_DATABASE_LOG) << "Date" << reader.value() << "is invalid";
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

            sql += QLatin1String(" (") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql, SearchXml::GreaterThan);
            sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql, SearchXml::LessThan);
            sql += QLatin1String(" ?) ");
        }
        else if (relation == SearchXml::Interval || relation == SearchXml::IntervalOpen)
        {
            QList<QString> values = reader.valueToStringList();

            if (values.size() != 2)
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";
                return;
            }

            sql += QLatin1String(" (") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
            sql += QLatin1String(" ? AND ") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql,
                                              relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
            sql += QLatin1String(" ?) ");

            *boundValues << values.first() << values.last();
        }
        else
        {
            sql += QLatin1String(" (") + name + QLatin1Char(' ');
            ImageQueryBuilder::addSqlRelation(sql, relation);
            sql += QLatin1String(" ?) ");
            *boundValues << reader.value();
        }
    }

    void addChoiceIntField(const QString& name)
    {
        if (relation == SearchXml::OneOf)
        {
            QList<int> values  = reader.valueToIntList();
            bool searchForNull = values.removeAll(-1);
            sql               += QLatin1String(" (") + name + QLatin1String(" IN (");
            CoreDB::addBoundValuePlaceholders(sql, values.size());

            if (searchForNull)
            {
                sql += QLatin1String(") OR ") + name + QLatin1String(" IS NULL");
            }
            else
            {
                sql += QLatin1String(") ");
            }

            foreach(int v, values)
            {
                *boundValues << v;
            }

            sql += QLatin1String(" ) ");
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
            sql += QLatin1String(" (") + name + QLatin1String(" IN (");
            CoreDB::addBoundValuePlaceholders(sql, values.size());
            sql += QLatin1String(") ");

            foreach(const qlonglong& v, values)
            {
                *boundValues << v;
            }

            sql += QLatin1String(" ) ");
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
            sql += QLatin1String("( ");
            bool first = true;

            for (int i=0; i<values.size(); ++i)
            {
                if (!first)
                {
                    sql += QLatin1String("OR ");
                }

                first = false;
                sql += name + QLatin1String(" & ? ");
            }

            if (searchForNull)
            {
                sql += QLatin1String("OR ") + name + QLatin1String(" IS NULL ");
            }

            foreach(int v, values)
            {
                *boundValues << v;
            }

            sql += QLatin1String(" ) ");
        }
        else
        {
            if (relation == SearchXml::Equal)
            {
                sql += QLatin1String(" (") + name + QLatin1String(" & ") + QLatin1String(" ?) ");
            }
            else
            {
                sql += QLatin1String(" (NOT ") + name + QLatin1String(" & ") + QLatin1String(" ?) ");
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
                qCDebug(DIGIKAM_DATABASE_LOG) << "List for OneOf is empty";
                return;
            }

            QStringList simpleValues, wildcards;

            foreach(const QString& value, values)
            {
                if (value.contains(QLatin1String("*")))
                {
                    wildcards << value;
                }
                else
                {
                    simpleValues << value;
                }
            }

            bool firstCondition =  true;
            sql                 += QLatin1String(" (");

            if (!simpleValues.isEmpty())
            {
                firstCondition =  false;
                sql            += name + QLatin1String(" IN (");
                CoreDB::addBoundValuePlaceholders(sql, simpleValues.size());

                foreach(const QString& value, simpleValues)
                {
                    *boundValues << value;
                }

                sql += QLatin1String(" ) ");
            }

            if (!wildcards.isEmpty())
            {
                foreach(QString wildcard, wildcards) // krazy:exclude=foreach
                {
                    ImageQueryBuilder::addSqlOperator(sql, SearchXml::Or, firstCondition);
                    firstCondition = false;
                    wildcard.replace(QLatin1Char('*'), QLatin1Char('%'));
                    sql           += QLatin1Char(' ') + name + QLatin1Char(' ');
                    ImageQueryBuilder::addSqlRelation(sql, SearchXml::Like);
                    sql           += QLatin1String(" ? ");
                    *boundValues << wildcard;
                }
            }

            sql += QLatin1String(") ");
        }
        else
        {
            QString value = reader.value();

            if (relation == SearchXml::Like && value.contains(QLatin1String("*")))
            {
                // Handle special case: * denotes the place if the wildcard,
                // Don't automatically prepend and append %
                sql += QLatin1String(" (") + name + QLatin1Char(' ');
                ImageQueryBuilder::addSqlRelation(sql, SearchXml::Like);
                sql += QLatin1String(" ?) ");
                QString wildcard = reader.value();
                wildcard.replace(QLatin1Char('*'), QLatin1Char('%'));
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
            QStringRef type           = reader.attributes().value(QLatin1String("type"));
            QStringRef distanceString = reader.attributes().value(QLatin1String("distance"));
            // Distance in meters
            double distance           = 100;

            if (!distanceString.isEmpty())
            {
                distance = distanceString.toString().toDouble();
            }

            // Search type, "radius" or "rectangle"
            bool radiusSearch = true;

            if (type == QLatin1String("radius"))
            {
                radiusSearch = true;
            }
            else if (type == QLatin1String("rectangle"))
            {
                radiusSearch = false;
            }

            // Get a list of doubles:
            // Longitude and Latitude in (decimal) degrees
            QList<double> list = reader.valueToDoubleList();

            if (list.size() != 2)
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Relation 'Near' requires a list of two values";
                return;
            }

            double lon = list.at(0);
            double lat = list.at(1);

            sql += QLatin1String(" ( ");

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

            sql += QLatin1String(" ) ");
        }
        else if (relation == SearchXml::Inside)
        {
            // First read attributes
            QStringRef type = reader.attributes().value(QLatin1String("type"));

            // Search type, currently only "rectangle"
            if (type != QLatin1String("rectangle"))
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Relation 'Inside' supports no other type than 'rectangle'";
                return;
            }

            // Get a list of doubles:
            // Longitude and Latitude in (decimal) degrees
            QList<double> list = reader.valueToDoubleList();

            if (list.size() != 4)
            {
                qCWarning(DIGIKAM_DATABASE_LOG) << "Relation 'Inside' requires a list of four values";
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

            sql += QLatin1String(" ( ");
            addRectanglePositionSearch(lon1, lat1, lon2, lat2);
            sql += QLatin1String(" ) ");
        }
    }

    void addRectanglePositionSearch(double lon1, double lat1, double lon2, double lat2) const
    {
        // lon1 is always West of lon2. If the rectangle crosses 180 longitude, we have to treat a special case.
        if (lon1 <= lon2)
        {
            sql += QString::fromUtf8(" ImagePositions.LongitudeNumber > ? AND ImagePositions.LatitudeNumber < ? "
                   " AND ImagePositions.LongitudeNumber < ? AND ImagePositions.LatitudeNumber > ? ");
            *boundValues << lon1 << lat1 << lon2 << lat2;
        }
        else
        {
            // this effectively means splitting the rectangle is two parts, one East, one West
            // to the 180 line. But no need to check for less/greater than -180/180.
            sql += QString::fromUtf8(" (ImagePositions.LongitudeNumber > ? OR ImagePositions.LongitudeNumber < ?) "
                   " AND ImagePositions.LatitudeNumber < ? AND ImagePositions.LatitudeNumber > ? ");
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

    // First catch all noeffect fields. Those are only used for message passing when no Signal-Slot-communication is possible
    if (name.startsWith(QLatin1String("noeffect_")))
    {
        return false;
    }
    else if (name == QLatin1String("albumid"))
    {
        if (relation == SearchXml::Equal || relation == SearchXml::Unequal)
        {
            fieldQuery.addIntField(QLatin1String("Images.album"));
        }
        else if (relation == SearchXml::InTree)
        {
            // see also: CoreDB::getItemNamesInAlbum
            QList<int> ids = reader.valueToIntOrIntList();

            if (ids.isEmpty())
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Relation 'InTree', name 'albumid': No values given";
                return false;
            }

            sql += QString::fromUtf8("(Images.album IN "
                   "   (SELECT DISTINCT id "
                   "    FROM Albums WHERE ");
            bool firstCondition = true;

            foreach(int albumID, ids)
            {
                addSqlOperator(sql, SearchXml::Or, firstCondition);
                firstCondition = false;

                CoreDbAccess access;
                int rootId           = access.db()->getAlbumRootId(albumID);
                QString relativePath = access.db()->getAlbumRelativePath(albumID);

                QString childrenWildcard;

                if (relativePath == QLatin1String("/"))
                {
                    childrenWildcard = QLatin1String("/%");
                }
                else
                {
                    childrenWildcard = relativePath + QLatin1String("/%");
                }

                sql += QString::fromUtf8(" ( albumRoot=? AND (relativePath=? OR relativePath LIKE ?) ) ");
                *boundValues << rootId << relativePath << childrenWildcard;
            }

            sql += QLatin1String(" ))");
        }
        else if (relation == SearchXml::OneOf)
        {
            fieldQuery.addChoiceIntField(QLatin1String("Images.album"));
        }
    }
    else if (name == QLatin1String("albumname"))
    {
        fieldQuery.addStringField(QLatin1String("Albums.relativePath"));
    }
    else if (name == QLatin1String("albumcaption"))
    {
        fieldQuery.addStringField(QLatin1String("Albums.caption"));
    }
    else if (name == QLatin1String("albumcollection"))
    {
        fieldQuery.addChoiceStringField(QLatin1String("Albums.collection"));
    }
    else if (name == QLatin1String("tagid") || name == QLatin1String("labels"))
    {
        if (relation == SearchXml::Equal)
        {
            sql += QString::fromUtf8(" (Images.id IN "
                   "   (SELECT imageid FROM ImageTags "
                   "    WHERE tagid = ?)) ");
            *boundValues << reader.valueToInt();
        }
        else if (relation == SearchXml::Unequal)
        {
            sql += QString::fromUtf8(" (Images.id NOT IN "
                   "   (SELECT imageid FROM ImageTags "
                   "    WHERE tagid = ?)) ");
            *boundValues << reader.valueToInt();
        }
        else if (relation == SearchXml::InTree || relation == SearchXml::NotInTree)
        {
            QList<int> ids = reader.valueToIntOrIntList();

            if (ids.isEmpty())
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "Relation 'InTree', name 'tagid': No values given";
                return false;
            }

            if (relation == SearchXml::InTree)
            {
                sql += QString::fromUtf8(" (Images.id IN ");
            }
            else
            {
                sql += QString::fromUtf8(" (Images.id NOT IN ");
            }

            sql += QString::fromUtf8("   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                   "    WHERE ");

            bool firstCondition = true;

            foreach(int tagID, ids)
            {
                addSqlOperator(sql, SearchXml::Or, firstCondition);
                firstCondition = false;
                sql += QString::fromUtf8(" (TagsTree.pid = ? OR ImageTags.tagid = ? ) ");
                *boundValues << tagID << tagID;
            }

            sql += QString::fromUtf8(" )) ");
        }
        else if (relation == SearchXml::OneOf)
        {
            QList<int> values  = reader.valueToIntList();
            bool searchForNull = values.removeAll(-1);
            sql               += QLatin1String(" (Images.id IN (");

            if (searchForNull)
            {
                sql += QLatin1String(") OR ") + name + QLatin1String(" IS NULL) ");
            }
            else
            {
                sql += QString::fromUtf8(" SELECT imageid FROM ImageTags "
                   "    WHERE tagid IN (");
                CoreDB::addBoundValuePlaceholders(sql, values.size());
                sql += QLatin1String("))) ");
            }

            foreach(int tagID, values)
            {
                *boundValues << tagID;
            }
        }
        else if (relation == SearchXml::AllOf)
        {
            // there must be an entry in ImageTags for every given tag id
            QList<int> ids = reader.valueToIntOrIntList();

            bool firstCondition = true;

            foreach(int tagID, ids)
            {
                addSqlOperator(sql, SearchXml::And, firstCondition);
                firstCondition = false;
                sql += QString::fromUtf8(" (Images.id IN "
                   "   (SELECT imageid FROM ImageTags "
                   "    WHERE tagid = ?)) ");
                *boundValues << tagID;
            }
        }
    }
    else if (name == QLatin1String("tagname"))
    {
        QString tagname = QLatin1Char('%') + reader.value() + QLatin1Char('%');

        if (relation == SearchXml::Equal || relation == SearchXml::Like)
        {
            sql += QString::fromUtf8(" (Images.id IN "
                   "   (SELECT imageid FROM ImageTags "
                   "    WHERE tagid IN "
                   "   (SELECT id FROM Tags WHERE name LIKE ?))) ");
            *boundValues << tagname;
        }
        else if (relation == SearchXml::Unequal || relation == SearchXml::NotLike)
        {
            sql += QString::fromUtf8(" (Images.id NOT IN "
                   "   (SELECT imageid FROM ImageTags "
                   "    WHERE tagid IN "
                   "   (SELECT id FROM Tags WHERE name LIKE ?))) ");
            *boundValues << tagname;
        }
        else if (relation == SearchXml::InTree)
        {
            sql += QString::fromUtf8(" (Images.id IN "
                   "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                   "    WHERE TagsTree.pid = (SELECT id FROM Tags WHERE name LIKE ?) "
                   "    or ImageTags.tagid = (SELECT id FROM Tags WHERE name LIKE ?) )) ");
            *boundValues << tagname << tagname;
        }
        else if (relation == SearchXml::NotInTree)
        {
            sql += QString::fromUtf8(" (Images.id NOT IN "
                   "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                   "    WHERE TagsTree.pid = (SELECT id FROM Tags WHERE name LIKE ?) "
                   "    or ImageTags.tagid = (SELECT id FROM Tags WHERE name LIKE ?) )) ");
            *boundValues << tagname << tagname;
        }
    }
    else if (name == QLatin1String("notag"))
    {
        reader.readToEndOfElement();
        sql += QString::fromUtf8(" (Images.id NOT IN "
               "   (SELECT imageid FROM ImageTags)) ");
    }
    else if (name == QLatin1String("imageid"))
    {
        fieldQuery.addLongListField(QLatin1String("Images.id"));
    }
    else if (name == QLatin1String("filename"))
    {
        fieldQuery.addStringField(QLatin1String("Images.name"));
    }
    else if (name == QLatin1String("modificationdate"))
    {
        fieldQuery.addDateField(QLatin1String("Images.modificationDate"));
    }
    else if (name == QLatin1String("filesize"))
    {
        fieldQuery.addIntField(QLatin1String("Images.fileSize"));
    }
    else if (name == QLatin1String("rating"))
    {
        fieldQuery.addIntField(QLatin1String("ImageInformation.rating"));
    }
    else if (name == QLatin1String("creationdate"))
    {
        fieldQuery.addDateField(QLatin1String("ImageInformation.creationDate"));
    }
    else if (name == QLatin1String("digitizationdate"))
    {
        fieldQuery.addDateField(QLatin1String("ImageInformation.digitizationDate"));
    }
    else if (name == QLatin1String("orientation"))
    {
        fieldQuery.addChoiceIntField(QLatin1String("ImageInformation.orientation"));
    }
    else if (name == QLatin1String("pageorientation"))
    {
        if (relation == SearchXml::Equal)
        {
            int pageOrientation = reader.valueToInt();

            // "1" is landscape, "2" is portrait, "3" is landscape regardless of Exif, "4" is portrait regardless of Exif
            if (pageOrientation == 1)
            {
                sql += QString::fromUtf8(" ( (ImageInformation.orientation <= ? AND ImageInformation.width >= ImageInformation.height) "
                       "  OR (ImageInformation.orientation >= ? AND ImageInformation.width <= ImageInformation.height) ) ");
                *boundValues << MetaEngine::ORIENTATION_VFLIP << MetaEngine::ORIENTATION_ROT_90_HFLIP;
            }
            else if (pageOrientation == 2)
            {
                sql += QString::fromUtf8(" ( (ImageInformation.orientation <= ? AND ImageInformation.width < ImageInformation.height) "
                       "  OR (ImageInformation.orientation >= ? AND ImageInformation.width > ImageInformation.height) ) ");
                *boundValues << MetaEngine::ORIENTATION_VFLIP << MetaEngine::ORIENTATION_ROT_90_HFLIP;
            }
            else if (pageOrientation == 3 || pageOrientation == 4)
            {
                // ignoring Exif orientation
                sql += QString::fromUtf8(" ( ImageInformation.width ");
                ImageQueryBuilder::addSqlRelation(sql, pageOrientation == 3 ? SearchXml::GreaterThanOrEqual : SearchXml::LessThanOrEqual);
                sql += QString::fromUtf8(" ImageInformation.height) ");
            }
        }
    }
    else if (name == QLatin1String("width"))
    {
        sql += QString::fromUtf8(" ( (ImageInformation.orientation <= ? AND ");
        *boundValues << MetaEngine::ORIENTATION_VFLIP;
        fieldQuery.addIntField(QLatin1String("ImageInformation.width"));
        sql += QString::fromUtf8(") OR (ImageInformation.orientation >= ? AND ");
        *boundValues << MetaEngine::ORIENTATION_ROT_90_HFLIP;
        fieldQuery.addIntField(QLatin1String("ImageInformation.height"));
        sql += QString::fromUtf8(" ) ) ");
    }
    else if (name == QLatin1String("height"))
    {
        sql += QString::fromUtf8(" ( (ImageInformation.orientation <= ? AND ");
        *boundValues << MetaEngine::ORIENTATION_VFLIP;
        fieldQuery.addIntField(QLatin1String("ImageInformation.height"));
        sql += QString::fromUtf8(") OR (ImageInformation.orientation >= ? AND ");
        *boundValues << MetaEngine::ORIENTATION_ROT_90_HFLIP;
        fieldQuery.addIntField(QLatin1String("ImageInformation.width"));
        sql += QString::fromUtf8(" ) ) ");
    }
    else if (name == QLatin1String("aspectratioimg"))
    {
        QString query;
        QString readerString = (reader.valueToStringOrStringList()).at(0);

        if(readerString.contains(QRegExp(QLatin1String("^\\d+:\\d+$"))))
        {
            QStringList ratioNum = readerString.split(QLatin1String(":"), QString::SkipEmptyParts);
            int num              = ratioNum.at(0).toInt();
            int denominator = ratioNum.at(1).toInt();
            query                = QString::fromUtf8("abs((ImageInformation.width/CAST(ImageInformation.height as REAL)) - ?)  < 0.1");
            sql                 += QString::fromUtf8(" (") + query + QString::fromUtf8(") ");
            *boundValues << (double)num/denominator;
        }
        else if(readerString.contains(QRegExp(QLatin1String("^\\d+(.\\d+)?$"))))
        {
            query = QString::fromUtf8("abs((ImageInformation.width/CAST(ImageInformation.height as REAL)) - ?)  < 0.1");
            sql  += QString::fromUtf8(" (") + query + QString::fromUtf8(") ");
            *boundValues << readerString.toDouble();
        }
    }
    else if (name == QLatin1String("pixelsize"))
    {
        fieldQuery.addIntField(QLatin1String("(ImageInformation.width * ImageInformation.height)"));
    }
    else if (name == QLatin1String("pixels"))
    {
        fieldQuery.addIntField(QLatin1String("(ImageInformation.width * ImageInformation.height)"));
    }
    else if (name == QLatin1String("format"))
    {
        fieldQuery.addChoiceStringField(QLatin1String("ImageInformation.format"));
    }
    else if (name == QLatin1String("colordepth"))
    {
        fieldQuery.addIntField(QLatin1String("ImageInformation.colorDepth"));
    }
    else if (name == QLatin1String("colormodel"))
    {
        fieldQuery.addIntField(QLatin1String("ImageInformation.colorModel"));
    }
    else if (name == QLatin1String("videoaspectratio"))
    {
        if (relation == SearchXml::OneOf)
        {
            QStringList values = reader.valueToStringList();

            if (values.isEmpty())
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "List for OneOf is empty";
                return false;
            }

            QList<double> ratioValues;

            foreach(const QString& value, values)
            {
                 *boundValues << value;

                 if (value.contains(QLatin1String(":")))
                 {
                     QStringList ratioNum = value.split(QLatin1String(":"), QString::SkipEmptyParts);
                     int num              = ratioNum.at(0).toInt();
                     int denominator      = ratioNum.at(1).toInt();
                     ratioValues << (double)num/denominator;
                 }
            }

            sql += QString::fromUtf8("(VideoMetadata.aspectRatio IN (");
            CoreDB::addBoundValuePlaceholders(sql, values.size());
            sql += QString::fromUtf8(") ");
            QString query = QString::fromUtf8("abs((CAST(VideoMetadata.aspectRatio as REAL) - ?)  < 0.1) ");

            foreach(double value, ratioValues)
            {
                *boundValues << value;
                sql +=  QString::fromUtf8("OR ") + query;
            }

            sql += QString::fromUtf8(") ");
        }
        else
        {
            QString value = reader.value();
            *boundValues << value;

            if (value.contains(QLatin1String(":")))
            {
                QStringList ratioNum = value.split(QLatin1String(":"), QString::SkipEmptyParts);
                int num              = ratioNum.at(0).toInt();
                int denominator      = ratioNum.at(1).toInt();
                *boundValues << (double)num/denominator;
            }

            sql += QString::fromUtf8("(VideoMetadata.aspectRatio=? OR abs((CAST(VideoMetadata.aspectRatio as REAL) - ?)  < 0.1 )) ");
        }
    }
    else if (name == QLatin1String("videoaudiobitrate"))
    {
        //fieldQuery.addIntField("VideoMetadata.audioBitRate");
        QList<int> values = reader.valueToIntList();

        if (values.size() != 2)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";
            return false;
        }

        sql += QString::fromUtf8(" ( CAST(VideoMetadata.audioBitRate AS INTEGER)");
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
        sql += QString::fromUtf8(" ? AND CAST(VideoMetadata.audioBitRate AS INTEGER)");
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
        sql += QString::fromUtf8(" ?) ");

        *boundValues << values.first() << values.last();
    }
    else if (name == QLatin1String("videoaudiochanneltype"))
    {
        if (relation == SearchXml::OneOf)
        {
            QStringList values = reader.valueToStringList();

            if (values.isEmpty())
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "List for OneOf is empty";
                return false;
            }

            foreach(const QString& value, values)
            {
                 *boundValues << value;

                 if (value == QLatin1String("1"))
                 {
                     *boundValues << QLatin1String("Mono");
                 }
                 else if (value == QLatin1String("2"))
                 {
                     *boundValues << QLatin1String("Stereo");
                 }
            }

            sql += QString::fromUtf8("(VideoMetadata.audioChannelType IN (");
            CoreDB::addBoundValuePlaceholders(sql, boundValues->size());
            sql += QString::fromUtf8(")) ");
        }
        else
        {
            QString value = reader.value();
            *boundValues << value;

            if (value == QLatin1String("1"))
            {
                *boundValues << QLatin1String("Mono");
            }
            else if (value == QLatin1String("2"))
            {
                *boundValues << QLatin1String("Stereo");
            }

            sql += QString::fromUtf8("(VideoMetadata.audioChannelType IN (");
            CoreDB::addBoundValuePlaceholders(sql, boundValues->size());
            sql += QString::fromUtf8(")) ");
        }
    }
    else if (name == QLatin1String("videoaudioCodec"))
    {
        fieldQuery.addChoiceStringField(QLatin1String("VideoMetadata.audioCompressor"));
    }
    else if (name == QLatin1String("videoduration"))
    {
        QList<int> values = reader.valueToIntList();

        if (values.size() != 2)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";
            return false;
        }

        sql += QString::fromUtf8(" ( CAST(VideoMetadata.duration AS INTEGER)");
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
        sql += QString::fromUtf8(" ? AND CAST(VideoMetadata.duration AS INTEGER)");
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
        sql += QString::fromUtf8(" ?) ");

        *boundValues << values.first()*1000 << values.last()*1000;
    }
    else if (name == QLatin1String("videoframerate"))
    {
        //fieldQuery.addChoiceStringField("VideoMetadata.frameRate");
        QList<double> values = reader.valueToDoubleList();

        if (values.size() != 2)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Relation Interval requires a list of two values";
            return false;
        }

        sql += QString::fromUtf8(" ( CAST(VideoMetadata.frameRate AS REAL)");
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::GreaterThanOrEqual : SearchXml::GreaterThan);
        sql += QString::fromUtf8(" ? AND CAST(VideoMetadata.frameRate AS REAL)");
        ImageQueryBuilder::addSqlRelation(sql,
                                          relation == SearchXml::Interval ? SearchXml::LessThanOrEqual : SearchXml::LessThan);
        sql += QString::fromUtf8(" ?) ");

        *boundValues << values.first() << values.last();
    }
    else if (name == QLatin1String("videocodec"))
    {
        if (relation == SearchXml::OneOf)
        {
            QStringList values = reader.valueToStringList();

            if (values.isEmpty())
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "List for OneOf is empty";
                return false;
            }

            foreach(const QString& value, values)
            {
                sql += QString::fromUtf8("( Upper(VideoMetadata.videoCodec) LIKE '%") + value.toUpper() + QString::fromUtf8("%' ");

                if (value != values.last())
                {
                    sql += QString::fromUtf8("OR ");
                }
            }

            sql += QString::fromUtf8(") ");
        }
        else
        {
            QString value = reader.value();
            sql += QString::fromUtf8("(Upper(VideoMetadata.videoCodec) LIKE '%") + value.toUpper() + QString::fromUtf8("%') ");
        }
    }
    else if (name == QLatin1String("make"))
    {
        fieldQuery.addChoiceStringField(QLatin1String("ImageMetadata.make"));
    }
    else if (name == QLatin1String("model"))
    {
        fieldQuery.addChoiceStringField(QLatin1String("ImageMetadata.model"));
    }
    else if (name == QLatin1String("lenses"))
    {
        fieldQuery.addChoiceStringField(QLatin1String("ImageMetadata.lens"));
    }
    else if (name == QLatin1String("aperture"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImageMetadata.aperture"));
    }
    else if (name == QLatin1String("focallength"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImageMetadata.focalLength"));
    }
    else if (name == QLatin1String("focallength35"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImageMetadata.focalLength35"));
    }
    else if (name == QLatin1String("exposuretime"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImageMetadata.exposureTime"));
    }
    else if (name == QLatin1String("exposureprogram"))
    {
        fieldQuery.addChoiceIntField(QLatin1String("ImageMetadata.exposureProgram"));
    }
    else if (name == QLatin1String("exposuremode"))
    {
        fieldQuery.addChoiceIntField(QLatin1String("ImageMetadata.exposureMode"));
    }
    else if (name == QLatin1String("sensitivity"))
    {
        fieldQuery.addIntField(QLatin1String("ImageMetadata.sensitivity"));
    }
    else if (name == QLatin1String("flashmode"))
    {
        fieldQuery.addIntBitmaskField(QLatin1String("ImageMetadata.flash"));
    }
    else if (name == QLatin1String("whitebalance"))
    {
        fieldQuery.addChoiceIntField(QLatin1String("ImageMetadata.whiteBalance"));
    }
    else if (name == QLatin1String("whitebalancecolortemperature"))
    {
        fieldQuery.addIntField(QLatin1String("ImageMetadata.whiteBalanceColorTemperature"));
    }
    else if (name == QLatin1String("meteringmode"))
    {
        fieldQuery.addChoiceIntField(QLatin1String("ImageMetadata.meteringMode"));
    }
    else if (name == QLatin1String("subjectdistance"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImageMetadata.subjectDistance"));
    }
    else if (name == QLatin1String("subjectdistancecategory"))
    {
        fieldQuery.addChoiceIntField(QLatin1String("ImageMetadata.subjectDistanceCategory"));
    }
    else if (name == QLatin1String("position"))
    {
        fieldQuery.addPosition();
    }
    else if (name == QLatin1String("latitude"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImagePositions.latitudeNumber"));
    }
    else if (name == QLatin1String("longitude"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImagePositions.longitudeNumber"));
    }
    else if (name == QLatin1String("altitude"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImagePositions.altitude"));
    }
    else if (name == QLatin1String("positionorientation"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImagePositions.orientation"));
    }
    else if (name == QLatin1String("positiontilt"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImagePositions.tilt"));
    }
    else if (name == QLatin1String("positionroll"))
    {
        fieldQuery.addDoubleField(QLatin1String("ImagePositions.roll"));
    }
    else if (name == QLatin1String("positiondescription"))
    {
        fieldQuery.addStringField(QLatin1String("ImagePositions.description"));
    }
    else if (name == QLatin1String("nogps"))
    {
        sql += QString::fromUtf8(" (ImagePositions.latitudeNumber IS NULL AND ImagePositions.longitudeNumber IS NULL) ");
    }
    else if (name == QLatin1String("comment"))
    {
        sql += QString::fromUtf8(" (Images.id IN "
               " (SELECT imageid FROM ImageComments "
               "  WHERE type=? AND comment ");
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += QString::fromUtf8(" ?)) ");
        *boundValues << DatabaseComment::Comment << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == QLatin1String("commentauthor"))
    {
        sql += QString::fromUtf8(" (Images.id IN "
               " (SELECT imageid FROM ImageComments "
               "  WHERE type=? AND author ");
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += QString::fromUtf8(" ?)) ");
        *boundValues << DatabaseComment::Comment << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == QLatin1String("headline"))
    {
        sql += QString::fromUtf8(" (Images.id IN "
               " (SELECT imageid FROM ImageComments "
               "  WHERE type=? AND comment ");
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += QString::fromUtf8(" ?)) ");
        *boundValues << DatabaseComment::Headline << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == QLatin1String("title"))
    {
        sql += QString::fromUtf8(" (Images.id IN "
               " (SELECT imageid FROM ImageComments "
               "  WHERE type=? AND comment ");
        ImageQueryBuilder::addSqlRelation(sql, relation);
        sql += QString::fromUtf8(" ?)) ");
        *boundValues << DatabaseComment::Title << fieldQuery.prepareForLike(reader.value());
    }
    else if (name == QLatin1String("imagetagproperty"))
    {
        if (relation == SearchXml::Equal || relation == SearchXml::InTree)
        {
            // First, read attributes
            QStringRef tagAttribute = reader.attributes().value(QLatin1String("tagid"));
            int tagId               = 0;

            if (!tagAttribute.isEmpty())
            {
                tagId = tagAttribute.toString().toInt();
            }

            // read values: one or two strings
            QStringList values = reader.valueToStringOrStringList();

            if (values.size() < 1 || values.size() > 2)
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << "The imagetagproperty field requires one value (property) or two values (property, value).";
                return false;
            }

            QString selectQuery;
            // %1 is resolved to either "ImageTagProperties." or the empty string
            if (tagId)
            {
                if (relation == SearchXml::Equal)
                {
                    selectQuery += QString::fromUtf8("%1tagid=? AND ");
                    *boundValues << tagId;
                }
                else // InTree
                {
                    selectQuery += QString::fromUtf8("(%1tagid=? OR %1tagid IN (SELECT id FROM TagsTree WHERE pid=?)) AND ");
                    *boundValues << tagId << tagId;
                }
            }

            if (values.size() == 1)
            {
                selectQuery += QString::fromUtf8("%1property=? ");
                *boundValues << values.first();
            }
            else if (values.size() == 2)
            {
                selectQuery += QString::fromUtf8("%1property=? AND %1value ");
                ImageQueryBuilder::addSqlRelation(selectQuery, relation);
                selectQuery += QString::fromUtf8(" ? ");
                *boundValues << values.at(0) << fieldQuery.prepareForLike(values.at(1));
            }

            // This indicates that the ImageTagProperties is joined in the SELECT query,
            // so one entry is listed for each property entry (not for each image id)
            if (m_imageTagPropertiesJoined)
            {
                sql += QString::fromUtf8(" ( ");
                sql += selectQuery.arg(QString::fromUtf8("ImageTagProperties."));
                sql += QString::fromUtf8(" ) ");
            }
            else
            {
                sql += QString::fromUtf8(" (Images.id IN "
                       " (SELECT imageid FROM ImageTagProperties WHERE ");
                sql += selectQuery.arg(QString());
                sql += QString::fromUtf8(" )) ");
            }
        }
    }
    else if (name == QLatin1String("keyword"))
    {
        // keyword is the common search in the text fields

        sql += QLatin1String(" ( ");

        addSqlOperator(sql, SearchXml::Or, true);
        buildField(sql, reader, QLatin1String("albumname"), boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, QLatin1String("filename"), boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, QLatin1String("tagname"), boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, QLatin1String("albumcaption"), boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, QLatin1String("albumcollection"), boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, QLatin1String("comment"), boundValues, hooks);

        addSqlOperator(sql, SearchXml::Or, false);
        buildField(sql, reader, QLatin1String("title"), boundValues, hooks);

        sql += QLatin1String(" ) ");
    }
    else if (name == QLatin1String("similarity"))
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Search field \"similarity\" is not supported by ImageQueryBuilder";
    }
    else
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Search field" << name << "not known by this version of ImageQueryBuilder";
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
            sql += QLatin1String("NOT");
        }

        return;
    }

    switch (op)
    {
        case SearchXml::And:
            sql += QLatin1String("AND");
            break;
        case SearchXml::Or:
            sql += QLatin1String("OR");
            break;
        case SearchXml::AndNot:
            sql += QLatin1String("AND NOT");
            break;
        case SearchXml::OrNot:
            sql += QLatin1String("OR NOT");
            break;
    }
}

void ImageQueryBuilder::addSqlRelation(QString& sql, SearchXml::Relation rel)
{
    switch (rel)
    {
        default:
        case SearchXml::Equal:
            sql += QLatin1Char('=');
            break;
        case SearchXml::Unequal:
            sql += QLatin1String("<>");
            break;
        case SearchXml::Like:
            sql += QLatin1String("LIKE");
            break;
        case SearchXml::NotLike:
            sql += QLatin1String("NOT LIKE");
            break;
        case SearchXml::LessThan:
            sql += QLatin1Char('<');
            break;
        case SearchXml::GreaterThan:
            sql += QLatin1Char('>');
            break;
        case SearchXml::LessThanOrEqual:
            sql += QLatin1String("<=");
            break;
        case SearchXml::GreaterThanOrEqual:
            sql += QLatin1String(">=");
            break;
        case SearchXml::OneOf:
            sql += QLatin1String("IN");
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
            sql += QLatin1String(" 1 ");
            break;
        case SearchXml::AndNot:
        case SearchXml::OrNot:
            sql += QLatin1String(" 0 ");
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

QString ImageQueryBuilder::convertFromUrlToXml(const QUrl& url) const
{
    int  count = QUrlQuery(url).queryItemValue(QLatin1String("count")).toInt();

    if (count <= 0)
    {
        return QString();
    }

    QMap<int, RuleTypeForConversion> rulesMap;

    for (int i=1; i<=count; ++i)
    {
        RuleTypeForConversion rule;

        QString key = QUrlQuery(url).queryItemValue(QString::number(i) + QLatin1String(".key")).toLower();
        QString op  = QUrlQuery(url).queryItemValue(QString::number(i) + QLatin1String(".op")).toLower();

        if (key == QLatin1String("album"))
        {
            rule.key = QLatin1String("albumid");
        }
        else if (key == QLatin1String("imagename"))
        {
            rule.key = QLatin1String("filename");
        }
        else if (key == QLatin1String("imagecaption"))
        {
            rule.key = QLatin1String("comment");
        }
        else if (key == QLatin1String("imagedate"))
        {
            rule.key = QLatin1String("creationdate");
        }
        else if (key == QLatin1String("tag"))
        {
            rule.key = QLatin1String("tagid");
        }
        else
        {
            // other field names did not change:
            // albumname, albumcaption, albumcollection, tagname, keyword, rating
            rule.key = key;
        }

        if (op == QLatin1String("eq"))
        {
            rule.op = SearchXml::Equal;
        }
        else if (op == QLatin1String("ne"))
        {
            rule.op = SearchXml::Unequal;
        }
        else if (op == QLatin1String("lt"))
        {
            rule.op = SearchXml::LessThan;
        }
        else if (op == QLatin1String("lte"))
        {
            rule.op = SearchXml::LessThanOrEqual;
        }
        else if (op == QLatin1String("gt"))
        {
            rule.op = SearchXml::GreaterThan;
        }
        else if (op == QLatin1String("gte"))
        {
            rule.op = SearchXml::GreaterThanOrEqual;
        }
        else if (op == QLatin1String("like"))
        {
            if (key == QLatin1String("tag"))
            {
                rule.op = SearchXml::InTree;
            }
            else
            {
                rule.op = SearchXml::Like;
            }
        }
        else if (op == QLatin1String("nlike"))
        {
            if (key == QLatin1String("tag"))
            {
                rule.op = SearchXml::NotInTree;
            }
            else
            {
                rule.op = SearchXml::NotLike;
            }
        }

        rule.val = QUrlQuery(url).queryItemValue(QString::number(i) + QLatin1String(".val"));

        rulesMap.insert(i, rule);
    }

    SearchXmlWriter writer;

    // set an attribute marking this search as converted from 0.9 style search
    writer.writeAttribute(QLatin1String("convertedFrom09Url"), QLatin1String("true"));
    writer.writeGroup();

    QStringList strList = url.path().split(QLatin1Char(' '), QString::SkipEmptyParts);

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

            if (expr == QLatin1String("AND"))
            {
                // add another field
            }
            else if (expr == QLatin1String("OR"))
            {
                // open a new group
                writer.finishGroup();
                writer.writeGroup();
                writer.setGroupOperator(SearchXml::Or);
            }
            else if (expr == QLatin1String("("))
            {
                // open a subgroup
                writer.writeGroup();
            }
            else if (expr == QLatin1String(")"))
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

QString ImageQueryBuilder::buildQueryFromUrl(const QUrl& url, QList<QVariant>* boundValues) const
{
    int count = QUrlQuery(url).queryItemValue(QLatin1String("count")).toInt();

    if (count <= 0)
    {
        return QString();
    }

    QMap<int, RuleType> rulesMap;

    for (int i=1; i<=count; ++i)
    {
        RuleType rule;

        QString key = QUrlQuery(url).queryItemValue(QString::number(i) + QLatin1String(".key")).toLower();
        QString op  = QUrlQuery(url).queryItemValue(QString::number(i) + QLatin1String(".op")).toLower();

        if (key == QLatin1String("album"))
        {
            rule.key = ALBUM;
        }
        else if (key == QLatin1String("albumname"))
        {
            rule.key = ALBUMNAME;
        }
        else if (key == QLatin1String("albumcaption"))
        {
            rule.key = ALBUMCAPTION;
        }
        else if (key == QLatin1String("albumcollection"))
        {
            rule.key = ALBUMCOLLECTION;
        }
        else if (key == QLatin1String("imagename"))
        {
            rule.key = IMAGENAME;
        }
        else if (key == QLatin1String("imagecaption"))
        {
            rule.key = IMAGECAPTION;
        }
        else if (key == QLatin1String("imagedate"))
        {
            rule.key = IMAGEDATE;
        }
        else if (key == QLatin1String("tag"))
        {
            rule.key = TAG;
        }
        else if (key == QLatin1String("tagname"))
        {
            rule.key = TAGNAME;
        }
        else if (key == QLatin1String("keyword"))
        {
            rule.key = KEYWORD;
        }
        else if (key == QLatin1String("rating"))
        {
            rule.key = RATING;
        }
        else
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Unknown rule type: " << key << " passed to kioslave";
            continue;
        }

        if (op == QLatin1String("eq"))
        {
            rule.op = EQ;
        }
        else if (op == QLatin1String("ne"))
        {
            rule.op = NE;
        }
        else if (op == QLatin1String("lt"))
        {
            rule.op = LT;
        }
        else if (op == QLatin1String("lte"))
        {
            rule.op = LTE;
        }
        else if (op == QLatin1String("gt"))
        {
            rule.op = GT;
        }
        else if (op == QLatin1String("gte"))
        {
            rule.op = GTE;
        }
        else if (op == QLatin1String("like"))
        {
            rule.op = LIKE;
        }
        else if (op == QLatin1String("nlike"))
        {
            rule.op = NLIKE;
        }
        else
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Unknown op type: " << op << " passed to dbjob";
            continue;
        }

        rule.val = QUrlQuery(url).queryItemValue(QString::number(i) + QLatin1String(".val"));

        rulesMap.insert(i, rule);
    }

    QString         sqlQuery;
    SubQueryBuilder subQuery;
    QStringList     strList = url.path().split(QLatin1Char(' '), QString::SkipEmptyParts);

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

                    sqlQuery += QLatin1Char('(');
                    QList<SKey>::const_iterator it = todo.constBegin();

                    while ( it != todo.constEnd() )
                    {
                        sqlQuery += subQuery.build(*it, rule.op, rule.val, boundValues);
                        ++it;

                        if ( it != todo.constEnd() )
                        {
                            sqlQuery += QLatin1String(" OR ");
                        }
                    }

                    sqlQuery += QLatin1Char(')');
                }
            }
            else
            {
                sqlQuery += subQuery.build(rule.key, rule.op, rule.val, boundValues);
            }
        }
        else
        {
            sqlQuery += QLatin1Char(' ') + *it + QLatin1Char(' ');
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
        val = QLatin1Char('%') + val + QLatin1Char('%');
    }

    switch (key)
    {
        case(ALBUM):
        {
            query = QString::fromUtf8(" (Images.dirid $$##$$ ?) ");
            *boundValues << val;
            break;
        }
        case(ALBUMNAME):
        {
            query = QString::fromUtf8(" (Images.dirid IN "
                    "  (SELECT id FROM Albums WHERE url $$##$$ ?)) ");
            *boundValues << val;
            break;
        }
        case(ALBUMCAPTION):
        {
            query = QString::fromUtf8(" (Images.dirid IN "
                    "  (SELECT id FROM Albums WHERE caption $$##$$ ?)) ");
            *boundValues << val;
            break;
        }
        case(ALBUMCOLLECTION):
        {
            query = QString::fromUtf8(" (Images.dirid IN "
                    "  (SELECT id FROM Albums WHERE collection $$##$$ ?)) ");
            *boundValues << val;
            break;
        }
        case(TAG):
        {
            if (op == EQ)
            {
                query = QString::fromUtf8(" (Images.id IN "
                        "   (SELECT imageid FROM ImageTags "
                        "    WHERE tagid = ?)) ");
                *boundValues << val.toInt();
            }
            else if (op == NE)
            {
                query = QString::fromUtf8(" (Images.id NOT IN "
                        "   (SELECT imageid FROM ImageTags "
                        "    WHERE tagid = ?)) ");
                *boundValues << val.toInt();
            }
            else if (op == LIKE)
            {
                query = QString::fromUtf8(" (Images.id IN "
                        "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                        "    WHERE TagsTree.pid = ? or ImageTags.tagid = ? )) ");
                *boundValues << val.toInt() << val.toInt();
            }
            else // op == NLIKE
            {
                query = QString::fromUtf8(" (Images.id NOT IN "
                        "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                        "    WHERE TagsTree.pid = ? or ImageTags.tagid = ? )) ");
                *boundValues << val.toInt() << val.toInt();
            }

            //         query = QString::fromUtf8(" (Images.id IN "
            //                 "   (SELECT imageid FROM ImageTags "
            //                 "    WHERE tagid $$##$$ ?)) ");

            break;
        }
        case(TAGNAME):
        {
            query = QString::fromUtf8(" (Images.id IN "
                    "  (SELECT imageid FROM ImageTags "
                    "   WHERE tagid IN "
                    "   (SELECT id FROM Tags WHERE name $$##$$ ?))) ");
            *boundValues << val;
            break;
        }
        case(IMAGENAME):
        {
            query = QString::fromUtf8(" (Images.name $$##$$ ?) ");
            *boundValues << val;
            break;
        }
        case(IMAGECAPTION):
        {
            query = QString::fromUtf8(" (Images.caption $$##$$ ?) ");
            *boundValues << val;
            break;
        }
        case(IMAGEDATE):
        {
            query = QString::fromUtf8(" (Images.datetime $$##$$ ?) ");
            *boundValues << val;
            break;
        }
        case (KEYWORD):
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "KEYWORD Detected which is not possible";
            break;
        }
        case(RATING):
        {
            query = QString::fromUtf8(" (ImageProperties.value $$##$$ ? and ImageProperties.property='Rating') ");
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
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("="));
                break;
            }
            case(NE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("<>"));
                break;
            }
            case(LT):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("<"));
                break;
            }
            case(GT):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8(">"));
                break;
            }
            case(LTE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("<="));
                break;
            }
            case(GTE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8(">="));
                break;
            }
            case(LIKE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("LIKE"));
                break;
            }
            case(NLIKE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("NOT LIKE"));
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

        query = QString::fromUtf8(" (Images.datetime > ? AND Images.datetime < ?) ");
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
            return QString::fromUtf8("%1-%-%").arg(num);
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
                monGlob = QString::fromUtf8("%-") + monGlob + QString::fromUtf8("-%");
                return monGlob;
            }
        }
    }

    return QString();
}

}  // namespace Digikam
