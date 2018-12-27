/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
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

#ifndef TEST_UPLOAD_H
#define TEST_UPLOAD_H

// Qt includes

#include <QObject>
#include <QtTest>

// KDE includes

#include <kjob.h>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_upload.h"
#include "fakeserver/fakeserver.h"

using MediaWiki::Iface;
using MediaWiki::Upload;

Q_DECLARE_METATYPE(FakeServer::Request)
Q_DECLARE_METATYPE(Upload*)

class Q_DECL_HIDDEN UploadTest : public QObject
{
    Q_OBJECT

public:

    UploadTest()
    {
        uploadCount = 0;
        m_file      = 0;
        m_mediaWiki = 0;
    }

public Q_SLOTS:

    void uploadHandle(KJob*)
    {
        uploadCount++;
    }

private Q_SLOTS:

    void initTestCase()
    {
        uploadCount          = 0;
        this->m_mediaWiki    = new Iface(QUrl(QStringLiteral("http://127.0.0.1:12566")));
        this->m_infoScenario = QStringLiteral("<api><query><pages><page pageid=\"27697087\" ns=\"0\" title=\"API\" touched=\"2010-11-25T13:59:03Z\" lastrevid=\"367741756\" counter=\"0\" length=\"70\" redirect=\"\" starttimestamp=\"2010-11-25T16:14:51Z\" edittoken=\"cecded1f35005d22904a35cc7b736e18%2B\" talkid=\"5477418\" fullurl=\"http://en.wikipedia.org/wiki/API\" editurl=\"http://en.wikipedia.org/w/index.php?title=API&action=edit\" ><protection /></page></pages></query></api>");
        this->m_file         = new QFile(QCoreApplication::applicationFilePath() + QStringLiteral("_image.jpg"));
        this->m_file->open(QIODevice::ReadOnly);
    }

    void uploadSetters()
    {
        QFETCH(QString, request);
        QFETCH(QString, senario);
        QFETCH(Upload*, job);

        uploadCount = 0;
        FakeServer fakeserver;
        fakeserver.setScenario(m_infoScenario);
        fakeserver.addScenario(senario);
        fakeserver.startAndWait();

        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(uploadHandle(KJob*)));

        job->exec();
        FakeServer::Request serverrequest = fakeserver.getRequest()[1];
        QCOMPARE(serverrequest.type, QStringLiteral("POST"));
        QCOMPARE(serverrequest.value, request);
        QCOMPARE(job->error(), (int)Upload::NoError);
        QCOMPARE(this->uploadCount, 1);
    }

    void uploadSetters_data()
    {
        QTest::addColumn<QString>("request");
        QTest::addColumn<QString>("senario");
        QTest::addColumn<Upload*>("job");

        Upload* const e1 = new Upload( *m_mediaWiki, NULL);
        e1->setFile(this->m_file);
        e1->setFilename(QStringLiteral("Test.jpeg"));
        e1->setComment(QStringLiteral("Test"));
        e1->setText(QStringLiteral("{{Information|Description=Ajout du logo de l'IUP ISI, Toulouse.|Source=http://www.iupisi.ups-tlse.fr/|Date=1992-01-01|Author=iup|Permission={{PD-EEA}}|other_versions=}}"));
        QTest::newRow("Text")
                << QStringLiteral("/?action=upload&format=xml")
                << QStringLiteral("<api><upload result=\"Success\" pageid=\"12\" title=\"Talk:Main Page\" oldrevid=\"465\" newrevid=\"471\" /></api>")
                << e1;
    }

    void error()
    {
        QFETCH(QString, scenario);
        QFETCH(int, error);

        uploadCount = 0;
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));
        FakeServer fakeserver;

        if (scenario != QStringLiteral("error server"))
        {
            fakeserver.setScenario(m_infoScenario);
            fakeserver.addScenario(scenario);
            fakeserver.startAndWait();
        }

        Upload* const job = new Upload(MediaWiki, NULL);
        job->setFile(this->m_file);
        job->setFilename(QStringLiteral("Test.jpeg"));
        job->setComment(QStringLiteral("Test"));
        job->setText(QStringLiteral("{{Information|Description=Ajout du logo de l'IUP ISI, Toulouse.|Source=http://www.iupisi.ups-tlse.fr/|Date=1992-01-01|Author=iup|Permission={{PD-EEA}}|other_versions=}}"));

        connect(job,  SIGNAL(result(KJob*)),
                this, SLOT(uploadHandle(KJob*)));

        job->exec();

        if (scenario != QStringLiteral("error server"))
        {
            QList<FakeServer::Request> requests = fakeserver.getRequest();
            QCOMPARE(requests.size(), 2);
        }

        QCOMPARE(job->error(), error);
        QCOMPARE(uploadCount, 1);

        if (scenario != QStringLiteral("error server"))
        {
            QVERIFY(fakeserver.isAllScenarioDone());
        }
    }

    void error_data()
    {
        QTest::addColumn<QString>("scenario");
        QTest::addColumn<int>("error");

        QTest::newRow("UploadDisabled")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"upload-disabled\" info=\"\" /></upload> </api>")
                << int(Upload::UploadDisabled);

        QTest::newRow("InvalidSessionKey")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"invalid-session-key\" info=\"\" /></upload> </api>")
                << int(Upload::InvalidSessionKey);

        QTest::newRow("BadAccess")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"bad-access-groups\" info=\"\" /></upload> </api>")
                << int(Upload::BadAccess);

        QTest::newRow("ParamMissing")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"missing-param\" info=\"\" /></upload> </api>")
                << int(Upload::ParamMissing);

        QTest::newRow("MustBeLoggedIn")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"mustbeloggedin\" info=\"\" /></upload> </api>")
                << int(Upload::MustBeLoggedIn);

        QTest::newRow("FetchFileError")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"fetchfile-error\" info=\"\" /></upload> </api>")
                << int(Upload::FetchFileError);

        QTest::newRow("NoModule")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"no-module\" info=\"\" /></upload> </api>")
                << int(Upload::NoModule);

        QTest::newRow("EmptyFile")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"emptyfile\" info=\"\" /></upload> </api>")
                << int(Upload::EmptyFile);

        QTest::newRow("ExtensionMissing")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"filetype-missing\" info=\"\" /></upload> </api>")
                << int(Upload::ExtensionMissing);

        QTest::newRow("TooShortFilename")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"filenametooshort\" info=\"\" /></upload> </api>")
                << int(Upload::TooShortFilename);

        QTest::newRow("OverWriting")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"overwrite\" info=\"\" /></upload> </api>")
                << int(Upload::OverWriting);

        QTest::newRow("StashFailed")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"stashfailed\" info=\"\" /></upload> </api>")
                << int(Upload::StashFailed);

        QTest::newRow("InternalError")
                << QStringLiteral("<api><upload result=\"Failure\"><error code=\"internal-error\" info=\"\" /></upload> </api>")
                << int(Upload::InternalError);
    }

    void cleanupTestCase()
    {
        delete this->m_mediaWiki;
        delete this->m_file;
    }

private:

    int        uploadCount;
    QString    request;
    QString    m_infoScenario;
    QIODevice* m_file;
    Iface*     m_mediaWiki;
};

QTEST_MAIN(UploadTest)

#include "uploadtest.moc"

#endif // TEST_UPLOAD_H
