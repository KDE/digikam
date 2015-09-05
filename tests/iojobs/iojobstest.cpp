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

// KDE includes

#include "klocalizedstring.h"

// Local includes

#include "iojob.h"

using namespace Digikam;

const QString testFileName   = QLatin1String("test.png");
const QString filePath       = QFINDTESTDATA(testFileName);
const QString srcFolderName  = QLatin1String("src");
const QString dstFolderPath  = QLatin1String("dst");
const QString testFolderName = QLatin1String("test");
const QString testFilePath   = (QDir::currentPath() + QDir::separator() + srcFolderName + QDir::separator() + testFileName);
const QString testFolderPath = (QDir::currentPath() + QDir::separator() + srcFolderName + QDir::separator() + testFolderName + QDir::separator());
const QString destPath       = (QDir::currentPath() + QDir::separator() + dstFolderPath + QDir::separator());

void IOJobsTest::init()
{
    QFile f(filePath);

    QDir d;
    d.mkpath(QDir::currentPath() + QDir::separator() + srcFolderName + QDir::separator() + testFolderName);
    d.mkpath(QDir::currentPath() + QDir::separator() + dstFolderPath);

    QVERIFY2(f.copy(QDir::currentPath() + QDir::separator() + srcFolderName + QDir::separator() + testFileName),
             qPrintable(QLatin1String("Could not copy the test target to src folder")));

    QVERIFY2(f.copy(QDir::currentPath() + QDir::separator() + srcFolderName + QDir::separator() + testFolderName + QDir::separator() + testFileName),
             qPrintable(QLatin1String("Could not copy the test target to src folder")));
}

void IOJobsTest::cleanup()
{
    QDir src(QDir::currentPath() + QDir::separator() + srcFolderName);
    QDir dst(QDir::currentPath() + QDir::separator() + dstFolderPath);

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

    QVERIFY(srcFi.exists());
    QVERIFY(dstFi.exists());

    QUrl srcUrl = QUrl::fromLocalFile(srcFi.absoluteFilePath());
    QUrl dstUrl = QUrl::fromLocalFile(dstFi.absoluteFilePath());

    CopyJob* job = new CopyJob(srcUrl, dstUrl, isMove);

    QThreadPool::globalInstance()->start(job);
    QThreadPool::globalInstance()->waitForDone();

    delete job;

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
            << (destPath + testFolderName + QDir::separator() + testFileName);

    QTest::newRow(qPrintable(QLatin1String("Moving Folder")))
            << testFolderPath
            << destPath
            << true
            << true
            << (destPath + testFolderName + QDir::separator() + testFileName);
}

void IOJobsTest::permanentDel()
{
    QFETCH(QString, srcToDel);

    QFileInfo fi(srcToDel);

    QUrl fileUrl = QUrl::fromLocalFile(fi.absoluteFilePath());

    DeleteJob* job = new DeleteJob(fileUrl, false);

    QThreadPool::globalInstance()->start(job);
    QThreadPool::globalInstance()->waitForDone();

    delete job;

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

QTEST_MAIN(IOJobsTest)
