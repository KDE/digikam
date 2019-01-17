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
#include <QTest>

// Local includes

#include "fakeserver/fakeserver.h"
#include "mediawiki_iface.h"
#include "mediawiki_imageinfo.h"
#include "mediawiki_queryimageinfo.h"

using namespace MediaWiki;

class Q_DECL_HIDDEN QueryImageinfoTest : public QObject
{
    Q_OBJECT

public:

    QueryImageinfoTest()
    {
        requestWithMissingTitleHasBeenSent = false;
    }

public Q_SLOTS:

    void resultHandle(const QList<Imageinfo>& imageinfos)
    {
        imageinfosReceived.push_back(imageinfos);
    }

    void missingTitleHandle(const QList<Imageinfo>& imageinfos)
    {
        Q_UNUSED(imageinfos)
        requestWithMissingTitleHasBeenSent = true;
    }

private Q_SLOTS:

    void init()
    {
        imageinfosReceived = QList<QList<Imageinfo> >();
    }

    void testQuery()
    {
        // Constructs the fakeserver
        FakeServer fakeserver;
        fakeserver.setScenario(QStringLiteral("<?xml version=\"1.0\"?><api><query><normalized><n from=\"Image:Image.bmp\" to=\"File:Image.bmp\" /></normalized><pages><page ns=\"6\" title=\"File:Image.bmp\" missing=\"\" imagerepository=\"shared\"><imageinfo><ii timestamp=\"2008-06-06T22:27:45Z\" user=\"User1\" size=\"448798\" width=\"924\" height=\"1203\" url=\"http://url/File:Image.bmp\" thumburl=\"http://thumburl/File:Image.bmp\" thumbwidth=\"78\" thumbheight=\"102\" descriptionurl=\"http://descriptionurl/File:Image.bmp\" comment=\"Comment1\" sha1=\"00be23585fde01190a0f8c60fc4267ea00f3745d\" mime=\"image/bmp\"><metadata><metadata name=\"Name1\" value=\"Value1\" /><metadata name=\"Name2\" value=\"Value2\" /></metadata></ii></imageinfo></page></pages></query><query-continue><imageinfo iistart=\"2007-06-06T22:27:45Z\" /></query-continue></api>"));
        fakeserver.addScenario(QStringLiteral("<?xml version=\"1.0\"?><api><query><normalized><n from=\"Image:Image.bmp\" to=\"File:Image.bmp\" /></normalized><pages><page ns=\"6\" title=\"File:Image.bmp\" missing=\"\" imagerepository=\"shared\"><imageinfo><ii timestamp=\"2007-06-06T22:27:45Z\" user=\"User2\" size=\"448798\" width=\"924\" height=\"1203\" url=\"http://url/File:Image.bmp\" descriptionurl=\"http://descriptionurl/File:Image.bmp\" comment=\"Comment2\" sha1=\"00be23585fde01190a0f8c60fc4267ea00f3745d\" mime=\"image/bmp\"><metadata><metadata name=\"Name1\" value=\"Value1\" /><metadata name=\"Name2\" value=\"Value2\" /></metadata></ii></imageinfo></page></pages></query></api>"));
        fakeserver.startAndWait();

        // Prepare the job
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));
        QueryImageinfo* const job = new QueryImageinfo(MediaWiki);
        job->setTitle(QStringLiteral("Image:Image.bmp"));

        job->setProperties(
            QueryImageinfo::Timestamp |
            QueryImageinfo::User      |
            QueryImageinfo::Comment   |
            QueryImageinfo::Url       |
            QueryImageinfo::Size      |
            QueryImageinfo::Sha1      |
            QueryImageinfo::Mime      |
            QueryImageinfo::Metadata);

        job->setLimit(1u);
        job->setBeginTimestamp(QDateTime(QDate(2008, 06, 06), QTime(22, 27, 45, 0)));
        job->setEndTimestamp(QDateTime(QDate(2007, 06, 06), QTime(22, 27, 45, 0)));
        job->setWidthScale(78u);
        job->setHeightScale(102u);

        connect(job, SIGNAL(result(QList<Imageinfo>)),
                this, SLOT(resultHandle(QList<Imageinfo>)));

        job->exec();

        // Test job
        QCOMPARE(job->error(), int(KJob::NoError));

        // Test fakeserver
        QVERIFY(fakeserver.isAllScenarioDone());

        // Test requests sent
        const QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), 2);

        for (unsigned int i = 0 ; i < 2 ; ++i)
        {
            QCOMPARE(requests[i].agent, MediaWiki.userAgent());
            QCOMPARE(requests[i].type, QStringLiteral("GET"));
        }

        QCOMPARE(requests[0].value, QStringLiteral("/?format=xml&action=query&titles=Image:Image.bmp&prop=imageinfo&iiprop=timestamp%7Cuser%7Ccomment%7Curl%7Csize%7Csha1%7Cmime%7Cmetadata&iilimit=1&iistart=2008-06-06T22:27:45Z&iiend=2007-06-06T22:27:45Z&iiurlwidth=78&iiurlheight=102"));
        QCOMPARE(requests[1].value, QStringLiteral("/?format=xml&action=query&titles=Image:Image.bmp&prop=imageinfo&iiprop=timestamp%7Cuser%7Ccomment%7Curl%7Csize%7Csha1%7Cmime%7Cmetadata&iilimit=1&iistart=2007-06-06T22:27:45Z&iiend=2007-06-06T22:27:45Z&iiurlwidth=78&iiurlheight=102"));

        // Test pages received
        QList<QList<Imageinfo> > imageinfosExpected;
        {
            QHash<QString, QVariant> metadata;
            metadata[QStringLiteral("Name1")] = QStringLiteral("Value1");
            metadata[QStringLiteral("Name2")] = QStringLiteral("Value2");

            {
                Imageinfo imageinfoExpected;
                imageinfoExpected.setTimestamp(QDateTime(QDate(2008, 06, 06), QTime(22, 27, 45, 0)));
                imageinfoExpected.setUser(QStringLiteral("User1"));
                imageinfoExpected.setComment(QStringLiteral("Comment1"));
                imageinfoExpected.setUrl(QUrl(QStringLiteral("http://url/File:Image.bmp")));
                imageinfoExpected.setDescriptionUrl(QUrl(QStringLiteral("http://descriptionurl/File:Image.bmp")));
                imageinfoExpected.setThumbUrl(QUrl(QStringLiteral("http://thumburl/File:Image.bmp")));
                imageinfoExpected.setThumbWidth(78);
                imageinfoExpected.setThumbHeight(102);
                imageinfoExpected.setSize(448798);
                imageinfoExpected.setWidth(924);
                imageinfoExpected.setHeight(1203);
                imageinfoExpected.setSha1(QStringLiteral("00be23585fde01190a0f8c60fc4267ea00f3745d"));
                imageinfoExpected.setMime(QStringLiteral("image/bmp"));
                imageinfoExpected.setMetadata(metadata);
                imageinfosExpected.push_back(QList<Imageinfo>() << imageinfoExpected);
            }

            {
                Imageinfo imageinfoExpected;
                imageinfoExpected.setTimestamp(QDateTime(QDate(2007, 06, 06), QTime(22, 27, 45, 0)));
                imageinfoExpected.setUser(QStringLiteral("User2"));
                imageinfoExpected.setComment(QStringLiteral("Comment2"));
                imageinfoExpected.setUrl(QUrl(QStringLiteral("http://url/File:Image.bmp")));
                imageinfoExpected.setDescriptionUrl(QUrl(QStringLiteral("http://descriptionurl/File:Image.bmp")));
                imageinfoExpected.setSize(448798);
                imageinfoExpected.setWidth(924);
                imageinfoExpected.setHeight(1203);
                imageinfoExpected.setSha1(QStringLiteral("00be23585fde01190a0f8c60fc4267ea00f3745d"));
                imageinfoExpected.setMime(QStringLiteral("image/bmp"));
                imageinfoExpected.setMetadata(metadata);
                imageinfosExpected.push_back(QList<Imageinfo>() << imageinfoExpected);
            }
        }

        QCOMPARE(imageinfosReceived, imageinfosExpected);
    }

    void testMissingTitle()
    {
        // Constructs the fakeserver
        FakeServer fakeserver;
        fakeserver.startAndWait();

        // Prepare the job
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));
        QueryImageinfo* const job = new QueryImageinfo(MediaWiki);

        job->setProperties(
            QueryImageinfo::Timestamp |
            QueryImageinfo::User      |
            QueryImageinfo::Comment   |
            QueryImageinfo::Url       |
            QueryImageinfo::Size      |
            QueryImageinfo::Sha1      |
            QueryImageinfo::Mime      |
            QueryImageinfo::Metadata);

        job->setLimit(1u);
        job->setBeginTimestamp(QDateTime(QDate(2008, 06, 06), QTime(22, 27, 45, 0)));
        job->setEndTimestamp(QDateTime(QDate(2007, 06, 06), QTime(22, 27, 45, 0)));
        job->setWidthScale(78u);
        job->setHeightScale(102u);

        connect(job, SIGNAL(result(QList<Imageinfo>)),
                this, SLOT(missingTitleHandle(QList<Imageinfo>)));

        job->exec();

        // Test job
        QCOMPARE(job->error(), int(QueryImageinfo::MissingMandatoryParameter));

        // Test fakeserver
        QCOMPARE(requestWithMissingTitleHasBeenSent, false);
    }

private:

    QList<QList<Imageinfo> > imageinfosReceived;
    bool                     requestWithMissingTitleHasBeenSent;
};

QTEST_MAIN(QueryImageinfoTest)

#include "queryimageinfotest.moc"
