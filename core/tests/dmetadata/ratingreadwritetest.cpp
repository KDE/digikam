/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-12
 * Description : DMetadata Settings Tests for getImageRating and setImageRating.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "ratingreadwritetest.h"

// Qt includes

#include <QTest>
#include <QStringList>
#include <QString>
#include <QDebug>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(RatingReadWriteTest)

void RatingReadWriteTest::initTestCase()
{
}

void RatingReadWriteTest::testSimpleReadAfterWrite()
{
    DMetadata dmeta;

    // Trick dmetadata, so it will think that we have a file path
    dmeta.setFilePath(QLatin1String("random.org"));
    int rez = -1;

    qDebug() << dmeta.supportXmp();

    for (int i = 0; i < 6; i++)
    {
        dmeta.setImageRating(i);
        rez = dmeta.getImageRating();
        QCOMPARE(rez, i);
    }
}

void RatingReadWriteTest::testWriteToDisabledNamespaces()
{
    DMetadata dmeta;
    dmeta.setFilePath(QLatin1String("random.org"));

    DMetadataSettingsContainer dmsettings;

    QList<int> defaultVal, microsoftMappings, iptcMappings;
    defaultVal << 0 << 1 << 2 << 3 << 4 << 5;
    microsoftMappings << 0 << 1 << 25 << 50 << 75 << 99;
    iptcMappings << 8 << 6 << 5 << 4 << 2 << 1;

    NamespaceEntry ratingNs2;
    ratingNs2.namespaceName = QLatin1String("Xmp.acdsee.rating");
    ratingNs2.convertRatio  = defaultVal;
    ratingNs2.nsType        = NamespaceEntry::RATING;
    ratingNs2.index         = 1;
    ratingNs2.subspace = NamespaceEntry::XMP;
    ratingNs2.isDisabled    = true;

    NamespaceEntry ratingNs3;
    ratingNs3.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.Rating");
    ratingNs3.convertRatio  = microsoftMappings;
    ratingNs3.nsType        = NamespaceEntry::RATING;
    ratingNs3.index         = 2;
    ratingNs3.subspace = NamespaceEntry::XMP;


    dmsettings.getWriteMapping(QLatin1String(DM_RATING_CONTAINER)).clear();
    dmsettings.getWriteMapping(QLatin1String(DM_RATING_CONTAINER))
             << ratingNs2
             << ratingNs3;

    for (int i = 0; i < 6; i++)
    {
        dmeta.setImageRating(i, dmsettings);

        QString data;
        bool ok;

        data    = dmeta.getXmpTagString("Xmp.acdsee.rating", false);

        QVERIFY(data.isEmpty());

        data    = dmeta.getXmpTagString("Xmp.MicrosoftPhoto.Rating", false);
        int rez = data.toInt(&ok);

        QCOMPARE(ok, true);
        QCOMPARE(rez, microsoftMappings.at(i));
    }
}

void RatingReadWriteTest::testReadFromDisabledNamespaces()
{
    DMetadata dmeta;
    dmeta.setFilePath(QLatin1String("random.org"));

    DMetadataSettingsContainer dmsettings;

    QList<int> defaultVal, microsoftMappings, iptcMappings;
    defaultVal << 0 << 1 << 2 << 3 << 4 << 5;
    microsoftMappings << 0 << 1 << 25 << 50 << 75 << 99;
    iptcMappings << 8 << 6 << 5 << 4 << 2 << 1;

    NamespaceEntry ratingNs2;
    ratingNs2.namespaceName = QLatin1String("Xmp.acdsee.rating");
    ratingNs2.convertRatio  = defaultVal;
    ratingNs2.nsType        = NamespaceEntry::RATING;
    ratingNs2.index         = 1;
    ratingNs2.subspace = NamespaceEntry::XMP;
    ratingNs2.isDisabled    = true;

    NamespaceEntry ratingNs3;
    ratingNs3.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.Rating");
    ratingNs3.convertRatio  = microsoftMappings;
    ratingNs3.nsType        = NamespaceEntry::RATING;
    ratingNs3.index         = 2;
    ratingNs3.subspace = NamespaceEntry::XMP;


    dmsettings.getReadMapping(QLatin1String(DM_RATING_CONTAINER)).clear();
    dmsettings.getReadMapping(QLatin1String(DM_RATING_CONTAINER))
             << ratingNs2
             << ratingNs3;

    for (int i = 0; i < 6; i++)
    {

        dmeta.setXmpTagString("Xmp.acdsee.rating", QString::number(5-i));
        dmeta.setXmpTagString("Xmp.MicrosoftPhoto.Rating", QString::number(microsoftMappings.at(i)));

        int rez = dmeta.getImageRating(dmsettings);
        QCOMPARE(rez, i);
    }
}
