/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-05
 * Description : film color negative inverter filter
 *
 * Copyright (C) 2012 by Matthias Welwarsky <matthias at welwarsky dot de>
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

#ifndef FILMFILTER_P_H
#define FILMFILTER_P_H

#include "filmfilter.h"

namespace Digikam
{

class DColor;

class FilmProfile
{
public:

    FilmProfile(double rdm=0.0, double gdm=0.0, double bdm=0.0)
        : redDmax(rdm),
          greenDmax(gdm),
          blueDmax(bdm),
          rBalance(1.0),
          gBalance(1.0),
          bBalance(1.0),
          wpRed(1.0),
          wpGreen(1.0),
          wpBlue(1.0)
    {
    }

    double dmax(int channel) const
    {
        switch (channel)
        {
            case RedChannel:    return redDmax;
            case GreenChannel:  return greenDmax;
            case BlueChannel:   return blueDmax;
            default:            return 0.0;
        }
    }

    FilmProfile& setBalance(double rB, double gB, double bB)
    {
        rBalance = rB;
        gBalance = gB;
        bBalance = bB;
        return *this;
    }

    double balance(int channel) const
    {
        switch (channel)
        {
            case RedChannel:    return rBalance;
            case GreenChannel:  return gBalance;
            case BlueChannel:   return bBalance;
            default:            return 1.0;
        }
    }

    FilmProfile& setWp(double rWp, double gWp, double bWp)
    {
        wpRed   = rWp;
        wpGreen = gWp;
        wpBlue  = bWp;
        return *this;
    }

    double wp(int channel) const
    {
        switch (channel)
        {
            case RedChannel:    return wpRed;
            case GreenChannel:  return wpGreen;
            case BlueChannel:   return wpBlue;
            default:            return 1.0;
        }
    }

private:

    double redDmax;
    double greenDmax;
    double blueDmax;

    double rBalance;
    double gBalance;
    double bBalance;

    double wpRed;
    double wpGreen;
    double wpBlue;
};

// --------------------------------------------------------------------------------------------------------

class FilmContainer::Private
{
public:

    Private()
        : gamma(1.0),
          exposure(1.0),
          sixteenBit(false),
          profile(FilmProfile(1.0, 1.0, 1.0)),
          cnType(CNNeutral),
          whitePoint(DColor(QColor("white"), false)),
          applyBalance(true)
    {
    }

    ~Private()
    {
    }

    double        gamma;
    double        exposure;
    bool          sixteenBit;
    FilmProfile   profile;
    CNFilmProfile cnType;
    DColor        whitePoint;
    bool          applyBalance;
};

// ------------------------------------------------------------------

class FilmFilter::Private
{
public:

    Private()
    {
    }

    ~Private()
    {
    }

    FilmContainer film;
};

} // namespace Digikam

#endif // FILMFILTER_P_H
