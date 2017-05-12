/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : Test parsing rg data.
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
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

#include "test_rgparsing.h"

// Qt includes

#include <QDateTime>
#include <QUrl>

// local includes

#include "backend-rg.h"
#include "parsetagstring.h"

using namespace Digikam;

/**
 * @brief Dummy test that does nothing
 */
void TestRGParsing::testNoOp()
{
}

void TestRGParsing::testVerifyRG()
{
    {
        // testing tag parsing
        RGInfo info;
        QString country = QString::fromUtf8("France");
        QString city = QString::fromUtf8("Paris");

        const QString howToFormat = QLatin1String("My Tags/{Country}/{City}");

        info.rgData[QLatin1String("country")] = country;
        info.rgData[QLatin1String("city")]    = city;

        QCOMPARE( makeTagString(info, howToFormat, QLatin1String("OSM")).last(), QLatin1String("My Tags/France/Paris"));
    }

    {
        // german special characters
        RGInfo info;
        QString country = QString::fromUtf8("Germany");
        QString city    = QString::fromUtf8("Düsseldorf");

        const QString howToFormat = QLatin1String("My Tags/{Country}/{City}");

        info.rgData[QLatin1String("country")] = country;
        info.rgData[QLatin1String("city")]    = city;

        QCOMPARE( makeTagString(info, howToFormat, QLatin1String("OSM")).last(), QString::fromUtf8("My Tags/Germany/Düsseldorf") );
    }

    {
        // chinese special characters
        RGInfo info;
        QString country = QString::fromUtf8("中国");
        QString city    = QString::fromUtf8("北京市");

        const QString howToFormat = QLatin1String("My Tags/{Country}/{City}");

        info.rgData[QLatin1String("country")] = country;
        info.rgData[QLatin1String("city")]    = city;

        QCOMPARE( makeTagString(info, howToFormat, QLatin1String("OSM")).last(), QString::fromUtf8("My Tags/中国/北京市") );
    }

    {
        // romanian special characters
        RGInfo info;
        QString country = QString::fromUtf8("România");
        QString city    = QString::fromUtf8("București");

        const QString howToFormat = QLatin1String("My Tags/{Country}/{City}");

        info.rgData[QLatin1String("country")] = country;
        info.rgData[QLatin1String("city")]    = city;

        QCOMPARE( makeTagString(info, howToFormat, QLatin1String("OSM")).last(), QString::fromUtf8("My Tags/România/București"));
    }

    {
        // No {Country}
        RGInfo info;
        QString country = QString::fromUtf8("România");
        QString city    = QString::fromUtf8("București");

        const QString howToFormat = QLatin1String("My Tags/{City}");

        info.rgData[QLatin1String("country")] = country;
        info.rgData[QLatin1String("city")]    = city;

        QCOMPARE( makeTagString(info, howToFormat, QLatin1String("OSM")).last(), QString::fromUtf8("My Tags/București"));
    }

    {
        // No {City}
        RGInfo info;
        QString country = QString::fromUtf8("România");
        QString city    = QString::fromUtf8("");

        const QString howToFormat = QLatin1String("My Tags/{Country}/{City}");

        info.rgData[QLatin1String("country")] = country;
        info.rgData[QLatin1String("city")]    = city;

        QCOMPARE( makeTagString(info, howToFormat, QLatin1String("OSM")).last(), QString::fromUtf8("My Tags/România"));
    }

    {
        // {Ctonury} instead of {Country} -> parsing is aborted
        RGInfo info;
        QString country = QString::fromUtf8("România");
        QString city    = QString::fromUtf8("București");

        const QString howToFormat = QLatin1String("My Tags/{Ctonury}/{City}");

        info.rgData[QLatin1String("country")] = country;
        info.rgData[QLatin1String("city")]    = city;

        QCOMPARE( makeTagString(info, howToFormat, QLatin1String("OSM")).last(), QString::fromUtf8("My Tags/"));
    }

    {
        // no retrieved data
        RGInfo info;
        QString country = QString::fromUtf8("");
        QString city    = QString::fromUtf8("");

        const QString howToFormat = QLatin1String("My Tags/{Country}/{City}");

        info.rgData[QLatin1String("country")] = country;
        info.rgData[QLatin1String("city")]    = city;

        QCOMPARE( makeTagString(info, howToFormat, QLatin1String("OSM")).last(), QString::fromUtf8("My Tags"));
    }

    // TODO:
    // - some fields are missing (no country)
    // * simply deletes {Country}
    // - invalid field names
    // * if we have {Contry} or {bla} it deletes the etiquette
    // - how do we handle these cases in the UI?
    // * now that we have makeTagString separate, these are handled same in tests and UI
}

QTEST_GUILESS_MAIN(TestRGParsing)
