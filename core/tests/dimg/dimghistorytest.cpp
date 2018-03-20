/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-01
 * Description : a test for the DImageHistory
 *
 * Copyright (C) 2010 by Marcel Wiesweg <user dot wiesweg at gmx dot de>
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

#include "dimghistorytest.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QTest>
#include <QDebug>

// Local includes

#include "digikam_config.h"
#include "editortooliface.h"
#include "editorwindow.h"
#include "imageiface.h"
#include "iofilesettings.h"

using namespace Digikam;

QTEST_MAIN(DImgHistoryTest)

void DImgHistoryTest::initTestCase()
{
    initBaseTestCase();
}

void DImgHistoryTest::cleanupTestCase()
{
    cleanupBaseTestCase();
}

void DImgHistoryTest::testXml()
{
    DImageHistory history = history1();

    QString xml = history.toXml();

    DImageHistory history2 = DImageHistory::fromXml(xml);
    QString xml2 = history2.toXml();

    QCOMPARE(xml, xml2);

    // Does not need to work: Some fields in the ids like originalUUID are filled out even if not in XML
    //QCOMPARE(history2, historyWithoutCurrent);
}

void DImgHistoryTest::testDImg()
{
    QDir imageDir(imagePath());
    imageDir.setNameFilters(QStringList() << QLatin1String("*.jpg"));
    QList<QFileInfo> imageFiles = imageDir.entryInfoList();

    IOFileSettings container;
    m_im->load(imageFiles.first().filePath(), &container);

    m_loop.exec();

    DImageHistory history = m_im->getImg()->getImageHistory();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Current);
    QVERIFY(history.entries().last().referredImages.isEmpty());

    m_im->saveAs(m_tempFile, &container, true, QString(), QString());

    m_loop.exec();

    history = m_im->getImg()->getImageHistory();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Current);
    QCOMPARE(history.entries().last().referredImages.size(), 1);
    QCOMPARE(history.entries().last().referredImages.first().m_type, HistoryImageId::Intermediate);

    m_im->switchToLastSaved();

    history = m_im->getImg()->getOriginalImageHistory();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Original);
    QCOMPARE(history.entries().last().referredImages.size(), 1);
    QCOMPARE(history.entries().last().referredImages.first().m_type, HistoryImageId::Current);
}

void DImgHistoryTest::slotImageLoaded(const QString&, bool success)
{
    QVERIFY(success);

    applyFilters1();

    DImageHistory h = m_im->getImg()->getImageHistory();
    h.adjustReferredImages();

    for (int i=0; i<3; ++i)
    {
        qDebug() << i << h.entries().at(i).referredImages.size();

        if (h.entries().at(i).referredImages.size())
        {
            qDebug() << " " << i << h.entries().at(i).referredImages.first().m_type;
        }
    }

    m_loop.quit();
}

void DImgHistoryTest::slotImageSaved(const QString& fileName, bool success)
{
    QVERIFY(success);

    m_im->setLastSaved(fileName);

    DImg img(fileName);
    DImageHistory history = img.getImageHistory();
    qDebug() << history.toXml();

    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Original);
    QCOMPARE(history.action(1).category(), FilterAction::ReproducibleFilter);
    QCOMPARE(history.action(2).category(), FilterAction::ReproducibleFilter);

    DImageHistory history2 = DImageHistory::fromXml(history.toXml());
    QCOMPARE(history2.size(), 3);
    QCOMPARE(history2.entries().first().referredImages.size(), 1);
    QCOMPARE(history2.entries().first().referredImages.first().m_type, HistoryImageId::Original);
    QCOMPARE(history2.action(1).category(), FilterAction::ReproducibleFilter);
    QCOMPARE(history2.action(2).category(), FilterAction::ReproducibleFilter);

    m_loop.quit();
}
