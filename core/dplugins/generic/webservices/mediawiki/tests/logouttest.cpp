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
#include "mediawiki_logout.h"
#include "fakeserver/fakeserver.h"

using MediaWiki::Iface;
using MediaWiki::Logout;

class Q_DECL_HIDDEN LogoutTest : public QObject
{
    Q_OBJECT

public:

    LogoutTest()
    {
        logoutCount = 0;
        m_mediaWiki = nullptr;
        m_server    = nullptr;
    }

public Q_SLOTS:

    void logoutHandle(KJob* job)
    {
        Q_UNUSED(job)
        logoutCount++;
    }

private Q_SLOTS:

    void initTestCase()
    {
        logoutCount       = 0;
        this->m_mediaWiki = new Iface(QUrl(QStringLiteral("http://127.0.0.1:12566")));
        this->m_server    = new FakeServer;
        this->request     = QStringLiteral("/?format=xml&action=logout");
    }

    void logoutTestConnectTrue()
    {
        QString senario(QStringLiteral("<api />") );
        QString cookie( QStringLiteral("cookieprefix=\"enwiki\" sessionid=\"17ab96bd8ffbe8ca58a78657a918558e\" expires=\"Sat, 12-Feb-2011 21:39:30 GMT\""));
        m_server->setScenario(senario, cookie);
        m_server->startAndWait();

        logoutCount = 0;
        Logout logout(*m_mediaWiki);

        connect(&logout, SIGNAL(result(KJob*)),
                this, SLOT(logoutHandle(KJob*)));

        logout.exec();   // krazy:exclude=crashy
        QCOMPARE(this->logoutCount, 1);
        QCOMPARE(logout.error(), (int)Logout::NoError);

        QList<FakeServer::Request> requests = m_server->getRequest();
        QCOMPARE(requests.size(), 1);

        FakeServer::Request request = requests[0];
        QCOMPARE(request.agent, m_mediaWiki->userAgent());
        QCOMPARE(request.type, QStringLiteral("GET"));
        QCOMPARE(request.value, QStringLiteral("/?format=xml&action=logout"));
    }

    void cleanupTestCase()
    {
        delete this->m_mediaWiki;
        delete this->m_server;
    }

private:

    int         logoutCount;
    QString     request;
    Iface*      m_mediaWiki;
    FakeServer* m_server;
};

QTEST_MAIN(LogoutTest)

#include "logouttest.moc"
