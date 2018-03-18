/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-05
 * Description : Geodetic tools
 *
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef GEODETIC_TOOLS_H
#define GEODETIC_TOOLS_H

// C++ includes

#include <cmath>

// Qt includes

#include <QString>
#include <QPointF>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

namespace Coordinates
{

/// Converting between radians and degrees

inline double toRadians(double deg)
{
    return (deg * M_PI / 180.0);
}

inline double toRadiansFactor()
{
    return (M_PI / 180.0);
}

inline double toDegrees(double rad)
{
    return (rad * 180.0 / M_PI);
}

inline double toDegreesFactor()
{
    return (180.0 / M_PI);
}

} // namespace Coordinates

// ------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT Ellipsoid
{
    /**
    * Geometric figure that can be used to describe the approximate shape of the earth.
    * In mathematical terms, it is a surface formed by the rotation of an ellipse about
    * its minor axis. An ellipsoid requires two defining parameters:
    *  - semi-major axis and inverse flattening, or
    *  - semi-major axis and semi-minor axis.
    */

public:
    /**
    * WGS 1984 ellipsoid with axis in metres. This ellipsoid is used
    * in GPS systems and is the default for most <code>org.geotools</code> packages.
    */
    static Ellipsoid WGS84();

    /**
    * GRS 80 ellipsoid with axis in metres.
    *
    * @since 2.2
    */
    static Ellipsoid GRS80();

    /**
    * International 1924 ellipsoid with axis in metres.
    */
    static Ellipsoid INTERNATIONAL_1924();

    /**
    * Clarke 1866 ellipsoid with axis in metres.
    *
    * @since 2.2
    */
    static Ellipsoid CLARKE_1866();

    /**
    * A sphere with a radius of 6371000 metres. Spheres use a simpler
    * algorithm for orthodromic distance computation, which
    * may be faster and more robust.
    */
    static Ellipsoid SPHERE();


    /**
    * Constructs a new ellipsoid using the specified axis length.
    *
    * @param name              The ellipsoid name.
    * @param semiMajorAxis The equatorial radius.
    * @param semiMinorAxis The polar radius.
    */
    static Ellipsoid createEllipsoid(const QString& name,
                                     double semiMajorAxis, double semiMinorAxis);

    /**
    * Constructs a new ellipsoid using the specified axis length and inverse flattening value.
    *
    * @param name              The ellipsoid name.
    * @param semiMajorAxis     The equatorial radius.
    * @param inverseFlattening The inverse flattening value.
    *                          values.
    */
    static Ellipsoid createFlattenedSphere(const QString& name,
                                           double semiMajorAxis, double inverseFlattening);

    /**
    * Length of the semi-major axis of the ellipsoid. This is the
    * equatorial radius in axis linear unit.
    *
    * @return Length of semi-major axis.
    */
    double semiMajorAxis() const;

    /**
    * Length of the semi-minor axis of the ellipsoid. This is the
    * polar radius in axis linear unit.
    *
    * @return Length of semi-minor axis.
    */
    double semiMinorAxis() const;

    /**
    * The ratio of the distance between the center and a focus of the ellipse
    * to the length of its semimajor axis. The eccentricity can alternately be
    * computed from the equation: e=sqrt(2f-f^2).
    */
    double eccentricity() const;

    /**
    * Returns the value of the inverse of the flattening constant. Flattening is a value
    * used to indicate how closely an ellipsoid approaches a spherical shape. The inverse
    * flattening is related to the equatorial/polar radius by the formula
    *
    * ivf=r_e/(r_e-r_p).
    *
    * For perfect spheres (i.e. if isSphere returns @c true),
    * the DoublePOSITIVE_INFINITY value is used.
    *
    * @return The inverse flattening value.
    */
    double inverseFlattening() const;

    /**
    * Indicates if the inverse flattening is definitive for
    * this ellipsoid. Some ellipsoids use the IVF as the defining value, and calculate the polar
    * radius whenever asked. Other ellipsoids use the polar radius to calculate the IVF whenever
    * asked. This distinction can be important to avoid floating-point rounding errors.
    *
    * @return @c true if the inverse flattening is
    *         definitive, or @c false if the polar radius
    *         is definitive.
    */
    bool isIvfDefinitive() const;

    /**
    * @c true if the ellipsoid is degenerate and is actually a sphere. The sphere is
    * completely defined by the semi-major axis, which is the
    * radius of the sphere.
    *
    * @return @c true if the ellipsoid is degenerate and is actually a sphere.
    */
    bool isSphere() const;

    /**
    * Returns the orthodromic distance between two geographic coordinates.
    * The orthodromic distance is the shortest distance between two points
    * on a sphere's surface. The orthodromic path is always on a great circle.
    * This is different from the loxodromic distance, which is a
    * longer distance on a path with a constant direction on the compass.
    *
    * @param  x1 Longitude of first  point (in decimal degrees).
    * @param  y1 Latitude  of first  point (in decimal degrees).
    * @param  x2 Longitude of second point (in decimal degrees).
    * @param  y2 Latitude  of second point (in decimal degrees).
    * @return The orthodromic distance (in the units of this ellipsoid's axis).
    */
    double orthodromicDistance(double x1, double y1, double x2, double y2);

    /**
    * Returns the Radius Of Curvature for the given latitude,
    * using the geometric mean of two radii of
    * curvature for all azimuths.
    * @param latitude in degrees
    */
    double radiusOfCurvature(double latitude);

protected:
    /**
    * Constructs a new ellipsoid using the specified axis length. The properties map is
    * given unchanged to the AbstractIdentifiedObjectAbstractIdentifiedObject(Map)
    * super-class constructor.
    *
    * @param semiMajorAxis     The equatorial radius.
    * @param semiMinorAxis     The polar radius.
    * @param inverseFlattening The inverse of the flattening value.
    * @param ivfDefinitive     @c true if the inverse flattening is definitive.
    *
    * @see createEllipsoid
    * @see createFlattenedSphere
    */
    Ellipsoid(const QString& name, double semiMajorAxis, double  semiMinorAxis,
              double inverseFlattening, bool ivfDefinitive);
    Ellipsoid(const QString& name, double radius, bool ivfDefinitive);

    QString name;

    /**
    * The equatorial radius.
    * @see getSemiMajorAxis
    */
    double m_semiMajorAxis;

    /**
    * The polar radius.
    * @see getSemiMinorAxis
    */
    double m_semiMinorAxis;

    /**
    * The inverse of the flattening value, or DBL_MAX
    * if the ellipsoid is a sphere.
    *
    * @see getInverseFlattening
    */
    double m_inverseFlattening;

    /**
    * Tells if the Inverse Flattening definitive for this ellipsoid.
    *
    * @see isIvfDefinitive
    */
    bool m_ivfDefinitive;

    bool m_isSphere;
};

// ------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT GeodeticCalculator
{
    /**
     * Performs geodetic calculations on an ellipsoid. This class encapsulates
     * a generic ellipsoid and calculates the following properties:
     *
     *
     *   Distance and azimuth between two points.
     *   Point located at a given distance and azimuth from an other point.
     *
     *
     * The calculation use the following information:
     *
     *
     *   The starting position (setStartingPosition), which is always considered valid.
     *       It is initially set at (0,0) and can only be changed to another legitimate value.
     *   Only one of the following:
     *
     *         The destination position (setDestinationPosition), or
     *         An azimuth and distance (setDirection).
     *
     *       The latest one set overrides the other and determines what will be calculated.
     *
     *
     */

public:

    explicit GeodeticCalculator(const Ellipsoid& e = Ellipsoid::WGS84());


    /**
    * Returns the referenced ellipsoid.
    */
    Ellipsoid ellipsoid() const;

    /**
    * Set the starting point in geographic coordinates.
    * The azimuth, the orthodromic distance and the destination point
    * are discarded. They will need to be specified again.
    * Coordinates positive North and East.
    *
    * @param  longitude The longitude in decimal degrees between -180 and +180°
    * @param  latitude  The latitude  in decimal degrees between  -90 and  +90°
    */
    void setStartingGeographicPoint(double longitude, double latitude);

    /**
    * Set the destination point in geographic coordinates. The azimuth and distance values
    * will be updated as a side effect of this call. They will be recomputed the next time
    * getAzimuth() or getOrthodromicDistance() are invoked.
    * Coordinates positive North and East.
    *
    * @param  longitude The longitude in decimal degrees between -180 and +180°
    * @param  latitude  The latitude in decimal degrees between  -90 and  +90°
    *
    */
    void setDestinationGeographicPoint(double longitude, double latitude);

    /**
     * Returns the destination point. This method returns the point set by the last
     * call to a setDestinationGeographicPoint(...)
     * method, except if setDirection(...) has been
     * invoked after. In this later case, the destination point will be computed from the
     * starting point to the azimuth and distance specified.
     * Coordinates positive North and East.
     *
     * @return The destination point. The x and y coordinates
     *         are the longitude and latitude in decimal degrees, respectively.
     */
    bool destinationGeographicPoint(double* longitude, double* latitude);
    QPointF destinationGeographicPoint();

    /**
    * Set the azimuth and the distance from the startingGeographicPoint
    * starting point. The destination point will be updated as a side effect of this call.
    * It will be recomputed the next time destinationGeographicPoint() is invoked.
    * Azimuth 0° North.
    *
    * @param  azimuth The azimuth in decimal degrees from -180° to 180°.
    * @param  distance The orthodromic distance in the same units as the ellipsoid axis.
    */
    void setDirection(double azimuth, double distance);

    /**
    * Returns the azimuth. This method returns the value set by the last call to
    * <code>setDirection(double,double) setDirection(azimuth,distance)</code>,
    * except if <code>setDestinationGeographicPoint(double,double)
    * setDestinationGeographicPoint(...)</code> has been invoked after. In this later case, the
    * azimuth will be computed from the startingGeographicPoint starting point
    * to the destination point.
    *
    * @return The azimuth, in decimal degrees from -180° to +180°.
    */
    double azimuth();

    /**
    * Returns the orthodromic distance. This method returns the value set by the last call to
    * <code>setDirection(double,double) setDirection(azimuth,distance)</code>,
    * except if <code>setDestinationGeographicPoint(double,double)
    * setDestinationGeographicPoint(...)</code> has been invoked after. In this later case, the
    * distance will be computed from the startingGeographicPoint starting point
    * to the destination point.
    *
    * @return The orthodromic distance, in the same units as the
    *         getEllipsoid ellipsoid axis.
    */
    double orthodromicDistance();

    /**
    * Computes the orthodromic distance using the algorithm implemented in the Geotools's
    * ellipsoid class (if available), and check if the error is smaller than some tolerance
    * error.
    */
    bool checkOrthodromicDistance();

    /**
    * Computes the destination point from the starting
    * point, the azimuth and the orthodromic distance.
    */
    bool computeDestinationPoint();

    /**
    * Calculates the meridian arc length between two points in the same meridian
    * in the referenced ellipsoid.
    *
    * @param  latitude1 The latitude of the first  point (in decimal degrees).
    * @param  latitude2 The latitude of the second point (in decimal degrees).
    * @return Returned the meridian arc length between latitude1 and latitude2
    */
    double meridianArcLength(double latitude1, double latitude2);

    /**
    * Calculates the meridian arc length between two points in the same meridian
    * in the referenced ellipsoid.
    *
    * @param  P1 The latitude of the first  point (in radians).
    * @param  P2 The latitude of the second point (in radians).
    * @return Returned the meridian arc length between P1 and P2
    */
    double meridianArcLengthRadians(double P1, double P2);

    /**
    * Computes the azimuth and orthodromic distance from the
    * startingGeographicPoint starting point and the
    * destinationGeographicPoint destination point.
    */
    bool computeDirection();

protected:

    double castToAngleRange(const double alpha);

    /**
    * Checks the latitude validity. The argument @c latitude should be
    * greater or equal than -90 degrees and lower or equals than +90 degrees. As
    * a convenience, this method converts the latitude to radians.
    *
    * @param  latitude The latitude value in decimal degrees.
    */
    bool checkLatitude(double* latitude);

    /**
    * Checks the longitude validity. The argument @c longitude should be
    * greater or equal than -180 degrees and lower or equals than +180 degrees. As
    * a convenience, this method converts the longitude to radians.
    *
    * @param  longitude The longitude value in decimal degrees.
    */
    bool checkLongitude(double* longitude);

    /**
    * Checks the azimuth validity. The argument @c azimuth should be
    * greater or equal than -180 degrees and lower or equals than +180 degrees.
    * As a convenience, this method converts the azimuth to radians.
    *
    * @param  azimuth The azimuth value in decimal degrees.
    */
    bool checkAzimuth(double* azimuth);

    /**
    * Checks the orthodromic distance validity. Arguments @c orthodromicDistance
    * should be greater or equal than 0 and lower or equals than the maximum orthodromic distance.
    *
    * @param  distance The orthodromic distance value.
    */
    bool checkOrthodromicDistance(const double distance);

    /**
     * Tolerance factors from the strictest (<code>TOLERANCE_0</CODE>)
     * to the most relax one (<code>TOLERANCE_3</CODE>).
     */
    double TOLERANCE_0, TOLERANCE_1, TOLERANCE_2, TOLERANCE_3;

    /**
     * Tolerance factor for assertions. It has no impact on computed values.
     */
    double TOLERANCE_CHECK;

    /**
     * The encapsulated ellipsoid.
     */
    Ellipsoid m_ellipsoid;

    /*
    * The semi major axis of the referenced ellipsoid.
    */
    double m_semiMajorAxis;

    /*
    * The semi minor axis of the referenced ellipsoid.
    */
    double m_semiMinorAxis;

    /*
    * The eccentricity squared of the referenced ellipsoid.
    */
    double m_eccentricitySquared;

    /*
    * The maximum orthodromic distance that could be calculated onto the referenced ellipsoid.
    */
    double m_maxOrthodromicDistance;

    /**
     * GPNARC parameters computed from the ellipsoid.
     */
    double A, B, C, D, E, F;

    /**
     * GPNHRI parameters computed from the ellipsoid.
     *
     * @c f if the flattening of the referenced ellipsoid. @c f2,
     * @c f3 and @c f4 are <var>f<sup>2</sup></var>,
     * <var>f<sup>3</sup></var> and <var>f<sup>4</sup></var> respectively.
     */
    double fo, f, f2, f3, f4;

    /**
     * Parameters computed from the ellipsoid.
     */
    double T1, T2, T4, T6;

    /**
     * Parameters computed from the ellipsoid.
     */
    double a01, a02, a03, a21, a22, a23, a42, a43, a63;

    /**
     * The (<var>latitude</var>, <var>longitude</var>) coordinate of the first point
     * in radians. This point is set by setStartingGeographicPoint.
     */
    double m_lat1, m_long1;

    /**
     * The (<var>latitude</var>, <var>longitude</var>) coordinate of the destination point
     * in radians. This point is set by setDestinationGeographicPoint.
     */
    double m_lat2, m_long2;

    /**
     * The distance and azimuth (in radians) from the starting point
     * (long1, lat1) to the destination point
     * (long2, lat2).
     */
    double m_distance, m_azimuth;

    /**
     * Tell if the destination point is valid.
     * @c false if long2 and lat2 need to be computed.
     */
    bool m_destinationValid;

    /**
     * Tell if the azimuth and the distance are valids.
     * @c false if distance and azimuth need to be computed.
     */
    bool m_directionValid;
};

} // namespace Digikam

#endif // GEODETIC_TOOLS_H
