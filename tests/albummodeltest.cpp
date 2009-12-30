/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-11
 * Description : test cases for the various album models
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "albummodeltest.moc"

// Qt includes

#include <qdebug.h>
#include <qdir.h>
#include <qtest.h>

// KDE includes

#include <qtest_kde.h>
#include <kio/netaccess.h>

// Local includes

#include "albumdb.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumthumbnailloader.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "config-digikam.h"
#include "loadingcacheinterface.h"
#include "modeltest.h"
#include "scancontroller.h"
#include "thumbnailloadthread.h"

using namespace Digikam;

const QString IMAGE_PATH(KDESRCDIR"albummodeltestimages");

QTEST_KDEMAIN(AlbumModelTest, GUI)

AlbumModelTest::AlbumModelTest() :
    albumCategory("DummyCategory")
{
}

AlbumModelTest::~AlbumModelTest()
{
}

void AlbumModelTest::initTestCase()
{

    tempSuffix = "albummodeltest-" + QTime::currentTime().toString();
    dbPath = QDir::temp().absolutePath() + QString("/") + tempSuffix;
    ;
    if (QDir::temp().exists(tempSuffix))
    {
        QString msg = QString("Error creating temp path") + dbPath;
        QVERIFY2(false, msg.toAscii().data());
    }

    QDir::temp().mkdir(tempSuffix);

    qDebug() << "Using database path for test: " << dbPath;

    // use a testing database
    AlbumManager::instance();
    AlbumManager::checkDatabaseDirsAfterFirstRun(QDir::temp().absoluteFilePath(
                    tempSuffix), QDir::temp().absoluteFilePath(tempSuffix));
    bool dbChangeGood = AlbumManager::instance()->setDatabase(
                    QDir::temp().absoluteFilePath(tempSuffix), false,
                    QDir::temp().absoluteFilePath(tempSuffix));
    QVERIFY2(dbChangeGood, "Could not set temp album db");
    QList<CollectionLocation> locs =
                    CollectionManager::instance()->allAvailableLocations();
    QVERIFY2(locs.size(), "Failed to auto-create one collection in setDatabase");

    ScanController::instance()->completeCollectionScan();
    AlbumManager::instance()->startScan();

    QVERIFY2(AlbumManager::instance()->allPAlbums().size() == 2,
                    "Failed to scan empty directory and have one root and one album root album");
}

void AlbumModelTest::cleanupTestCase()
{

    ScanController::instance()->shutDown();
    AlbumManager::instance()->cleanUp();
    ThumbnailLoadThread::cleanUp();
    AlbumThumbnailLoader::instance()->cleanUp();
    LoadingCacheInterface::cleanUp();

    KUrl deleteUrl = KUrl::fromPath(dbPath);
    KIO::NetAccess::del(deleteUrl, 0);
    qDebug() << "deleted test folder " << deleteUrl;
}

// Qt test doesn't use exceptions, so using assertion macros in methods called
// from a test slot doesn't stop the test method and may result in inconsistent
// data or segfaults. Therefore use macros for these functions.

#define safeCreatePAlbum(parent, name, result) \
{ \
    QString error; \
    result = AlbumManager::instance()->createPAlbum(parent, name, name, \
                    QDate::currentDate(), albumCategory, error); \
    QVERIFY2(result, QString(QString("Error creating PAlbum for test: ") + error).toAscii()); \
}

#define safeCreateTAlbum(parent, name, result) \
{ \
    QString error; \
    result = AlbumManager::instance()->createTAlbum(parent, name, "", error); \
    QVERIFY2(result, QString(QString("Error creating TAlbum for test: ") + error).toAscii()); \
}

void AlbumModelTest::init()
{
    // insert some test data

    // physical albums

    // create two of them by creating directories and scanning
    QDir dir(dbPath);
    dir.mkdir("root0");
    dir.mkdir("root1");

    ScanController::instance()->completeCollectionScan();
    AlbumManager::instance()->refresh();

    QCOMPARE(AlbumManager::instance()->allPAlbums().size(), 4);

    QString error;
    palbumRoot0 = AlbumManager::instance()->findPAlbum(KUrl::fromPath(dbPath
                    + "/root0"));
    QVERIFY2(palbumRoot0, "Error having PAlbum root0 in AlbumManager");
    palbumRoot1 = AlbumManager::instance()->findPAlbum(KUrl::fromPath(dbPath
                    + "/root1"));
    QVERIFY2(palbumRoot1, "Error having PAlbum root1 in AlbumManager");

    // Create some more through AlbumManager
    palbumRoot2 = AlbumManager::instance()->createPAlbum(dbPath, "root2",
                    "root album 2", QDate::currentDate(), albumCategory, error);
    QVERIFY2(palbumRoot2, QString(QString("Error creating PAlbum for test: ") + error).toAscii());

    safeCreatePAlbum(palbumRoot0, "root0child0", palbumChild0Root0);
    safeCreatePAlbum(palbumRoot0, "root0child1", palbumChild1Root0);
    const QString sameName = "sameName Album";
    safeCreatePAlbum(palbumRoot0, sameName, palbumChild2Root0);

    safeCreatePAlbum(palbumRoot1, sameName, palbumChild0Root1);

    qDebug() << "AlbumManager now knows these PAlbums: "
                    << AlbumManager::instance()->allPAlbums();

    // tags

    rootTag = AlbumManager::instance()->findTAlbum(0);
    QVERIFY(rootTag);

    safeCreateTAlbum(rootTag, "root0", talbumRoot0);
    safeCreateTAlbum(rootTag, "root1", talbumRoot1);

    safeCreateTAlbum(talbumRoot0, "child0 root 0", talbumChild0Root0);
    safeCreateTAlbum(talbumRoot0, "child1 root 0", talbumChild1Root0);

    safeCreateTAlbum(talbumChild1Root0, sameName, talbumChild0Child1Root0);

    safeCreateTAlbum(talbumRoot1, sameName, talbumChild0Root1);

    qDebug() << "created tags";

    // add some images for having date albums

    QDir imageDir(IMAGE_PATH);
    imageDir.setNameFilters(QStringList("*.jpg"));
    QStringList imageFiles = imageDir.entryList();

    qDebug() << "copying images " << imageFiles << " to "
                    << palbumChild0Root0->fileUrl();

    foreach (const QString &imageFile, imageFiles)
    {
        QString src = IMAGE_PATH + "/" + imageFile;
        QString dst = palbumChild0Root0->fileUrl().toLocalFile() + "/" + imageFile;
        bool copied = QFile::copy(src, dst);
        QVERIFY2(copied, "Test images must be copied");
    }

    ScanController::instance()->completeCollectionScan();

    if (AlbumManager::instance()->allDAlbums().count() <= 1)
    {
        qDebug() << "Waiting for AlbumManager and the IOSlave to create DAlbums - there is a timer waiting for 5 seconds.";
        QEventLoop loop;
        connect(AlbumManager::instance(), SIGNAL(signalAllDAlbumsLoaded()),
                &loop, SLOT(quit()));
        loop.exec();
    }

    qDebug() << "date albums: " << AlbumManager::instance()->allDAlbums();

}

void AlbumModelTest::deletePAlbum(PAlbum *album)
{
    KUrl u;
    u.setProtocol("file");
    u.setPath(album->folderPath());
    KIO::NetAccess::del(u, 0);
}

void AlbumModelTest::cleanup()
{
    // remove all test data

    AlbumManager::instance()->refresh();

    // remove all palbums' directories
    deletePAlbum(palbumRoot0);
    deletePAlbum(palbumRoot1);
    deletePAlbum(palbumRoot2);

    // take over changes to database
    ScanController::instance()->completeCollectionScan();

    // reread from database
    AlbumManager::instance()->refresh();

    // root + one collection
    QCOMPARE(AlbumManager::instance()->allPAlbums().size(), 2);

    // remove all tags

    QString error;
    bool removed = AlbumManager::instance()->deleteTAlbum(talbumRoot0, error);
    QVERIFY2(removed, QString("Error removing a tag: " + error).toAscii());
    removed = AlbumManager::instance()->deleteTAlbum(talbumRoot1, error);
    QVERIFY2(removed, QString("Error removing a tag: " + error).toAscii());

    QCOMPARE(AlbumManager::instance()->allTAlbums().size(), 1);

}

void AlbumModelTest::testPAlbumModel()
{

    AlbumModel *albumModel = new AlbumModel();
    ModelTest *test = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;

    albumModel = new AlbumModel(AbstractAlbumModel::IgnoreRootAlbum);
    test = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;

}

void AlbumModelTest::testDAlbumModel()
{
    DateAlbumModel *albumModel = new DateAlbumModel();
    ModelTest *test = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;
}

void AlbumModelTest::testTAlbumModel()
{

    TagModel *albumModel = new TagModel();
    ModelTest *test = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;

    albumModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    test = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;

}

void AlbumModelTest::testSAlbumModel()
{
    SearchModel *albumModel = new SearchModel();
    ModelTest *test = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;
}
