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

#ifndef FILMFILTER_H_
#define FILMFILTER_H_

// Qt includes

#include <QString>
#include <QList>
#include <QListWidgetItem>
#include <QSharedPointer>

// Local includes

#include "dimgthreadedfilter.h"
#include "levelsfilter.h"
#include "cbfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT FilmContainer
{
public:

    enum CNFilmProfile
    {
        CNNeutral = 0,
        CNKodakGold100,
        CNKodakGold200,
        CNKodakEktar100,
        CNKodakProfessionalPortra160NC,
        CNKodakProfessionalPortra160VC,
        CNKodakProfessionalPortra400NC,
        CNKodakProfessionalPortra400VC,
        CNKodakProfessionalPortra800Box,
        CNKodakProfessionalPortra800P1,
        CNKodakProfessionalPortra800P2,
        CNKodakProfessionalNewPortra160,
        CNKodakProfessionalNewPortra400,
        CNKodakFarbwelt100,
        CNKodakFarbwelt200,
        CNKodakFarbwelt400,
        CNKodakRoyalGold400,
        CNAgfaphotoVistaPlus200,
        CNAgfaphotoVistaPlus400,
        CNFujicolorPro160S,
        CNFujicolorPro160C,
        CNFujicolorNPL160,
        CNFujicolorPro400H,
        CNFujicolorPro800Z,
        CNFujicolorSuperiaReala,
        CNFujicolorSuperia100,
        CNFujicolorSuperia200,
        CNFujicolorSuperiaXtra400,
        CNFujicolorSuperiaXtra800,
        CNFujicolorTrueDefinition400,
        CNFujicolorSuperia1600
    };

    class ListItem : public QListWidgetItem
    {
    public:

        ListItem(const QString& text, QListWidget* const parent, CNFilmProfile type)
            : QListWidgetItem(text, parent, type + QListWidgetItem::UserType)
        {
        }
    };

    explicit FilmContainer();
    explicit FilmContainer(CNFilmProfile profile, double gamma, bool sixteenBit);

    void   setWhitePoint(const DColor& wp);
    DColor whitePoint() const;

    void   setExposure(double strength);
    double exposure() const;

    void   setSixteenBit(bool val);
    void   setGamma(double val);
    double gamma() const;

    void          setCNType(CNFilmProfile profile);
    CNFilmProfile cnType() const;

    void setApplyBalance(bool val);
    bool applyBalance() const;

    LevelsContainer toLevels() const;
    CBContainer     toCB() const;

    static const QMap<int, QString> profileMap;
    static QList<ListItem*> profileItemList(QListWidget* const view);

private:

    int    whitePointForChannel(int channel) const;
    double blackPointForChannel(int ch) const;
    double gammaForChannel(int ch) const;

    static QMap<int, QString> profileMapInitializer();

private:

    class Private;
    QSharedPointer<Private> d;
};

// ---------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT FilmFilter: public DImgThreadedFilter
{
public:

    explicit FilmFilter(QObject* const parent=0);
    explicit FilmFilter(DImg* const orgImage, QObject* const parent=0, const FilmContainer& settings=FilmContainer());
    virtual ~FilmFilter();

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:FilmFilter");
    }

    static QString DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Color Negative Inverter"));
    }

    static QList<int> SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int CurrentVersion()
    {
        return 1;
    }

    virtual QString filterIdentifier() const
    {
        return FilterIdentifier();
    }

    virtual FilterAction filterAction();
    virtual void readParameters(const FilterAction& action);

private:

    void filterImage();

private:

    class Private;
    Private* d;
};

} // namespace Digikam

#endif /* FILMFILTER_H_ */
