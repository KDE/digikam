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

// Qt includes

#include <QObject>
#include <QTest>

// Local includes

#include "fakeserver/fakeserver.h"
#include "mediawiki_iface.h"
#include "mediawiki_queryimages.h"
#include "mediawiki_image.h"

using MediaWiki::Iface;
using MediaWiki::QueryImages;
using MediaWiki::Image;

Q_DECLARE_METATYPE(QList<QString>)
Q_DECLARE_METATYPE(QList<Image>)
Q_DECLARE_METATYPE(QList<QList<Image> >)

class Q_DECL_HIDDEN QueryImagesTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:

    void imagesHandle(const QList<Image>& images)
    {
        imagesReceivedList.push_back(images);
    }

private Q_SLOTS:

    void init()
    {
        imagesReceivedList.clear();
    }

    void testConstructor()
    {
        QFETCH(QList<QString>, scenarios);
        QFETCH(QString, title);
        QFETCH(unsigned int, limit);
        QFETCH(QList<QList<Image> >, imagesExpectedList);

        // Constructs the fakeserver
        FakeServer fakeserver;

        for (int i = 0 ; i < scenarios.size() ; ++i)
        {
            if (i == 0)
            {
                fakeserver.setScenario(scenarios[i]);
            }
            else
            {
                fakeserver.addScenario(scenarios[i]);
            }
        }

        fakeserver.startAndWait();

        // Prepare the job
        Iface MediaWiki(QUrl(QStringLiteral("http://127.0.0.1:12566")));
        QueryImages* const job = new QueryImages(MediaWiki);
        job->setTitle(title);
        job->setLimit(limit);

        connect(job, SIGNAL(images(QList<Image>)),
                this, SLOT(imagesHandle(QList<Image>)));

        job->exec();

        // Test job
        QCOMPARE(job->error(), int(KJob::NoError));

        // Test requests sent
        const QList<FakeServer::Request> requests = fakeserver.getRequest();
        QCOMPARE(requests.size(), imagesExpectedList.size());

        for (int i = 0 ; i < requests.size() ; ++i)
        {
            QCOMPARE(requests[i].agent, MediaWiki.userAgent());
            QCOMPARE(requests[i].type, QStringLiteral("GET"));

            if (i == 0)
            {
                QCOMPARE(requests[i].value, QString(QStringLiteral("/?format=xml&action=query&titles=") + title + QStringLiteral("&prop=images&imlimit=") + QString::number(limit)));
            }
            else
            {
                QCOMPARE(requests[i].value, QString(QStringLiteral("/?format=xml&action=query&titles=") + title + QStringLiteral("&prop=images&imlimit=") + QString::number(limit) + QStringLiteral("&imcontinue=1234%7C") + imagesExpectedList[i][0].title().remove(0, 5)));
            }
        }

        // Test pages received
        QCOMPARE(imagesReceivedList, imagesExpectedList);

        // Test fakeserver
        QVERIFY(fakeserver.isAllScenarioDone());
    }

    void testConstructor_data()
    {
        QTest::addColumn<QList<QString> >("scenarios");
        QTest::addColumn<QString>("title");
        QTest::addColumn<unsigned int>("limit");
        QTest::addColumn<QList<QList<Image> > >("imagesExpectedList");
        Image image, image2, image3;

        QTest::newRow("Page with no image")
                << (QList<QString>()
                    << QStringLiteral("<?xml version=\"1.0\"?><api><query><pages><page pageid=\"736\" ns=\"1\" title=\"Title-1\"></page></pages></query></api>"))
                << QStringLiteral("Title-1")
                << 10u
                << (QList<QList<Image> >() << QList<Image>());

        image.setNamespaceId(46u);
        image.setTitle(QStringLiteral("File:Image-1-1"));
        QTest::newRow("Page with one image")
                << (QList<QString>()
                    << QStringLiteral("<?xml version=\"1.0\"?><api><query><pages><page pageid=\"736\" ns=\"1\" title=\"Title-1\"><images><im ns=\"46\" title=\"File:Image-1-1\" /></images></page></pages></query></api>"))
                << QStringLiteral("Title-1")
                << 10u
                << (QList<QList<Image> >() << (QList<Image>() << image));

        image2.setNamespaceId(9997u);
        image2.setTitle(QStringLiteral("File:Image-1-2"));
        QTest::newRow("Page with two images")
                << (QList<QString>()
                    << QStringLiteral("<?xml version=\"1.0\"?><api><query><pages><page pageid=\"736\" ns=\"1\" title=\"Title-1\"><images><im ns=\"46\" title=\"File:Image-1-1\" /><im ns=\"9997\" title=\"File:Image-1-2\" /></images></page></pages></query></api>"))
                << QStringLiteral("Title-1")
                << 10u
                << (QList<QList<Image> >() << (QList<Image>() << image << image2));

        image.setNamespaceId(8u);
        image.setTitle(QStringLiteral("File:Image-2-1"));

        image2.setNamespaceId(8998u);
        image2.setTitle(QStringLiteral("File:Image-2-2"));

        image3.setNamespaceId(38423283u);
        image3.setTitle(QStringLiteral("File:Image-2-3"));

        QTest::newRow("Page with three images by two signals")
                << (QList<QString>()
                    << QStringLiteral("<?xml version=\"1.0\"?><api><query><pages><page pageid=\"1234\" ns=\"5757\" title=\"Title-2\"><images><im ns=\"8\" title=\"File:Image-2-1\" /><im ns=\"8998\" title=\"File:Image-2-2\" /></images></page></pages></query><query-continue><images imcontinue=\"1234|Image-2-3\" /></query-continue></api>")
                    << QStringLiteral("<?xml version=\"1.0\"?><api><query><pages><page pageid=\"1234\" ns=\"5757\" title=\"Title-2\"><images><im ns=\"38423283\" title=\"File:Image-2-3\" /></images></page></pages></query></api>"))
                << QStringLiteral("Title-2")
                << 2u
                << (QList<QList<Image> >()
                    << (QList<Image>() << image << image2)
                    << (QList<Image>() << image3));


    }

private:

    QList<QList<Image> > imagesReceivedList;
};

QTEST_MAIN(QueryImagesTest)

#include "queryimagestest.moc"
