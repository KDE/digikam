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

QTEST_KDEMAIN(AlbumModelTest, GUI)

AlbumModelTest::AlbumModelTest()
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
    QList<CollectionLocation> locs = CollectionManager::instance()->allAvailableLocations();
    QVERIFY2(locs.size(), "Failed to auto-create one collection in setDatabase");
    int albumID = DatabaseAccess().db()->addAlbum(locs.first().id(), "/", QString(), QDate::currentDate(), QString());
    QVERIFY2(albumID > 0, "Failed to add album belonging to default collection");

    AlbumManager::instance()->startScan();

}

void AlbumModelTest::cleanupTestCase()
{

    ScanController::instance()->shutDown();
    AlbumManager::instance()->cleanUp();
    ThumbnailLoadThread::cleanUp();
    AlbumThumbnailLoader::instance()->cleanUp();
    LoadingCacheInterface::cleanUp();

    QDir::temp().rmdir(tempSuffix);
}

void AlbumModelTest::init()
{
    // insert some test data

    // physical albums

    QString error;
    QString category = "dummy category";
    PAlbum *palbumRoot0 = AlbumManager::instance()->createPAlbum(dbPath,
                    "root0", "root album 0", QDate::currentDate(), category,
                    error);
    QVERIFY2(palbumRoot0, QString(QString("Error creating PAlbum for test: ") + error).toAscii());
    PAlbum *palbumRoot1 = AlbumManager::instance()->createPAlbum(dbPath,
                    "root1", "root album 1", QDate::currentDate(), category,
                    error);
    QVERIFY2(palbumRoot1, QString(QString("Error creating PAlbum for test: ") + error).toAscii());
    PAlbum *palbumRoot2 = AlbumManager::instance()->createPAlbum(dbPath,
                    "root2", "root album 2", QDate::currentDate(), category,
                    error);
    QVERIFY2(palbumRoot2, QString(QString("Error creating PAlbum for test: ") + error).toAscii());

    PAlbum *palbumChild0Root0 = AlbumManager::instance()->createPAlbum(
                    palbumRoot0, "root0child0", "root 0 child 0",
                    QDate::currentDate(), category, error);
    QVERIFY2(palbumChild0Root0, QString(QString("Error creating PAlbum for test: ") + error).toAscii());
    PAlbum *palbumChild1Root0 = AlbumManager::instance()->createPAlbum(
                    palbumRoot0, "root0child1", "root 0 child 1",
                    QDate::currentDate(), category, error);
    QVERIFY2(palbumChild1Root0, QString(QString("Error creating PAlbum for test: ") + error).toAscii());
    QString sameName = "sameName";
    PAlbum *palbumChild2Root0 = AlbumManager::instance()->createPAlbum(
                    palbumRoot0, sameName, "root 0 child 2",
                    QDate::currentDate(), category, error);
    QVERIFY2(palbumChild2Root0, QString(QString("Error creating PAlbum for test: ") + error).toAscii());

    PAlbum *palbumChild0Root1 = AlbumManager::instance()->createPAlbum(
                    palbumRoot1, sameName, "root 1 child 0",
                    QDate::currentDate(), category, error);
    QVERIFY2(palbumChild0Root1, QString(QString("Error creating PAlbum for test: ") + error).toAscii());

    AlbumManager::instance()->refresh();

    qDebug() << "AlbumManager now knows these PAlbums: "
             << AlbumManager::instance()->allPAlbums();

}

void AlbumModelTest::cleanup()
{
    // remove all test data

    AlbumManager::instance()->refresh();

    qDebug() << "AlbumManager removes these PAlbums: "
             << AlbumManager::instance()->allPAlbums();


    // remove all palbums' directories
    foreach (Album* album, AlbumManager::instance()->allPAlbums())
    {
        PAlbum *palbum = dynamic_cast<PAlbum*> (album);
        if (palbum->isAlbumRoot())
        {
            continue;
        }
        qDebug() << "deleting directory: " << palbum->folderPath();
        KUrl u;
        u.setProtocol("file");
        u.setPath(palbum->folderPath());
        KIO::NetAccess::del(u, 0);
    }
    // take over changes to database
    ScanController::instance()->completeCollectionScan();

    // reread from database
    AlbumManager::instance()->refresh();

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
}

void AlbumModelTest::testSAlbumModel()
{
    SearchModel *albumModel = new SearchModel();
    ModelTest *test = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;
}
