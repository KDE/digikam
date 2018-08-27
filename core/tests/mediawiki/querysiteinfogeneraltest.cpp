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

#ifndef TEST_GENERAL_H
#define TEST_GENERAL_H

// Qt includes

#include <QObject>
#include <QtTest>

// KDE includes

#include <kjob.h>

// Local includes

#include "fakeserver/fakeserver.h"
#include "mediawiki_iface.h"
#include "mediawiki_querysiteinfogeneral.h"
#include "mediawiki_generalinfo.h"

using MediaWiki::Iface;
using MediaWiki::QuerySiteInfoGeneral;
using MediaWiki::Generalinfo;

class QuerySiteInfoGeneralTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:

void resultHandle(const Generalinfo& generalinfo)
{
    ++generalCount;
    generalResult = generalinfo;
}

private Q_SLOTS:

void initTestCase()
{
    generalCount  = 0;
    this->request = QStringLiteral("/?format=xml&action=query&meta=siteinfo&siprop=general");
}

void QuerySiteInfoGeneralTestConnectTrue()
{
    Iface mediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));
    FakeServer server;

    generalCount = 0;
    QString senario(QStringLiteral("<api><query><general mainpage=\"Main Page\" base=\"http://en.wikipedia.org/wiki/Main_Page\" sitename=\"Wikipedia\" generator=\"Iface 1.16wmf4\" phpversion=\"5.2.4-2ubuntu5.12wm1\" phpsapi=\"apache2handler\" dbtype=\"mysql\" dbversion=\"5.1.46-facebook-r3489-log\" rev=\"75268\" case=\"first-letter\" rights=\"Creative Commons Attribution-Share Alike 3.0 Unported\" lang=\"en\" fallback8bitEncoding=\"windows-1252\" writeapi=\"\" timezone=\"UTC\" timeoffset=\"0\" articlepath=\"/wiki/$1\" scriptpath=\"/w\" script=\"/w/index.php\" variantarticlepath=\"\" server=\"http://en.wikipedia.org\" wikiid=\"enwiki\" time=\"2010-10-24T19:53:13Z\"/></query></api>"));
    server.addScenario(senario);
    server.startAndWait();

    QuerySiteInfoGeneral* const general = new QuerySiteInfoGeneral(mediaWiki);

    connect(general, SIGNAL(result(Generalinfo)),
            this, SLOT(resultHandle(Generalinfo)));

    general->exec();
    FakeServer::Request serverrequest = server.getRequest()[0];
    QCOMPARE(this->generalCount, 1);
    QCOMPARE(serverrequest.type, QStringLiteral("GET"));
    QCOMPARE(serverrequest.value, this->request);
    QVERIFY(general->error() == QuerySiteInfoGeneral::NoError);
}

void QuerySiteInfoGeneralTestAttribute()
{
    Iface mediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));
    FakeServer server;

    generalCount = 0;
    QString senario(QStringLiteral("<api><query><general mainpage=\"Main Page\" base=\"http://en.wikipedia.org/wiki/Main_Page\" sitename=\"Wikipedia\" generator=\"Iface 1.16wmf4\" phpversion=\"5.2.4-2ubuntu5.12wm1\" phpsapi=\"apache2handler\" dbtype=\"mysql\" dbversion=\"5.1.46-facebook-r3489-log\" rev=\"75268\" case=\"first-letter\" rights=\"Creative Commons Attribution-Share Alike 3.0 Unported\" lang=\"en\" fallback8bitEncoding=\"windows-1252\" writeapi=\"\" timezone=\"UTC\" timeoffset=\"0\" articlepath=\"/wiki/$1\" scriptpath=\"/w\" script=\"/w/index.php\" variantarticlepath=\"\" server=\"http://en.wikipedia.org\" wikiid=\"enwiki\" time=\"2010-10-24T19:53:13Z\"/></query></api>"));
    server.addScenario(senario);
    server.startAndWait();

    QuerySiteInfoGeneral* const general = new QuerySiteInfoGeneral(mediaWiki);

    connect(general, SIGNAL(result(Generalinfo)),
            this, SLOT(resultHandle(Generalinfo)));

    general->exec();
    FakeServer::Request serverrequest = server.getRequest()[0];
    QCOMPARE(this->generalCount, 1);
    QCOMPARE(serverrequest.type, QStringLiteral("GET"));
    QCOMPARE(serverrequest.value, this->request);
    QVERIFY(general->error() == QuerySiteInfoGeneral::NoError);
    Generalinfo resultExpected;
    resultExpected.setMainPage(QStringLiteral("Main Page"));
    resultExpected.setUrl(QUrl(QStringLiteral("http://en.wikipedia.org/wiki/Main_Page")));
    resultExpected.setSiteName(QStringLiteral("Wikipedia"));
    resultExpected.setGenerator(QStringLiteral("Iface 1.16wmf4"));
    resultExpected.setPhpVersion(QStringLiteral("5.2.4-2ubuntu5.12wm1"));
    resultExpected.setPhpApi(QStringLiteral("apache2handler"));
    resultExpected.setDataBaseType(QStringLiteral("mysql"));
    resultExpected.setDataBaseVersion(QStringLiteral("5.1.46-facebook-r3489-log"));
    resultExpected.setRev(QStringLiteral("75268"));
    resultExpected.setCas(QStringLiteral("first-letter"));
    resultExpected.setLicence(QStringLiteral("Creative Commons Attribution-Share Alike 3.0 Unported"));
    resultExpected.setLanguage(QStringLiteral("en"));
    resultExpected.setFallBack8bitEncoding(QStringLiteral("windows-1252"));
    resultExpected.setWriteApi(QStringLiteral(""));
    resultExpected.setTimeZone(QStringLiteral("UTC"));
    resultExpected.setTimeOffset(QStringLiteral("0"));
    resultExpected.setArticlePath(QStringLiteral("/wiki/$1"));
    resultExpected.setScriptPath(QStringLiteral("/w"));
    resultExpected.setScript(QStringLiteral("/w/index.php"));
    resultExpected.setVariantArticlePath(QString());
    resultExpected.setServerUrl(QUrl(QStringLiteral("http://en.wikipedia.org")));
    resultExpected.setWikiId(QStringLiteral("enwiki"));
    resultExpected.setTime(QDateTime(QDate(2010, 10, 24), QTime(19, 53, 13)));
    QCOMPARE(generalResult, resultExpected);
}

void QuerySiteInfoGeneralTestConnectFalseXML()
{
    Iface mediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));
    FakeServer server;

    generalCount = 0;
    QString senario(QStringLiteral("<api><query<general mainpage=\"Main Page\" base=\"http://en.wikipedia.org/wiki/Main_Page\" sitename=\"Wikipedia\" generator=\"Iface 1.16wmf4\" phpversion=\"5.2.4-2ubuntu5.12wm1\" phpsapi=\"apache2handler\" dbtype=\"mysql\" dbversion=\"5.1.46-facebook-r3489-log\" rev=\"75268\" case=\"first-letter\" rights=\"Creative Commons Attribution-Share Alike 3.0 Unported\" lang=\"en\" fallback8bitEncoding=\"windows-1252\" writeapi=\"\" timezone=\"UTC\" timeoffset=\"0\" articlepath=\"/wiki/$1\" scriptpath=\"/w\" script=\"/w/index.php\" variantarticlepath=\"\" server=\"http://en.wikipedia.org\" wikiid=\"enwiki\" time=\"2010-10-24T19:53:13Z\"/>"));
    server.addScenario(senario);
    server.startAndWait();

    QuerySiteInfoGeneral* const general = new QuerySiteInfoGeneral(mediaWiki);

    connect(general, SIGNAL(result(Generalinfo)),
            this, SLOT(resultHandle(Generalinfo)));

    general->exec();
    FakeServer::Request serverrequest = server.getRequest()[0];
    QCOMPARE(this->generalCount, 0);
    QCOMPARE(serverrequest.type, QStringLiteral("GET"));
    QCOMPARE(serverrequest.value, this->request);
    QVERIFY(general->error() == QuerySiteInfoGeneral::XmlError);
}

void QuerySiteInfoGeneralTestErrortIncludeAllDenied()
{
    Iface mediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));
    FakeServer server;

    generalCount = 0;
    QString senario(QStringLiteral("<api><error code=\"includeAllDenied\" info=\"\"/></api>"));
    server.addScenario(senario);
    server.startAndWait();

    QuerySiteInfoGeneral* const general = new QuerySiteInfoGeneral(mediaWiki);

    connect(general, SIGNAL(result(Generalinfo)),
            this, SLOT(resultHandle(Generalinfo)));

    general->exec();
    FakeServer::Request serverrequest = server.getRequest()[0];
    QCOMPARE(this->generalCount, 0);
    QCOMPARE(serverrequest.type, QStringLiteral("GET"));
    QCOMPARE(serverrequest.value, this->request);
    QVERIFY(general->error() == QuerySiteInfoGeneral::IncludeAllDenied);
}

private:

    int         generalCount;
    Generalinfo generalResult;
    QString     request;
};

QTEST_MAIN(QuerySiteInfoGeneralTest)

#include "querysiteinfogeneraltest.moc"

#endif // TEST_GENERAL_H
