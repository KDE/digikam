/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-05
 * Description : Geodetic tools
 *
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2004-2006 by Daniele Franzoni
 * Copyright (C) 2004-2006 by Martin Desruisseaux
 * Copyright (C) 2003-2006 GeoTools Project Managment Committee (PMC), http://geotools.org
 * Copyright (C) 2001      Institut de Recherche pour le Developpement
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

#include "geodetictools.h"

// C++ includes

#include <cstdlib>
#include <cfloat>

namespace Digikam
{

using namespace Coordinates;

GeodeticCalculator::GeodeticCalculator(const Ellipsoid& e)
    : m_ellipsoid(e),
      m_lat1(0), m_long1(0), m_lat2(0), m_long2(0),
      m_distance(0), m_azimuth(0),
      m_destinationValid(false), m_directionValid(false)
{
    m_semiMajorAxis = m_ellipsoid.semiMajorAxis();
    m_semiMinorAxis = m_ellipsoid.semiMinorAxis();

    // constants
    TOLERANCE_0     = 5.0e-15,
    TOLERANCE_1     = 5.0e-14,
    TOLERANCE_2     = 5.0e-13,
    TOLERANCE_3     = 7.0e-3;
    TOLERANCE_CHECK = 1E-8;

    /* calculation of GPNHRI parameters */
    f                     = (m_semiMajorAxis-m_semiMinorAxis) / m_semiMajorAxis;
    fo                    = 1.0 - f;
    f2                    = f*f;
    f3                    = f*f2;
    f4                    = f*f3;
    m_eccentricitySquared = f * (2.0-f);

    /* Calculation of GNPARC parameters */
    const double E2 = m_eccentricitySquared;
    const double E4 = E2*E2;
    const double E6 = E4*E2;
    const double E8 = E6*E2;
    const double EX = E8*E2;

    A =  1.0+0.75*E2+0.703125*E4+0.68359375 *E6+0.67291259765625*E8+0.6661834716796875 *EX;
    B =      0.75*E2+0.9375  *E4+1.025390625*E6+1.07666015625   *E8+1.1103057861328125 *EX;
    C =              0.234375*E4+0.41015625 *E6+0.538330078125  *E8+0.63446044921875   *EX;
    D =                          0.068359375*E6+0.15380859375   *E8+0.23792266845703125*EX;
    E =                                         0.01922607421875*E8+0.0528717041015625 *EX;
    F =                                                             0.00528717041015625*EX;

    m_maxOrthodromicDistance = m_semiMajorAxis * (1.0-E2) * M_PI * A - 1.0;

    T1 = 1.0;
    T2 = -0.25*f*(1.0 + f + f2);
    T4 = 0.1875 * f2 * (1.0+2.25*f);
    T6 = 0.1953125 * f3;

    const double a = f3*(1.0+2.25*f);
    a01            = -f2*(1.0+f+f2)/4.0;
    a02            = 0.1875*a;
    a03            = -0.1953125*f4;
    a21            = -a01;
    a22            = -0.25*a;
    a23            = 0.29296875*f4;
    a42            = 0.03125*a;
    a43            = 0.05859375*f4;
    a63            = 5.0*f4/768.0;
}

double GeodeticCalculator::castToAngleRange(const double alpha)
{
    return alpha - (2*M_PI) * floor(alpha/(2*M_PI) + 0.5);
}

bool GeodeticCalculator::checkLatitude(double* latitude)
{
    if (*latitude >= -90.0 && *latitude <= 90.0)
    {
        *latitude = toRadians(*latitude);
        return true;
    }

    return false;
}

bool GeodeticCalculator::checkLongitude(double* longitude)
{
    if (*longitude >= -180.0 && *longitude <= 180.0)
    {
        *longitude = toRadians(*longitude);
        return true;
    }

    return false;
}

bool GeodeticCalculator::checkAzimuth(double* azimuth)
{
    if (*azimuth >= -180.0 && *azimuth <= 180.0)
    {
        *azimuth = toRadians(*azimuth);
        return true;
    }

    return false;
}

bool GeodeticCalculator::checkOrthodromicDistance(const double distance)
{
    return distance >= 0.0 && distance <= m_maxOrthodromicDistance;
}

Ellipsoid GeodeticCalculator::ellipsoid() const
{
    return m_ellipsoid;
}

void GeodeticCalculator::setStartingGeographicPoint(double longitude, double latitude)
{
    if (!checkLongitude(&longitude) || !checkLatitude(&latitude))
    {
        return;
    }

    // Check passed. Now performs the changes in this object.
    m_long1            = longitude;
    m_lat1             = latitude;
    m_destinationValid = false;
    m_directionValid   = false;
}

void GeodeticCalculator::setDestinationGeographicPoint(double longitude, double latitude)
{
    if (!checkLongitude(&longitude) || !checkLatitude(&latitude))
    {
        return;
    }

    // Check passed. Now performs the changes in this object.
    m_long2            = longitude;
    m_lat2             = latitude;
    m_destinationValid = true;
    m_directionValid   = false;
}

bool GeodeticCalculator::destinationGeographicPoint(double* longitude, double* latitude)
{
    if (!m_destinationValid)
    {
        if (!computeDestinationPoint())
        {
            return false;
        }
    }

    *longitude = toDegrees(m_long2);
    *latitude  = toDegrees(m_lat2);
    return true;
}

QPointF GeodeticCalculator::destinationGeographicPoint()
{
    double x, y;
    destinationGeographicPoint(&x, &y);
    QPointF point;
    point.setX(x);
    point.setY(y);
    return point;
}

void GeodeticCalculator::setDirection(double azimuth, double distance)
{
    // Check first in case an exception is raised
    // (in other words, we change all or nothing).
    if (!checkAzimuth(&azimuth))
    {
        return;
    }

    if (!checkOrthodromicDistance(distance))
    {
        return;
    }

    // Check passed. Now performs the changes in this object.
    m_azimuth          = azimuth;
    m_distance         = distance;
    m_destinationValid = false;
    m_directionValid   = true;
}

double GeodeticCalculator::azimuth()
{
    if (!m_directionValid)
    {
        computeDirection();
    }

    return toDegrees(m_azimuth);
}

double GeodeticCalculator::orthodromicDistance()
{
    if (!m_directionValid)
    {
        computeDirection();
        checkOrthodromicDistance();
    }

    return m_distance;
}

bool GeodeticCalculator::checkOrthodromicDistance()
{
    double check;
    check = m_ellipsoid.orthodromicDistance(toDegrees(m_long1), toDegrees(m_lat1),
                                            toDegrees(m_long2), toDegrees(m_lat2));
    check = fabs(m_distance - check);
    return (check <= (m_distance+1) * TOLERANCE_CHECK);
}

bool GeodeticCalculator::computeDestinationPoint()
{
    if (!m_directionValid)
    {
        return false;
    }

    // Protect internal variables from changes
    const double lat1     = m_lat1;
    const double long1    = m_long1;
    const double azimuth  = m_azimuth;
    const double distance = m_distance;
    /*
    * Solution of the geodetic direct problem after T.Vincenty.
    * Modified Rainsford's method with Helmert's elliptical terms.
    * Effective in any azimuth and at any distance short of antipodal.
    *
    * Latitudes and longitudes in radians positive North and East.
    * Forward azimuths at both points returned in radians from North.
    *
    * Programmed for CDC-6600 by LCDR L.Pfeifer NGS ROCKVILLE MD 18FEB75
    * Modified for IBM SYSTEM 360 by John G.Gergen NGS ROCKVILLE MD 7507
    * Ported from Fortran to Java by Daniele Franzoni.
    *
    * Source: ftp://ftp.ngs.noaa.gov/pub/pcsoft/for_inv.3d/source/forward.for
    *         subroutine DIRECT1
    */
    double TU  = fo*sin(lat1) / cos(lat1);
    double SF  = sin(azimuth);
    double CF  = cos(azimuth);
    double BAZ = (CF!=0) ? atan2(TU,CF)*2.0 : 0;
    double CU  = 1/sqrt(TU*TU + 1.0);
    double SU  = TU*CU;
    double SA  = CU*SF;
    double C2A = 1.0 - SA*SA;
    double X   = sqrt((1.0/fo/fo-1)*C2A+1.0) + 1.0;
    X          = (X-2.0)/X;
    double C   = 1.0-X;
    C          = (X*X/4.0+1.0)/C;
    double D   = (0.375*X*X-1.0)*X;
    TU         = distance / fo / m_semiMajorAxis / C;
    double Y   = TU;
    double SY, CY, CZ, E;

    do
    {
        SY = sin(Y);
        CY = cos(Y);
        CZ = cos(BAZ+Y);
        E  = CZ*CZ*2.0-1.0;
        C  = Y;
        X  = E*CY;
        Y  = E+E-1.0;
        Y  = (((SY*SY*4.0-3.0)*Y*CZ*D/6.0+X)*D/4.0-CZ)*SY*D+TU;
    }
    while (fabs(Y-C) > TOLERANCE_1);

    BAZ                = CU*CY*CF - SU*SY;
    C                  = fo*sqrt(SA*SA+BAZ*BAZ);
    D                  = SU*CY + CU*SY*CF;
    m_lat2             = atan2(D,C);
    C                  = CU*CY-SU*SY*CF;
    X                  = atan2(SY*SF,C);
    C                  = ((-3.0*C2A+4.0)*f+4.0)*C2A*f/16.0;
    D                  = ((E*CY*C+CZ)*SY*C+Y)*SA;
    m_long2            = long1+X - (1.0-C)*D*f;
    m_long2            = castToAngleRange(m_long2);
    m_destinationValid = true;
    return true;
}

double GeodeticCalculator::meridianArcLength(double latitude1, double latitude2)
{
    if (!checkLatitude(&latitude1) || !checkLatitude(&latitude2))
    {
        return 0.0;
    }

    return meridianArcLengthRadians(latitude1, latitude2);
}

double GeodeticCalculator::meridianArcLengthRadians(double P1, double P2)
{
    /*
    * Latitudes P1 and P2 in radians positive North and East.
    * Forward azimuths at both points returned in radians from North.
    *
    * Source: ftp://ftp.ngs.noaa.gov/pub/pcsoft/for_inv.3d/source/inverse.for
    *         subroutine GPNARC
    *         version    200005.26
    *         written by Robert (Sid) Safford
    *
    * Ported from Fortran to Java by Daniele Franzoni.
    */
    double S1 = fabs(P1);
    double S2 = fabs(P2);
    double DA = (P2-P1);

    // Check for a 90 degree lookup
    if (S1>TOLERANCE_0 || S2<=(M_PI/2-TOLERANCE_0) || S2>=(M_PI/2+TOLERANCE_0))
    {
        const double DB = sin(P2* 2.0) - sin(P1* 2.0);
        const double DC = sin(P2* 4.0) - sin(P1* 4.0);
        const double DD = sin(P2* 6.0) - sin(P1* 6.0);
        const double DE = sin(P2* 8.0) - sin(P1* 8.0);
        const double DF = sin(P2*10.0) - sin(P1*10.0);
        // Compute the S2 part of the series expansion
        S2              = -DB*B/2.0 + DC*C/4.0 - DD*D/6.0 + DE*E/8.0 - DF*F/10.0;
    }

    // Compute the S1 part of the series expansion
    S1 = DA*A;
    // Compute the arc length
    return fabs(m_semiMajorAxis * (1.0-m_eccentricitySquared) * (S1+S2));
}

/**
* Computes the azimuth and orthodromic distance from the
* startingGeographicPoint() and the
* destinationGeographicPoint().
*/
bool GeodeticCalculator::computeDirection()
{
    if (!m_destinationValid)
    {
        return false;
    }

    // Protect internal variables from change.
    const double long1 = m_long1;
    const double lat1  = m_lat1;
    const double long2 = m_long2;
    const double lat2  = m_lat2;
    /*
    * Solution of the geodetic inverse problem after T.Vincenty.
    * Modified Rainsford's method with Helmert's elliptical terms.
    * Effective in any azimuth and at any distance short of antipodal.
    *
    * Latitudes and longitudes in radians positive North and East.
    * Forward azimuths at both points returned in radians from North.
    *
    * Programmed for CDC-6600 by LCDR L.Pfeifer NGS ROCKVILLE MD 18FEB75
    * Modified for IBM SYSTEM 360 by John G.Gergen NGS ROCKVILLE MD 7507
    * Ported from Fortran to Java by Daniele Franzoni.
    *
    * Source: ftp://ftp.ngs.noaa.gov/pub/pcsoft/for_inv.3d/source/inverse.for
    *         subroutine GPNHRI
    *         version    200208.09
    *         written by robert (sid) safford
    */
    const double dlon = castToAngleRange(long2-long1);
    const double ss   = fabs(dlon);

    if (ss < TOLERANCE_1)
    {
        m_distance       = meridianArcLengthRadians(lat1, lat2);
        m_azimuth        = (lat2>lat1) ? 0.0 : M_PI;
        m_directionValid = true;
        return true;
    }

    /*
    * Computes the limit in longitude (alimit), it is equal
    * to twice  the distance from the equator to the pole,
    * as measured along the equator
    */
    // tests for antinodal difference
    const double ESQP   = m_eccentricitySquared / (1.0-m_eccentricitySquared);
    const double alimit = M_PI*fo;

    if (ss>=alimit &&
        lat1<TOLERANCE_3 && lat1>-TOLERANCE_3 &&
        lat2<TOLERANCE_3 && lat2>-TOLERANCE_3)
    {
        // Computes an approximate AZ
        const double CONS = (M_PI-ss)/(M_PI*f);
        double AZ         = asin(CONS);
        int iter          = 0;
        double AZ_TEMP, S, AO;

        do
        {
            if (++iter > 8)
            {
                //ERROR
                return false;
            }

            S                 = cos(AZ);
            const double C2   = S*S;
            // Compute new AO
            AO                = T1 + T2*C2 + T4*C2*C2 + T6*C2*C2*C2;
            const double _CS_ = CONS/AO;
            S                 = asin(_CS_);
            AZ_TEMP           = AZ;
            AZ                = S;
        }
        while (fabs(S-AZ_TEMP) >= TOLERANCE_2);

        const double AZ1 = (dlon < 0.0) ? 2.0*M_PI - S : S;
        m_azimuth        = castToAngleRange(AZ1);
        //const double AZ2 = 2.0*M_PI - AZ1;
        S                = cos(AZ1);

        // Equatorial - geodesic(S-s) SMS
        const double U2 = ESQP*S*S;
        const double U4 = U2*U2;
        const double U6 = U4*U2;
        const double U8 = U6*U2;
        const double BO =  1.0                  +
                           0.25             *U2 +
                           0.046875         *U4 +
                           0.01953125       *U6 +
                           -0.01068115234375*U8;
        S                = sin(AZ1);
        const double SMS = m_semiMajorAxis*M_PI*(1.0 - f*fabs(S)*AO - BO*fo);
        m_distance       = m_semiMajorAxis*ss - SMS;
        m_directionValid = true;
        return true;
    }

    // the reduced latitudes
    const double  u1 = atan(fo*sin(lat1)/cos(lat1));
    const double  u2 = atan(fo*sin(lat2)/cos(lat2));
    const double su1 = sin(u1);
    const double cu1 = cos(u1);
    const double su2 = sin(u2);
    const double cu2 = cos(u2);
    double xy, w, q2, q4, q6, r2, r3, sig, ssig, slon, clon, sinalf, ab=dlon;
    int kcount = 0;

    do
    {
        if (++kcount > 8)
        {
            //ERROR
            return false;
        }

        clon              = cos(ab);
        slon              = sin(ab);
        const double csig = su1*su2 + cu1*cu2*clon;
        ssig              = sqrt(slon*cu2*slon*cu2 + (su2*cu1-su1*cu2*clon)*(su2*cu1-su1*cu2*clon));
        sig               = atan2(ssig, csig);
        sinalf            = cu1*cu2*slon/ssig;
        w                 = (1.0 - sinalf*sinalf);
        const double t4   = w*w;
        const double t6   = w*t4;

        // the coefficents of type a
        const double ao = f+a01*w+a02*t4+a03*t6;
        const double a2 =   a21*w+a22*t4+a23*t6;
        const double a4 =         a42*t4+a43*t6;
        const double a6 =                a63*t6;

        // the multiple angle functions
        double qo  = 0.0;

        if (w > TOLERANCE_0)
        {
            qo = -2.0*su1*su2/w;
        }

        q2 = csig + qo;
        q4 = 2.0*q2*q2 - 1.0;
        q6 = q2*(4.0*q2*q2 - 3.0);
        r2 = 2.0*ssig*csig;
        r3 = ssig*(3.0 - 4.0*ssig*ssig);

        // the longitude difference
        const double s = sinalf*(ao*sig + a2*ssig*q2 + a4*r2*q4 + a6*r3*q6);
        double xz      = dlon+s;
        xy             = fabs(xz-ab);
        ab             = dlon+s;
    }
    while (xy >= TOLERANCE_1);

    const double z  = ESQP*w;
    const double bo = 1.0 + z*( 1.0/4.0 + z*(-3.0/  64.0 + z*(  5.0/256.0 - z*(175.0/16384.0))));
    const double b2 =       z*(-1.0/4.0 + z*( 1.0/  16.0 + z*(-15.0/512.0 + z*( 35.0/ 2048.0))));
    const double b4 =                   z*z*(-1.0/ 128.0 + z*(  3.0/512.0 - z*( 35.0/ 8192.0)));
    const double b6 =                                  z*z*z*(-1.0/1536.0 + z*(  5.0/ 6144.0));

    // The distance in ellispoid axis units.
    m_distance = m_semiMinorAxis * (bo*sig + b2*ssig*q2 + b4*r2*q4 + b6*r3*q6);
    double az1 = (dlon < 0.0) ? M_PI*(3.0/2.0) : M_PI/2.0;

    // now compute the az1 & az2 for latitudes not on the equator
    if ((fabs(su1)>=TOLERANCE_0) || (fabs(su2)>=TOLERANCE_0))
    {
        const double tana1 = slon*cu2 / (su2*cu1 - clon*su1*cu2);
        const double sina1 = sinalf/cu1;

        // azimuths from north,longitudes positive east
        az1 = atan2(sina1, sina1/tana1);
    }

    m_azimuth = castToAngleRange(az1);
    m_directionValid = true;
    return true;
}

/*
/ **
* Calculates the geodetic curve between two points in the referenced ellipsoid.
* A curve in the ellipsoid is a path which points contain the longitude and latitude
* of the points in the geodetic curve. The geodetic curve is computed from the
* {@linkplain #getStartingGeographicPoint starting point} to the
* {@linkplain #getDestinationGeographicPoint destination point}.
*
* @param  numberOfPoints The number of vertex in the geodetic curve.
*         NOTE: This argument is only a hint and may be ignored
*         in future version (if we compute a real curve rather than a list of line
*         segments).
* @return The path that represents the geodetic curve from the
*         {@linkplain #getStartingGeographicPoint starting point} to the
*         {@linkplain #getDestinationGeographicPoint destination point}.
*
* @todo We should check for cases where the path cross the 90N, 90S, 90E or 90W boundaries.
* /
public Shape getGeodeticCurve(const int numberOfPoints) {
    if (numberOfPoints < 0)
        return Shape;
    if (!directionValid) {
        computeDirection();
    }
    if (!destinationValid) {
        computeDestinationPoint();
    }
    const double         long2 = this->long2;
    const double          lat2 = this->lat2;
    const double      distance = this->distance;
    const double deltaDistance = distance / (numberOfPoints+1);
    final GeneralPath     path = new GeneralPath(GeneralPath.WIND_EVEN_ODD, numberOfPoints+1);
    path.moveTo((float)toDegrees(long1),
                (float)toDegrees(lat1));
    for (int i=1; i<numberOfPoints; ++i) {
        this->distance = i*deltaDistance;
        computeDestinationPoint();
        path.lineTo((float)toDegrees(this->long2),
                    (float)toDegrees(this->lat2));
    }
    this->long2    = long2;
    this->lat2     = lat2;
    this->distance = distance;
    path.lineTo((float)toDegrees(long2),
                (float)toDegrees(lat2));
    return path;
}

/ **
* Calculates the geodetic curve between two points in the referenced ellipsoid.
* A curve in the ellipsoid is a path which points contain the longitude and latitude
* of the points in the geodetic curve. The geodetic curve is computed from the
* {@linkplain #getStartingGeographicPoint starting point} to the
* {@linkplain #getDestinationGeographicPoint destination point}.
*
* @return The path that represents the geodetic curve from the
*         {@linkplain #getStartingGeographicPoint starting point} to the
*         {@linkplain #getDestinationGeographicPoint destination point}.
* /
public Shape getGeodeticCurve() {
    return getGeodeticCurve(10);
}
*/

// ---------------------------------------------------------------------------------

Ellipsoid Ellipsoid::WGS84()
{
    return createFlattenedSphere(QLatin1String("WGS84"), 6378137.0, 298.257223563);
}

Ellipsoid Ellipsoid::GRS80()
{
    return createFlattenedSphere(QLatin1String("GRS80"), 6378137.0, 298.257222101);
}

Ellipsoid Ellipsoid::INTERNATIONAL_1924()
{
    return createFlattenedSphere(QLatin1String("International 1924"), 6378388.0, 297.0);
}

Ellipsoid Ellipsoid::CLARKE_1866()
{
    return createFlattenedSphere(QLatin1String("Clarke 1866"), 6378206.4, 294.9786982);
}

Ellipsoid Ellipsoid::SPHERE()
{
    return createEllipsoid(QLatin1String("SPHERE"), 6371000, 6371000);
}

Ellipsoid::Ellipsoid(const QString& name, double semiMajorAxis, double  semiMinorAxis,
                     double inverseFlattening, bool ivfDefinitive)
    : name(name), m_semiMajorAxis(semiMajorAxis), m_semiMinorAxis(semiMinorAxis),
      m_inverseFlattening(inverseFlattening), m_ivfDefinitive(ivfDefinitive), m_isSphere(false)
{
}

Ellipsoid::Ellipsoid(const QString& name, double radius, bool ivfDefinitive)
    : name(name), m_semiMajorAxis(radius), m_semiMinorAxis(radius),
      m_inverseFlattening(DBL_MAX), m_ivfDefinitive(ivfDefinitive), m_isSphere(true)
{
}

Ellipsoid Ellipsoid::createEllipsoid(const QString& name,
                                     double m_semiMajorAxis, double m_semiMinorAxis)
{
    if (m_semiMajorAxis == m_semiMinorAxis)
    {
        return Ellipsoid(name, m_semiMajorAxis, false);
    }
    else
    {
        return Ellipsoid(name, m_semiMajorAxis, m_semiMinorAxis,
                         m_semiMajorAxis/(m_semiMajorAxis-m_semiMinorAxis), false);
    }
}

Ellipsoid Ellipsoid::createFlattenedSphere(const QString& name,
                                           double m_semiMajorAxis, double m_inverseFlattening)
{
    if (m_inverseFlattening == DBL_MAX)
    {
        return Ellipsoid(name, m_semiMajorAxis, true);
    }
    else
    {
        return Ellipsoid(name, m_semiMajorAxis,
                         m_semiMajorAxis*(1-1/m_inverseFlattening),
                         m_inverseFlattening, true);
    }
}

double Ellipsoid::semiMajorAxis() const
{
    return m_semiMajorAxis;
}

double Ellipsoid::semiMinorAxis() const
{
    return m_semiMinorAxis;
}

double Ellipsoid::eccentricity() const
{
    if (m_isSphere)
    {
        return 0.0;
    }

    const double f = 1-m_semiMinorAxis/m_semiMajorAxis;

    return sqrt(2*f - f*f);
}

double Ellipsoid::inverseFlattening() const
{
    return m_inverseFlattening;
}

bool Ellipsoid::isIvfDefinitive() const
{
    return m_ivfDefinitive;
}

bool Ellipsoid::isSphere() const
{
    return (m_semiMajorAxis == m_semiMinorAxis);
}

double Ellipsoid::orthodromicDistance(double x1, double y1, double x2, double y2)
{
    x1 = toRadians(x1);
    y1 = toRadians(y1);
    x2 = toRadians(x2);
    y2 = toRadians(y2);
    /*
    * Solution of the geodetic inverse problem after T.Vincenty.
    * Modified Rainsford's method with Helmert's elliptical terms.
    * Effective in any azimuth and at any distance short of antipodal.
    *
    * Latitudes and longitudes in radians positive North and East.
    * Forward azimuths at both points returned in radians from North.
    *
    * Programmed for CDC-6600 by LCDR L.Pfeifer NGS ROCKVILLE MD 18FEB75
    * Modified for IBM SYSTEM 360 by John G.Gergen NGS ROCKVILLE MD 7507
    * Ported from Fortran to Java by Martin Desruisseaux.
    *
    * Source: ftp://ftp.ngs.noaa.gov/pub/pcsoft/for_inv.3d/source/inverse.for
    *         subroutine INVER1
    */
    const int    MAX_ITERATIONS = 100;
    const double EPS            = 0.5E-13;
    const double F              = 1/m_inverseFlattening;
    const double R              = 1-F;

    double tu1 = R * sin(y1) / cos(y1);
    double tu2 = R * sin(y2) / cos(y2);
    double cu1 = 1 / sqrt(tu1*tu1 + 1);
    double cu2 = 1 / sqrt(tu2*tu2 + 1);
    double su1 = cu1*tu1;
    double s   = cu1*cu2;
    double baz = s*tu2;
    double faz = baz*tu1;
    double x   = x2-x1;

    for (int i=0; i<MAX_ITERATIONS; ++i)
    {
        const double sx  = sin(x);
        const double cx  = cos(x);
        tu1              = cu2*sx;
        tu2              = baz - su1*cu2*cx;
        const double sy  = sqrt(tu1*tu1 + tu2*tu2);
        const double cy  = s*cx + faz;
        const double y   = atan2(sy, cy);
        const double SA  = s*sx/sy;
        const double c2a = 1 - SA*SA;
        double cz        = faz+faz;

        if (c2a > 0)
        {
            cz = -cz/c2a + cy;
        }

        double e = cz*cz*2 - 1;
        double c = ((-3*c2a+4)*F+4)*c2a*F/16;
        double d = x;
        x        = ((e*cy*c+cz)*sy*c+y)*SA;
        x        = (1-c)*x*F + x2-x1;

        if (fabs(d-x) <= EPS)
        {
            if (false)
            {
                // 'faz' and 'baz' are forward azimuths at both points.
                // Since the current API can't returns this result, it
                // doesn't worth to compute it at this time.
                faz = atan2(tu1, tu2);
                baz = atan2(cu1*sx, baz*cx - su1*cu2)+M_PI;
            }

            x = sqrt((1/(R*R)-1) * c2a + 1)+1;
            x = (x-2)/x;
            c = 1-x;
            c = (x*x/4 + 1)/c;
            d = (0.375*x*x - 1)*x;
            x = e*cy;
            s = 1-2*e;
            s = ((((sy*sy*4 - 3)*s*cz*d/6-x)*d/4+cz)*sy*d+y)*c*R*m_semiMajorAxis;
            return s;
        }
    }

    // No convergence. It may be because coordinate points
    // are equals or because they are at antipodes.
    const double LEPS = 1E-10;

    if (fabs(x1-x2) <= LEPS && fabs(y1-y2) <= LEPS)
    {
        return 0.0; // Coordinate points are equals
    }

    if (fabs(y1) <= LEPS && fabs(y2) <= LEPS)
    {
        return fabs(x1-x2) * m_semiMajorAxis; // Points are on the equator.
    }

    // Other cases: no solution for this algorithm.
    return 0.0;
}

double Ellipsoid::radiusOfCurvature(double latitude)
{
    // WARNING: Code not from geotools
    double esquare = pow(eccentricity(), 2);
    return m_semiMajorAxis * sqrt(1 - esquare) / (1 - esquare * pow( sin(toRadians(latitude)), 2));
}

} // namespace Digikam
