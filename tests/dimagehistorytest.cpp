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

#include "dimagehistorytest.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>

// KDE includes

#include <qtest_kde.h>

// Local includes

#include "config-digikam.h"
#include "libs/dimg/filters/bcg/bcgfilter.h"
#include "libs/dimg/filters/curves/curvesfilter.h"
#include "dimagehistory.h"
#include "dimginterface.h"
#include "editortooliface.h"
#include "editorwindow.h"
#include "iccsettings.h"
#include "imageiface.h"
#include "iofilesettingscontainer.h"

using namespace Digikam;

QTEST_MAIN(DImageHistoryTest)

const QString IMAGE_PATH(KDESRCDIR+QString("albummodeltestimages"));

QDebug operator<<(QDebug dbg, const HistoryImageId& id)
{
    dbg.nospace() << " { ";
    dbg.nospace() << id.m_uuid;
    dbg.space() << id.m_type;
    dbg.space() << id.m_fileName;
    dbg.space() << id.m_filePath;
    dbg.space() << id.m_creationDate;
    dbg.space() << id.m_uniqueHash;
    dbg.space() << id.m_fileSize;
    dbg.space() << id.m_originalUUID;
    dbg.nospace() << " } ";
    return dbg;
}

void DImageHistoryTest::initTestCase()
{
    ICCSettingsContainer c = IccSettings::instance()->settings();
    c.enableCM = false;
    IccSettings::instance()->setSettings(c);

    im = new DImgInterface();
    DImgInterface::setDefaultInterface(im);

    connect(im, SIGNAL(signalImageLoaded(const QString&,bool)),
            this, SLOT(slotImageLoaded(const QString&,bool)));
    connect(im, SIGNAL(signalImageSaved(const QString&,bool)),
            this, SLOT(slotImageSaved(const QString&,bool)));

    QString tempSuffix = "dimagehistorytest-" + QTime::currentTime().toString();
    m_tempFile = QDir::temp().absolutePath() + QString("/") + tempSuffix;
}

void DImageHistoryTest::cleanupTestCase()
{
    delete im;
    QFile file(m_tempFile);
    file.remove();
}

void DImageHistoryTest::testXml()
{
    DImageHistory history;
    HistoryImageId id1("abc123");
    id1.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id1.setFileName("file1.jpg");
    id1.setPathOnDisk("/home/user/file1.jpg");
    id1.setUniqueHash("cde567", 987654);

    history <<id1;

    FilterAction fa("digikam:BCGFilter", 1);
    fa.addParameter("contrast", 1);
    fa.addParameter("channel", 1);
    fa.addParameter("brightness", 1);
    fa.addParameter("gamma", 1.2);

    history <<fa;

    HistoryImageId id2("12345");
    id2.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id2.setFileName("file2.jpg");
    id2.setPathOnDisk("/home/user/file2.jpg");
    id2.setUniqueHash("abc654", 876549);

    history <<id2;

    FilterAction fa2("digikam:SomeComplexFilter", 1, FilterAction::ComplexFilter);

    history <<fa2;

    HistoryImageId id3("abcdef");
    id3.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id3.setFileName("file3.jpg");
    id3.setPathOnDisk("/home/user/file3.jpg");
    id3.setUniqueHash("c76543", 765489);

    HistoryImageId id4("aaabbb");
    id4.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id4.setFileName("file4.jpg");
    id4.setPathOnDisk("/home/user/file4.jpg");
    id4.setUniqueHash("c96542", 654987);

    history << id3 << id4;

    QString xml = history.toXml();

    DImageHistory history2 = DImageHistory::fromXml(xml);
    QString xml2 = history2.toXml();

    QCOMPARE(xml, xml2);

    // Does not need to work: Some fields in the ids like originalUUID are filled out even if not in XML
    //QCOMPARE(history2, historyWithoutCurrent);
}

void DImageHistoryTest::testDImg()
{
    QDir imageDir(IMAGE_PATH);
    imageDir.setNameFilters(QStringList("*.jpg"));
    QList<QFileInfo> imageFiles = imageDir.entryInfoList();

    IOFileSettingsContainer container;
    im->load(imageFiles.first().filePath(), &container);

    m_loop.exec();

    DImageHistory history = im->getImg()->getImageHistory();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Current);
    QVERIFY(history.entries().last().referredImages.isEmpty());

    im->saveAs(m_tempFile, &container, true);

    m_loop.exec();

    history = im->getImg()->getImageHistory();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Current);
    QCOMPARE(history.entries().last().referredImages.size(), 1);
    QCOMPARE(history.entries().last().referredImages.first().m_type, HistoryImageId::Intermediate);

    im->switchToLastSaved(m_tempFile);

    history = im->getImg()->getImageHistory();
    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Original);
    QCOMPARE(history.entries().last().referredImages.size(), 1);
    QCOMPARE(history.entries().last().referredImages.first().m_type, HistoryImageId::Current);
}

void DImageHistoryTest::slotImageLoaded(const QString&, bool success)
{
    QVERIFY(success);

    ImageIface iface(0, 0);

    BCGFilter filter1(iface.getOriginalImg(), this);
    filter1.startFilterDirectly();
    iface.putOriginalImage(i18n("Brightness / Contrast / Gamma"), filter1.filterAction(), filter1.getTargetImage().bits());

    CurvesFilter filter2(iface.getOriginalImg(), this);
    filter2.startFilterDirectly();
    iface.putOriginalImage("Should be ignored in this test", filter2.filterAction(), filter2.getTargetImage().bits());

    DImageHistory h = im->getImg()->getImageHistory();
    h.adjustReferredImages();
    for (int i=0;i<3;i++)
    {
        qDebug() << i << h.entries()[i].referredImages.size();
        if (h.entries()[i].referredImages.size())
            qDebug() << " " << i << h.entries()[i].referredImages.first().m_type;
    }

    m_loop.quit();
}

void DImageHistoryTest::slotImageSaved(const QString& fileName, bool success)
{
    QVERIFY(success);

    DImg img(fileName);
    DImageHistory history = img.getImageHistory();
    //qDebug() << history.toXml();

    QCOMPARE(history.size(), 3);
    QCOMPARE(history.entries().first().referredImages.size(), 1);
    QCOMPARE(history.entries().first().referredImages.first().m_type, HistoryImageId::Original);

    m_loop.quit();
}


