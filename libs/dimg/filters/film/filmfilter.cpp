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

// C++ includes

#include <cmath>

// Qt includes

#include <QListWidget>

// Local includes

#include "filmfilter_p.h"
#include "invertfilter.h"

namespace Digikam
{

FilmContainer::FilmContainer() :
    d(QSharedPointer<Private>(new Private))
{
}

FilmContainer::FilmContainer(CNFilmProfile profile, double gamma, bool sixteenBit)
    : d(QSharedPointer<Private>(new Private))
{
    d->gamma      = gamma;
    d->sixteenBit = sixteenBit;
    d->whitePoint = DColor(QColor("white"), sixteenBit);
    setCNType(profile);
}

void FilmContainer::setWhitePoint(const DColor& wp)
{
    d->whitePoint = wp;
}

DColor FilmContainer::whitePoint() const
{
    return d->whitePoint;
}

void FilmContainer::setExposure(double strength)
{
    d->exposure = strength;
}

double FilmContainer::exposure() const
{
    return d->exposure;
}

void FilmContainer::setSixteenBit(bool val)
{
    d->sixteenBit = val;
}

void FilmContainer::setGamma(double val)
{
    d->gamma = val;
}
double FilmContainer::gamma() const
{
    return d->gamma;
}

void FilmContainer::setCNType(CNFilmProfile profile)
{
    d->cnType = profile;

    switch (profile)
    {
        default:
            d->profile = FilmProfile(1.0, 1.0, 1.0);
            d->cnType = CNNeutral;
            break;
        case CNKodakGold100:
            d->profile = FilmProfile(1.53, 2.00, 2.40); // check
            break;
        case CNKodakGold200:
            d->profile = FilmProfile(1.53, 2.00, 2.40); // check
            break;
        case CNKodakEktar100:
            d->profile = FilmProfile(1.40, 1.85, 2.34);
            break;
        case CNKodakProfessionalPortra160NC:
            d->profile = FilmProfile(1.49, 1.96, 2.46); // check
            break;
        case CNKodakProfessionalPortra160VC:
            d->profile = FilmProfile(1.56, 2.03, 2.55); // check
            break;
        case CNKodakProfessionalPortra400NC:
            d->profile = FilmProfile(1.69, 2.15, 2.69); // check
            break;
        case CNKodakProfessionalPortra400VC:
            d->profile = FilmProfile(1.78, 2.21, 2.77); // check
            break;
        case CNKodakProfessionalPortra800Box:
            d->profile = FilmProfile(1.89, 2.29, 2.89); // check
            break;
        case CNKodakProfessionalPortra800P1:
            d->profile = FilmProfile(1.53, 2.01, 2.46); // check
            break;
        case CNKodakProfessionalPortra800P2:
            d->profile = FilmProfile(1.74, 2.22, 2.64); // check
            break;
        case CNKodakProfessionalNewPortra160:
            d->profile = FilmProfile(1.41, 1.88, 2.32);
            break;
        case CNKodakProfessionalNewPortra400:
            d->profile = FilmProfile(1.69, 2.15, 2.68); // check
            break;
        case CNKodakFarbwelt100:
            d->profile = FilmProfile(1.86, 2.33, 2.77); // fix, check
            break;
        case CNKodakFarbwelt200:
            d->profile = FilmProfile(1.55, 2.03, 2.42); // check
            break;
        case CNKodakFarbwelt400:
            d->profile = FilmProfile(1.93, 2.43, 2.95); // fix, check
            break;
        case CNKodakRoyalGold400:
            d->profile = FilmProfile(2.24, 2.76, 3.27); // fix, check
            break;
        case CNAgfaphotoVistaPlus200:
            d->profile = FilmProfile(1.70, 2.13, 2.50);
            break;
        case CNAgfaphotoVistaPlus400:
            d->profile = FilmProfile(1.86, 2.35, 2.67); // fix, check
            break;
        case CNFujicolorPro160S:
            d->profile = FilmProfile(1.73, 2.27, 2.53); // fix, check
            break;
        case CNFujicolorPro160C:
            d->profile = FilmProfile(1.96, 2.46, 2.69); // fix, check
            break;
        case CNFujicolorNPL160:
            d->profile = FilmProfile(2.13, 2.36, 2.92); // fix, check
            break;
        case CNFujicolorPro400H:
            d->profile = FilmProfile(1.95, 2.37, 2.62); // fix, check
            break;
        case CNFujicolorPro800Z:
            d->profile = FilmProfile(2.12, 2.37, 2.56); // fix, check
            break;
        case CNFujicolorSuperiaReala:
            d->profile = FilmProfile(1.79, 2.14, 2.49); // check
            break;
        case CNFujicolorSuperia100:
            d->profile = FilmProfile(2.02, 2.46, 2.81); // fix, check
            break;
        case CNFujicolorSuperia200:
            d->profile = FilmProfile(2.11, 2.50, 2.79); // check
            break;
        case CNFujicolorSuperiaXtra400:
            d->profile = FilmProfile(2.11, 2.58, 2.96); // check
            break;
        case CNFujicolorSuperiaXtra800:
            d->profile = FilmProfile(2.44, 2.83, 3.18); // fix, check
            break;
        case CNFujicolorTrueDefinition400:
            d->profile = FilmProfile(1.93, 2.21, 2.39); // fix, check
            break;
        case CNFujicolorSuperia1600:
            d->profile = FilmProfile(2.35, 2.68, 2.96); // fix, check
            break;
    }
}

FilmContainer::CNFilmProfile FilmContainer::cnType() const
{
    return d->cnType;
}

void FilmContainer::setApplyBalance(bool val)
{
    d->applyBalance = val;
}

bool FilmContainer::applyBalance() const
{
    return d->applyBalance;
}

int FilmContainer::whitePointForChannel(int ch) const
{
    int max = d->sixteenBit ? 65535 : 255;

    switch (ch) {
    case RedChannel:    return d->whitePoint.red();
    case GreenChannel:  return d->whitePoint.green();
    case BlueChannel:   return d->whitePoint.blue();
    default:            return max;
    }

    /* not reached */
    return max;
}

double FilmContainer::blackPointForChannel(int ch) const
{
    if (ch == LuminosityChannel || ch == AlphaChannel)
        return 0.0;

    return pow(10, -d->profile.dmax(ch));
}

double FilmContainer::gammaForChannel(int ch) const
{
    int max = d->sixteenBit ? 65535 : 255;

    if (ch == GreenChannel || ch == BlueChannel)
    {
        double bpc = blackPointForChannel(ch)*d->exposure;
        double wpc = (double)whitePointForChannel(ch)/(double)max;
        double bpr = blackPointForChannel(RedChannel)*d->exposure;
        double wpr = (double)whitePointForChannel(RedChannel)/(double)max;

        return log10( bpc / wpc ) / log10( bpr / wpr );
    }

    return 1.0;
}

LevelsContainer FilmContainer::toLevels() const
{
    LevelsContainer l;
    int max = d->sixteenBit ? 65535 : 255;

    for (int i = LuminosityChannel; i <= AlphaChannel; i++)
    {
        l.lInput[i]  = blackPointForChannel(i) * max * d->exposure;
        l.hInput[i]  = whitePointForChannel(i) * d->profile.wp(i);
        l.lOutput[i] = 0;
        l.hOutput[i] = max;
        if (d->applyBalance)
            l.gamma[i]   = gammaForChannel(i);
        else
            l.gamma[i] = 1.0;
    }

    return l;
}

CBContainer FilmContainer::toCB() const
{
    CBContainer cb;

    cb.red        = d->profile.balance(RedChannel);
    cb.green      = d->profile.balance(GreenChannel);
    cb.blue       = d->profile.balance(BlueChannel);
    cb.gamma      = 1.0;

    return cb;
}

QList<FilmContainer::ListItem*> FilmContainer::profileItemList(QListWidget* view)
{
    QList<FilmContainer::ListItem*> itemList;
    QMap<int, QString>::ConstIterator it;

    for (it = profileMap.constBegin(); it != profileMap.constEnd(); it++)
        itemList << new ListItem(it.value(), view, (CNFilmProfile)it.key());

    return itemList;
}

QMap<int, QString> FilmContainer::profileMapInitializer()
{
    QMap<int, QString> profileMap;

    profileMap[CNNeutral]                       = QLatin1String("Neutral");
    profileMap[CNKodakGold100]                  = QLatin1String("Kodak Gold 100");
    profileMap[CNKodakGold200]                  = QLatin1String("Kodak Gold 200");
    profileMap[CNKodakProfessionalNewPortra160] = QLatin1String("Kodak Professional New Portra 160");
    profileMap[CNKodakProfessionalNewPortra400] = QLatin1String("Kodak Professional New Portra 400");
    profileMap[CNKodakEktar100]                 = QLatin1String("Kodak Ektar 100");
    profileMap[CNKodakFarbwelt100]              = QLatin1String("Kodak Farbwelt 100");
    profileMap[CNKodakFarbwelt200]              = QLatin1String("Kodak Farbwelt 200");
    profileMap[CNKodakFarbwelt400]              = QLatin1String("Kodak Farbwelt 400");
    profileMap[CNKodakProfessionalPortra160NC]  = QLatin1String("Kodak Professional Portra 160NC");
    profileMap[CNKodakProfessionalPortra160VC]  = QLatin1String("Kodak Professional Portra 160VC");
    profileMap[CNKodakProfessionalPortra400NC]  = QLatin1String("Kodak Professional Portra 400NC");
    profileMap[CNKodakProfessionalPortra400VC]  = QLatin1String("Kodak Professional Portra 400VC");
    profileMap[CNKodakProfessionalPortra800Box] = QLatin1String("Kodak Professional Portra 800 (Box Speed");
    profileMap[CNKodakProfessionalPortra800P1]  = QLatin1String("Kodak Professional Portra 800 (Push 1 stop");
    profileMap[CNKodakProfessionalPortra800P2]  = QLatin1String("Kodak Professional Portra 800 (Push 2 stop");
    profileMap[CNKodakRoyalGold400]             = QLatin1String("Kodak Royal Gold 400");
    profileMap[CNAgfaphotoVistaPlus200]         = QLatin1String("Agfaphoto Vista Plus 200");
    profileMap[CNAgfaphotoVistaPlus400]         = QLatin1String("Agfaphoto Vista Plus 400");
    profileMap[CNFujicolorPro160S]              = QLatin1String("Fujicolor Pro 160S");
    profileMap[CNFujicolorPro160C]              = QLatin1String("Fujicolor Pro 160C");
    profileMap[CNFujicolorNPL160]               = QLatin1String("Fujicolor NPL 160");
    profileMap[CNFujicolorPro400H]              = QLatin1String("Fujicolor Pro 400H");
    profileMap[CNFujicolorPro800Z]              = QLatin1String("Fujicolor Pro 800Z");
    profileMap[CNFujicolorSuperiaReala]         = QLatin1String("Fujicolor Superia Reala");
    profileMap[CNFujicolorSuperia100]           = QLatin1String("Fujicolor Superia 100");
    profileMap[CNFujicolorSuperia200]           = QLatin1String("Fujicolor Superia 200");
    profileMap[CNFujicolorSuperiaXtra400]       = QLatin1String("Fujicolor Superia X-Tra 400");
    profileMap[CNFujicolorSuperiaXtra800]       = QLatin1String("Fujicolor Superia X-Tra 800");
    profileMap[CNFujicolorTrueDefinition400]    = QLatin1String("Fujicolor Superia True Definition 400");
    profileMap[CNFujicolorSuperia1600]          = QLatin1String("Fujicolor Superia 1600");

    return profileMap;
}

const QMap<int, QString> FilmContainer::profileMap = FilmContainer::profileMapInitializer();

// ------------------------------------------------------------------

FilmFilter::FilmFilter(QObject* const parent)
    : DImgThreadedFilter(parent, QLatin1String("FilmFilter")),
      d(new Private())
{
    d->film = FilmContainer();
    initFilter();
}

FilmFilter::FilmFilter(DImg* const orgImage, QObject* const parent, const FilmContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("FilmFilter")),
      d(new Private())
{
    d->film = settings;
    initFilter();
}

FilmFilter::~FilmFilter()
{
    cancelFilter();
    delete d;
}

void FilmFilter::filterImage()
{
    DImg tmpLevel;
    DImg tmpGamma;
    DImg tmpInv;

    LevelsContainer l = d->film.toLevels();
    CBContainer cb  = d->film.toCB();
    CBContainer gamma;

    // level the image first, this removes the orange mask and corrects
    // colors according to the density ranges of the film profile
    LevelsFilter(l, this, m_orgImage, tmpLevel, 0, 40);

    // in case of a linear raw scan, gamma needs to be
    // applied after leveling the image, otherwise the image will
    // look too bright. The standard value is 2.2, but 1.8 is also
    // frequently found in literature
    gamma.gamma = d->film.gamma();
    CBFilter(gamma, this, tmpLevel, tmpGamma, 40, 80);

    // invert the image to have a positive image
    InvertFilter(this, tmpGamma, tmpInv, 80, 100);

    m_destImage = tmpInv;
    postProgress(100);
}

FilterAction FilmFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("CNType"),               d->film.cnType());
    action.addParameter(QLatin1String("ProfileName"),          FilmContainer::profileMap[d->film.cnType()]);
    action.addParameter(QLatin1String("Exposure"),             d->film.exposure());
    action.addParameter(QLatin1String("Gamma"),                d->film.gamma());
    action.addParameter(QLatin1String("ApplyColorBalance"),    d->film.applyBalance());
    action.addParameter(QLatin1String("WhitePointRed"),        d->film.whitePoint().red());
    action.addParameter(QLatin1String("WhitePointGreen"),      d->film.whitePoint().green());
    action.addParameter(QLatin1String("WhitePointBlue"),       d->film.whitePoint().blue());
    action.addParameter(QLatin1String("WhitePointAlpha"),      d->film.whitePoint().alpha());
    action.addParameter(QLatin1String("WhitePointSixteenBit"), d->film.whitePoint().sixteenBit());

    return action;
}

void FilmFilter::readParameters(const FilterAction& action)
{
    double red   = action.parameter(QLatin1String("WhitePointRed")).toDouble();
    double green = action.parameter(QLatin1String("WhitePointGreen")).toDouble();
    double blue  = action.parameter(QLatin1String("WhitePointBlue")).toDouble();
    double alpha = action.parameter(QLatin1String("WhitePointAlpha")).toDouble();
    bool sb      = action.parameter(QLatin1String("WhitePointSixteenBit")).toBool();
    bool balance = action.parameter(QLatin1String("ApplyColorBalance")).toBool();

    d->film.setWhitePoint(DColor(red, green, blue, alpha, sb));
    d->film.setExposure(action.parameter(QLatin1String("Exposure")).toDouble());
    d->film.setGamma(action.parameter(QLatin1String("Gamma")).toDouble());
    d->film.setCNType((FilmContainer::CNFilmProfile)(action.parameter(QLatin1String("CNType")).toInt()));
    d->film.setApplyBalance(balance);
}

} // namespace Digikam
