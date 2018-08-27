/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * Copyright (C) 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
 * Copyright (C) 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <QObject>
#include <QString>
#include <QUrl>
#include <QtTest>

#include "mediawiki_iface.h"

using mediawiki::Iface;

class IfaceTest : public QObject
{

    Q_OBJECT

private Q_SLOTS:

    void testConstructor() {
        QFETCH(QUrl, url);
        QFETCH(QString, customUserAgent);
        QFETCH(QString, userAgent);

        Iface mediawiki(url, customUserAgent);

        QCOMPARE(mediawiki.url(), url);
        QCOMPARE(mediawiki.userAgent(), userAgent);
    }

    void testConstructor_data() {
        QTest::addColumn<QUrl>("url");
        QTest::addColumn<QString>("customUserAgent");
        QTest::addColumn<QString>("userAgent");

        QTest::newRow("") << QUrl(QStringLiteral("http://127.0.0.1:12566")) << QString() << QStringLiteral("mediawiki-silk");
        QTest::newRow("") << QUrl(QStringLiteral("commons.wikimedia.org/w/api.php")) << QString() << QStringLiteral("mediawiki-silk");
        QTest::newRow("") << QUrl(QStringLiteral("http://commons.wikimedia.org/w/api.php")) << QStringLiteral("test1") << QStringLiteral("test1-mediawiki-silk");
        QTest::newRow("") << QUrl(QStringLiteral("http://commons.wikimedia.org/w/api.php/")) << QStringLiteral("test2") << QStringLiteral("test2-mediawiki-silk");
    }

};

QTEST_MAIN(IfaceTest)

#include "mediawikitest.moc"
