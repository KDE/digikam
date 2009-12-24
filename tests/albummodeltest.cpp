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

// Local includes

#include "albummanager.h"
#include "albummodel.h"
#include "config-digikam.h"
#include "modeltest.h"

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
    QString dbPath = QDir::temp().absolutePath() + QString("/") + tempSuffix;;
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
    bool dbChangeGood =
                    AlbumManager::instance()->setDatabase(
                    QDir::temp().absoluteFilePath(tempSuffix), false,
                    QDir::temp().absoluteFilePath(tempSuffix));
    QVERIFY2(dbChangeGood, "Could not set temp album db");

}

void AlbumModelTest::testPAlbumModel()
{
    AlbumModel *albumModel = new AlbumModel();
    ModelTest *test = new ModelTest(albumModel, 0);
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

void AlbumModelTest::cleanupTestCase()
{
    QDir::temp().rmdir(tempSuffix);
}
