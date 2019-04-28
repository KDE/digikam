/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QObject>
#include <QtTest>

// KDE includes

#include <kjob.h>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_parse.h"
#include "fakeserver/fakeserver.h"

using MediaWiki::Iface;
using MediaWiki::Parse;

Q_DECLARE_METATYPE(FakeServer::Request)
Q_DECLARE_METATYPE(QVariant)
Q_DECLARE_METATYPE(Parse*)

QString QStringFromFile(const QString& fileName)
{
    QFile file(fileName);

    if (!file.open( QFile::ReadOnly ))
        return QString();

    QTextStream in(&file);
    QString scenario;
    // When loading from files we never have the authentication phase
    // force jumping directly to authenticated state.

    while ( !in.atEnd() )
    {
        scenario.append( in.readLine() );
    }

    file.close();
    return scenario;
}

class Q_DECL_HIDDEN ParseTest : public QObject
{
    Q_OBJECT

public:

    ParseTest()
    {
        parseCount  = 0;
        m_mediaWiki = nullptr;
    }

public Q_SLOTS:

    void parseHandle(KJob* job)
    {
        Q_UNUSED(job)
        parseCount++;
    }

    void resultHandle(const QString& result)
    {
        parseResult = result;
    }

private Q_SLOTS:

    void initTestCase()
    {
        parseCount        = 0;
        this->m_mediaWiki = new Iface(QUrl(QStringLiteral("http://127.0.0.1:12566")));
    }

    void result()
    {
        QString scenario = QStringFromFile(QStringLiteral("./data/parsetest_result.rc"));
        QString result   = QStringFromFile(QStringLiteral("./data/parsetest_resulttrue.rc"));

        Parse* const job = new Parse(*m_mediaWiki, nullptr);
        parseCount       = 0;
        FakeServer fakeserver;
        fakeserver.setScenario(scenario);
        fakeserver.startAndWait();

        connect(job, SIGNAL(result(QString)),
                this, SLOT(resultHandle(QString)));

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(parseHandle(KJob*)));

        job->exec();
        QCOMPARE(parseResult, result);
        QCOMPARE(this->parseCount, 1);
    }

    void parseSetters()
    {
        QFETCH(QString, scenario);
        QFETCH(QString, request);
        QFETCH(Parse*, job);

        parseCount = 0;
        FakeServer fakeserver;
        fakeserver.setScenario(scenario);
        fakeserver.startAndWait();

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(parseHandle(KJob*)));

        job->exec();
        FakeServer::Request serverrequest = fakeserver.getRequest()[0];
        QCOMPARE(serverrequest.type, QStringLiteral("GET"));
        QCOMPARE(serverrequest.value, request);
        QCOMPARE(this->parseCount, 1);
    }

    void parseSetters_data()
    {
        QTest::addColumn<QString>("scenario");
        QTest::addColumn<QString>("request");
        QTest::addColumn<Parse*>("job");

        Parse* const p1 = new Parse( *m_mediaWiki, nullptr);
        p1->setText(QStringLiteral("listedecharacteres"));

        QTest::newRow("Text")
                << QStringFromFile(QStringLiteral("./data/parsetest.rc"))
                << QStringLiteral("/?format=xml&action=parse&text=listedecharacteres")
                << p1;

        Parse* const p2 = new Parse( *m_mediaWiki, nullptr);
        p2->setPageName(QStringLiteral("listedecharacteres"));

        QTest::newRow("Page Name")
                << QStringFromFile(QStringLiteral("./data/parsetest.rc"))
                << QStringLiteral("/?format=xml&action=parse&page=listedecharacteres")
                << p2;

        Parse* const p3 = new Parse( *m_mediaWiki, nullptr);
        p3->setTitle(QStringLiteral("listedecharacteres"));

        QTest::newRow("Title")
                << QStringFromFile(QStringLiteral("./data/parsetest.rc"))
                << QStringLiteral("/?format=xml&action=parse&title=listedecharacteres")
                << p3;

        Parse* const p4 = new Parse( *m_mediaWiki, nullptr);
        p4->setUseLang(QStringLiteral("fr"));

        QTest::newRow("User Langue")
                << QStringFromFile(QStringLiteral("./data/parsetest.rc"))
                << QStringLiteral("/?format=xml&action=parse&uselang=fr")
                << p4;
    }

    void error()
    {
        QFETCH(QString, scenario);
        QFETCH(int, error);
        parseCount = 0;
        FakeServer fakeserver;

        if (scenario != QStringLiteral("error serveur"))
        {
            fakeserver.addScenario(scenario);
            fakeserver.startAndWait();
        }

        Parse* const job = new Parse(*m_mediaWiki);
        job->setTitle( QStringLiteral("title") );

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(parseHandle(KJob*)));

        job->exec();

        if (scenario != QStringLiteral("error serveur"))
        {
            QList<FakeServer::Request> requests = fakeserver.getRequest();
            QCOMPARE(requests.size(), 1);
        }

        QCOMPARE(job->error(), error);
        QCOMPARE(parseCount, 1);

        if (scenario != QStringLiteral("error serveur"))
        {
            QVERIFY(fakeserver.isAllScenarioDone());
        }
    }

    void error_data()
    {
        QTest::addColumn<QString>("scenario");
        QTest::addColumn<int>("error");

        QTest::newRow("XML")
                << QStringLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?><api><parse><text>")
                << int(Parse::XmlError);

        QTest::newRow("Network")
                << QStringLiteral("error serveur")
                << int(Parse::NetworkError);

        QTest::newRow("Params")
                << QStringLiteral("<api><error code=\"params\" info=\"\"/></api>")
                << int(Parse::TooManyParams);

        QTest::newRow("Missing Title")
                << QStringLiteral("<api><error code=\"missingtitle\" info=\"\"/></api>")
                << int(Parse::MissingPage);
    }

    void cleanupTestCase()
    {
        delete this->m_mediaWiki;
    }

private:

    int        parseCount;
    QString    request;
    QString    parseResult;
    Iface*     m_mediaWiki;
};

QTEST_MAIN(ParseTest)

#include "parsetest.moc"
