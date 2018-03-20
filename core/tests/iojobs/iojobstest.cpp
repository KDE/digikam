/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-02
 * Description : Test the functions for dealing with DatabaseFields
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "iojobstest.h"

// Qt includes

#include <QApplication>
#include <QtTest>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

// Local includes

#include "iojob.h"

using namespace Digikam;

const QString testFileName   = QLatin1String("test.png");
const QString filePath       = QFINDTESTDATA(testFileName);
const QString srcFolderName  = QLatin1String("src");
const QString dstFolderPath  = QLatin1String("dst");
const QString testFolderName = QLatin1String("test");
const QString testFilePath   = (QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFileName);
const QString testFolderPath = (QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFolderName + QLatin1Char('/'));
const QString destPath       = (QDir::currentPath() + QLatin1Char('/') + dstFolderPath + QLatin1Char('/'));

void IOJobsTest::init()
{
    QFile f(filePath);

    QDir d;
    d.mkpath(QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFolderName);
    d.mkpath(QDir::currentPath() + QLatin1Char('/') + dstFolderPath);

    QVERIFY2(f.copy(QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFileName),
             qPrintable(QLatin1String("Could not copy the test target to src folder")));

    QVERIFY2(f.copy(QDir::currentPath() + QLatin1Char('/') + srcFolderName + QLatin1Char('/') + testFolderName + QLatin1Char('/') + testFileName),
             qPrintable(QLatin1String("Could not copy the test target to src folder")));
}

void IOJobsTest::cleanup()
{
    QDir src(QDir::currentPath() + QLatin1Char('/') + srcFolderName);
    QDir dst(QDir::currentPath() + QLatin1Char('/') + dstFolderPath);

    QVERIFY2(src.removeRecursively() && dst.removeRecursively(),
             qPrintable(QLatin1String("Could not cleanup")));
}

void IOJobsTest::copyAndMove()
{
    QFETCH(QString, src);
    QFETCH(QString, dst);
    QFETCH(bool, isMove);
    QFETCH(bool, shouldExistInDst);
    QFETCH(QString, pathToCheckInDst);

    QFileInfo srcFi(src);
    QFileInfo dstFi(dst);

    QUrl srcUrl           = QUrl::fromLocalFile(srcFi.absoluteFilePath());
    QUrl dstUrl           = QUrl::fromLocalFile(dstFi.absoluteFilePath());
    int operation         = (isMove ? IOJobData::MoveFiles : IOJobData::CopyFiles);

    IOJobData* const data = new IOJobData(operation, QList<QUrl>() << srcUrl, dstUrl);
    CopyJob* const job    = new CopyJob(data);

    QThreadPool::globalInstance()->start(job);
    QThreadPool::globalInstance()->waitForDone();

    delete job;
    delete data;

    QFileInfo dstFiAfterCopy(pathToCheckInDst);

    QCOMPARE(dstFiAfterCopy.exists(), shouldExistInDst);
    srcFi.refresh();
    QCOMPARE(srcFi.exists(), !isMove);
}

void IOJobsTest::copyAndMove_data()
{
    QTest::addColumn<QString>("src");
    QTest::addColumn<QString>("dst");
    QTest::addColumn<bool>("isMove");
    QTest::addColumn<bool>("shouldExistInDst");
    QTest::addColumn<QString>("pathToCheckInDst");

    QTest::newRow(qPrintable(QLatin1String("Copying file")))
            << testFilePath
            << destPath
            << false
            << true
            << (destPath + testFileName);

    QTest::newRow(qPrintable(QLatin1String("Moving file")))
            << testFilePath
            << destPath
            << true
            << true
            << (destPath + testFileName);

    QTest::newRow(qPrintable(QLatin1String("Copying Folder")))
            << testFolderPath
            << destPath
            << false
            << true
            << (destPath + testFolderName + QLatin1Char('/') + testFileName);

    QTest::newRow(qPrintable(QLatin1String("Moving Folder")))
            << testFolderPath
            << destPath
            << true
            << true
            << (destPath + testFolderName + QLatin1Char('/') + testFileName);
}

void IOJobsTest::permanentDel()
{
    QFETCH(QString, srcToDel);

    QFileInfo fi(srcToDel);

    QUrl fileUrl          = QUrl::fromLocalFile(fi.absoluteFilePath());

    IOJobData* const data = new IOJobData(IOJobData::DFiles, QList<QUrl>() << fileUrl);
    DeleteJob* job        = new DeleteJob(data);

    QThreadPool::globalInstance()->start(job);
    QThreadPool::globalInstance()->waitForDone();

    delete job;
    delete data;

    fi.refresh();
    QVERIFY(!fi.exists());
}

void IOJobsTest::permanentDel_data()
{
    QTest::addColumn<QString>("srcToDel");

    QTest::newRow(qPrintable(QLatin1String("Deleting File")))
            << testFilePath;

    QTest::newRow(qPrintable(QLatin1String("Deleting Folder")))
            << testFolderPath;
}

QTEST_GUILESS_MAIN(IOJobsTest)
