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

#ifndef TEST_QUERYINFO_H
#define TEST_QUERYINFO_H

// Qt includes

#include <QObject>
#include <QtTest>

// KDE includes

#include <kjob.h>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_queryinfo.h"
#include "mediawiki_page.h"
#include "mediawiki_protection.h"
#include "fakeserver/fakeserver.h"

using MediaWiki::Iface;
using MediaWiki::QueryInfo;
using MediaWiki::Page;
using MediaWiki::Protection;

Q_DECLARE_METATYPE(Page)
Q_DECLARE_METATYPE(Protection)
Q_DECLARE_METATYPE(QueryInfo*)
Q_DECLARE_METATYPE(QVector <Protection>)

class QueryInfoTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:

    void queryInfoHandlePages(const Page& page)
    {
        ++queryInfoCount;
        queryInfoResultsPage = page;
    }

    void queryInfoHandleProtection(const QVector<Protection>& protection)
    {
        ++queryInfoCount;
        queryInfoResultsProtections = protection;
    }

private Q_SLOTS:

    void initTestCase()
    {
        queryInfoCount    = 0;
        this->m_mediaWiki = new Iface(QUrl(QStringLiteral("http://127.0.0.1:12566")));
    }

    void constructQuery()
    {
        QFETCH(QString, request);
        QFETCH(QueryInfo*, job);

        queryInfoCount = 0;
        FakeServer fakeserver;
        fakeserver.startAndWait();

        job->exec();

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);

        FakeServer::Request requestServeur = requests[0];
        QCOMPARE(requestServeur.agent, m_mediaWiki->userAgent());
        QCOMPARE(requestServeur.type, QStringLiteral("GET"));
        QCOMPARE(requestServeur.value, request);
    }

    void constructQuery_data()
    {
        QTest::addColumn<QString>("request");
        QTest::addColumn<QueryInfo*>("job");

        QueryInfo* const j1 = new QueryInfo(*m_mediaWiki);
        j1->setPageName(QStringLiteral("API"));

        QTest::newRow("Name pages")
                << QStringLiteral("/?format=xml&action=query&prop=info&inprop=protection%7Ctalkid%7Cwatched%7Csubjectid%7Curl%7Creadable%7Cpreload&titles=API")
                << j1;

        QueryInfo* const j2 = new QueryInfo(*m_mediaWiki);
        j2->setToken( QStringLiteral("cecded1f35005d22904a35cc7b736e18+\\") );

        QTest::newRow("Token")
                << QStringLiteral("/?format=xml&action=query&prop=info&inprop=protection%7Ctalkid%7Cwatched%7Csubjectid%7Curl%7Creadable%7Cpreload&intoken=cecded1f35005d22904a35cc7b736e18+%5C")
                << j2;

        QueryInfo* const j3 = new QueryInfo(*m_mediaWiki);
        j3->setPageId(25255);

        QTest::newRow("Page Id")
                << QStringLiteral("/?format=xml&action=query&prop=info&inprop=protection%7Ctalkid%7Cwatched%7Csubjectid%7Curl%7Creadable%7Cpreload&pageids=25255")
                << j3;

        QueryInfo *j4 = new QueryInfo(*m_mediaWiki);
        j4->setRevisionId(44545);

        QTest::newRow("Revision Id")
                << QStringLiteral("/?format=xml&action=query&prop=info&inprop=protection%7Ctalkid%7Cwatched%7Csubjectid%7Curl%7Creadable%7Cpreload&revids=44545")
                << j4;
    }

    void parseData()
    {
        QFETCH(QString,scenario);
        QFETCH(Page ,page);
        QFETCH(QVector<Protection> ,protections);

        QueryInfo job(*m_mediaWiki);
        queryInfoCount = 0;
        FakeServer fakeserver;
        fakeserver.addScenario(scenario);
        fakeserver.startAndWait();

        connect(&job, SIGNAL(page(Page)),
                this,SLOT(queryInfoHandlePages(Page)));

        connect(&job, SIGNAL(protection(QVector<Protection>)),
                this,SLOT(queryInfoHandleProtection(QVector<Protection>)));

        job.exec();

        QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 1);

        QCOMPARE(queryInfoCount, 2);
        QCOMPARE(queryInfoResultsPage, page);
        QCOMPARE(queryInfoResultsProtections, protections);
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void parseData_data()
    {
        QTest::addColumn<QString>("scenario");
        QTest::addColumn< Page >("page");
        QTest::addColumn< QVector<Protection> >("protections");

        Protection pr1;
        pr1.setType(QStringLiteral("edit"));
        pr1.setLevel(QStringLiteral("sysop"));
        pr1.setExpiry(QStringLiteral("infinity"));
        pr1.setSource(QString());

        Protection pr2;
        pr2.setType(QStringLiteral("move"));
        pr2.setLevel(QStringLiteral("sysop"));
        pr2.setExpiry(QStringLiteral("infinity"));
        pr2.setSource(QString());

        Page page;
        page.setPageId(27697087);
        page.setTitle(QStringLiteral("API"));
        page.setNs(0);
        page.setTouched( QDateTime::fromString(QStringLiteral("2010-11-25T13:59:03Z"), QStringLiteral("yyyy'-'MM'-'dd'T'hh':'mm':'ss'Z'")) );
        page.setLastRevId(367741756);
        page.setCounter(0);
        page.setLength(70);
        page.setStarttimestamp(QDateTime::fromString(QStringLiteral("2010-11-25T16:14:51Z"), QStringLiteral("yyyy'-'MM'-'dd'T'hh':'mm':'ss'Z'")));
        page.setEditToken(QStringLiteral("+\\"));
        page.setTalkid(5477418);
        page.setFullurl(QUrl(QStringLiteral("http://en.wikipedia.org/wiki/API")));
        page.setEditurl(QUrl(QStringLiteral("http://en.wikipedia.org/w/index.php?title=API&action=edit")));
        page.setReadable(QString());
        page.setPreload(QString());

        QTest::newRow("No protection")
                << QStringLiteral("<api><query><pages><page pageid=\"27697087\" ns=\"0\" title=\"API\" touched=\"2010-11-25T13:59:03Z\" lastrevid=\"367741756\" counter=\"0\" length=\"70\" redirect=\"\" starttimestamp=\"2010-11-25T16:14:51Z\" edittoken=\"+\\\" talkid=\"5477418\" fullurl=\"http://en.wikipedia.org/wiki/API\" editurl=\"http://en.wikipedia.org/w/index.php?title=API&action=edit\" ><protection /></page></pages></query></api>")
                << page
                << QVector<Protection>();

        QTest::newRow("One pages and one protection")
                << QStringLiteral("<api><query><pages><page pageid=\"27697087\" ns=\"0\" title=\"API\" touched=\"2010-11-25T13:59:03Z\" lastrevid=\"367741756\" counter=\"0\" length=\"70\" redirect=\"\" starttimestamp=\"2010-11-25T16:14:51Z\" edittoken=\"+\\\" talkid=\"5477418\" fullurl=\"http://en.wikipedia.org/wiki/API\" editurl=\"http://en.wikipedia.org/w/index.php?title=API&action=edit\" ><protection><pr type=\"edit\" level=\"sysop\" expiry=\"infinity\"/></protection></page></pages></query></api>")
                << page
                << (QVector<Protection>() << pr1);

        QTest::newRow("One pages and two protection")
                << QStringLiteral("<api><query><pages><page pageid=\"27697087\" ns=\"0\" title=\"API\" touched=\"2010-11-25T13:59:03Z\" lastrevid=\"367741756\" counter=\"0\" length=\"70\" redirect=\"\" starttimestamp=\"2010-11-25T16:14:51Z\" edittoken=\"+\\\" talkid=\"5477418\" fullurl=\"http://en.wikipedia.org/wiki/API\" editurl=\"http://en.wikipedia.org/w/index.php?title=API&action=edit\" ><protection><pr type=\"edit\" level=\"sysop\" expiry=\"infinity\"/><pr type=\"move\" level=\"sysop\" expiry=\"infinity\"/></protection></page></pages></query></api>")
                << page
                << (QVector<Protection>() << pr1 << pr2);
    }

    void cleanupTestCase()
    {
        delete this->m_mediaWiki;
    }

private:

    int                  queryInfoCount;
    Page                 queryInfoResultsPage;
    QVector <Protection> queryInfoResultsProtections;
    Iface*               m_mediaWiki;
};

QTEST_MAIN(QueryInfoTest)

#include "queryinfotest.moc"

#endif // TEST_QUERYINFO_H
