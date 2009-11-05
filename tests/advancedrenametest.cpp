/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the AdvancedRename utility
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "advancedrenametest.h"
#include "advancedrenametest.moc"

// KDE includes

#include <qtest_kde.h>

// Local includes

#include "defaultrenameparser.h"
#include "parser.h"

using namespace Digikam;

QTEST_KDEMAIN(AdvancedRenameWidgetTest, GUI)

void AdvancedRenameWidgetTest::testFileNameToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString filename("myfile001.jpg");

    QTest::newRow("[file]") << QString("[file]") << filename << QString("myfile001");
}

void AdvancedRenameWidgetTest::testFileNameToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testFileExtensionToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString filename("myfile001.jpg");
    QString filename2("myfile001.blub.jpg");

    QTest::newRow(filename.toAscii())  << QString("[ext]") << filename  << QString("jpg");
    QTest::newRow(filename2.toAscii()) << QString("[ext]") << filename2 << QString("jpg");
}

void AdvancedRenameWidgetTest::testFileExtensionToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testDirectoryNameToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString filename("/mnt/data/photos/2009/digikam_tests/myfile001.jpg");

    QTest::newRow("[dir]")        << QString("[dir]")        << filename << QString("digikam_tests");
    QTest::newRow("[dir.]")       << QString("[dir.]")       << filename << QString("2009");
    QTest::newRow("[dir.]_[dir]") << QString("[dir.]_[dir]") << filename << QString("2009_digikam_tests");
    QTest::newRow("[dir.....]")   << QString("[dir.....]")   << filename << QString();
}

void AdvancedRenameWidgetTest::testDirectoryNameToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testNumberToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QTest::newRow("#")                      << QString("#")                      << 1  << QString("1");
    QTest::newRow("# (index:20)")           << QString("#")                      << 20 << QString("20");
    QTest::newRow("##")                     << QString("##")                     << 2  << QString("02");
    QTest::newRow("###")                    << QString("###")                    << 4  << QString("004");
    QTest::newRow("### (index:40)")         << QString("###")                    << 40 << QString("040");
    QTest::newRow("###_bla_##")             << QString("###_bla_##")             << 10 << QString("010_bla_10");
    QTest::newRow("####[2,3]")              << QString("####[2,3]")              << 1  << QString("0002");
    QTest::newRow("####[2,3](10)")          << QString("####[2,3]")              << 10 << QString("0029");
    QTest::newRow("####[ 2, 3]")            << QString("####[ 2, 3]")            << 10 << QString("0029");
    QTest::newRow("####[2,3]_bla_## ###")   << QString("####[2,3]_bla_## ###")   << 1  << QString("0002_bla_01 001");
    QTest::newRow("####[2, 3]_bla_## ###")  << QString("####[2, 3]_bla_## ###")  << 1  << QString("0002_bla_01 001");
    QTest::newRow("####[ 2, 3]_bla_## ###") << QString("####[ 2, 3]_bla_## ###") << 1  << QString("0002_bla_01 001");
    QTest::newRow("###[100]_bla")           << QString("###[100]_bla")           << 1  << QString("100_bla");
    QTest::newRow("###[100,]_bla")          << QString("###[100,]_bla")          << 1  << QString("100_bla");
    QTest::newRow("###[100,   ]_bla")       << QString("###[100,   ]_bla")       << 1  << QString("100_bla");
    QTest::newRow("###[100   ,   ]_bla")    << QString("###[100   ,   ]_bla")    << 1  << QString("100_bla");
}

void AdvancedRenameWidgetTest::testNumberToken()
{
    QFETCH(QString,   parseString);
    QFETCH(int,       index);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.index = index;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testTrimmedModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001");

    QTest::newRow("01") << QString("[file]!") << QString("myfilename001 ")  << fileName;
    QTest::newRow("02") << QString("[file]!") << QString(" myfilename001 ") << fileName;

    QTest::newRow("03") << QString("[file]!") << QString("       myfilename001      ")    << fileName;
    QTest::newRow("04") << QString("[file]!") << QString("        myfilename    001    ") << QString("myfilename 001");
}

void AdvancedRenameWidgetTest::testTrimmedModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testFirstLetterOfEachWordUppercaseModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("[file]*")      << QString("[file]*")  << fileName                << QString("Myfilename001");
    QTest::newRow("[file]**")     << QString("[file]**") << fileName                << QString("Myfilename001");
    QTest::newRow("my image.jpg") << QString("[file]*")  << QString("my image.jpg") << QString("My Image");
    QTest::newRow("my_image.jpg") << QString("[file]*")  << QString("my_image.jpg") << QString("My_Image");

    QTest::newRow("[file]*(token in filename)") << QString("[file]*") << QString("myfilename*001.jpg")
                                                << QString("Myfilename*001");
}

void AdvancedRenameWidgetTest::testFirstLetterOfEachWordUppercaseModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testChainedModifiers_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("[file]*&")    << QString("[file]*&")    << fileName << QString("MYFILENAME001");
    QTest::newRow("[file]{3-}*") << QString("[file]{3-}*") << fileName << QString("Filename001");

    QTest::newRow("[file]{3-}{\"name\",\"age\"}*") << QString("[file]{3-}{\"name\",\"age\"}*")
                                                   << fileName << QString("Fileage001");
}

void AdvancedRenameWidgetTest::testChainedModifiers()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testUppercaseModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("myfilename001.jpg")  << QString("[file]&") << fileName << QString("MYFILENAME001");
    QTest::newRow("my/filename001.jpg") << QString("[file]&") << QString("my/filename001.jpg")
                                        << QString("FILENAME001");

    QTest::newRow("myfilename001.jpg(token in filename)") << QString("[file]&") << QString("myfilename0&01.jpg")
                                                          << QString("MYFILENAME0&01");

    QTest::newRow("my image.jpg") << QString("[file]&") << QString("my image.jpg")
                                  << QString("MY IMAGE");

    QTest::newRow("my_image.jpg") << QString("[file]&") << QString("my_image.jpg")
                                  << QString("MY_IMAGE");
}

void AdvancedRenameWidgetTest::testUppercaseModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testRangeModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("DSC1234.jpg");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("[file]{1}")  << QString("[file]{1}") << fileName
                                << QString("D");

    QTest::newRow("[file]{3 }")  << QString("[file]{3 }") << fileName
                                 << QString("C");

    QTest::newRow("[file]{ 3 }")  << QString("[file]{ 3 }") << fileName
                                  << QString("C");

    QTest::newRow("[file]{ 3}")  << QString("[file]{ 3}") << fileName
                                 << QString("C");

    QTest::newRow("[file]{1-3}")  << QString("[file]{1-3}") << fileName
                                  << QString("DSC");

    QTest::newRow("[file]{ 1 - 3 }")  << QString("[file]{ 1 - 3 }") << fileName
                                      << QString("DSC");

    QTest::newRow("[file]{3-}")  << QString("[file]{3-}") << fileName
                                 << QString("C1234");
}

void AdvancedRenameWidgetTest::testRangeModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath = filename;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testLowercaseModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("myfilename001.jpg") << QString("[file]%") << fileName << QString("myfilename001");

    QTest::newRow("myfilename001.jpg(token in filename)") << QString("[file]%") << QString("mYfilenAme0%01.jpg")
                                                          << QString("myfilename0%01");

    QTest::newRow("my image.jpg") << QString("[file]%") << QString("MY image.jpg")
                                  << QString("my image");

    QTest::newRow("my_image.jpg") << QString("[file]%") << QString("mY_Image.jpg")
                                  << QString("my_image");
}

void AdvancedRenameWidgetTest::testLowercaseModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath = filename;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testCameraToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString filename("myfile001.jpg");
    QString camname("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("empty")    << QString("[cam]") << filename << QString() << curdate << 1
                              << QString();

    QTest::newRow("   ")      << QString("[cam]") << filename << QString("   ") << curdate << 1
                              << QString();

    QTest::newRow("[cam]")    << QString("[cam]") << filename << camname << curdate << 1
                              << camname;

    QTest::newRow("[Cam]")    << QString("[Cam]") << filename << camname << curdate << 1
                              << camname;

    QTest::newRow("[CAM]")    << QString("[CAM]") << filename << camname << curdate << 1
                              << camname;

    QTest::newRow("[ cam ]")  << QString("[ cam ]") << filename << camname << curdate << 1
                              << QString("[ cam ]");

    QTest::newRow("[camcam]") << QString("[camcam]") << filename << camname << curdate << 1
                              << QString("[camcam]");
}

void AdvancedRenameWidgetTest::testCameraToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   cameraName);
    QFETCH(QDateTime, cameraDate);
    QFETCH(int,       index);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;
    info.cameraName = cameraName;
    info.dateTime   = cameraDate;
    info.index      = index;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testEmptyParseString()
{
    QString filename("myfilename001.jpg");
    QDateTime curdate = QDateTime::currentDateTime();

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;
    info.cameraName = QString();
    info.index      = 1;
    info.dateTime   = curdate;

    // test for empty parser string
    QString parsed = parser.parse(QString(), info);
    QCOMPARE(parsed, QString("myfilename001"));

    parsed = parser.parse(QString(""), info);
    QCOMPARE(parsed, QString("myfilename001"));

    parsed = parser.parse(QString("   "), info);
    QCOMPARE(parsed, QString("myfilename001"));

    // the following is not invalid
    parsed = parser.parse(QString("  [file]%_##"), info);
    QCOMPARE(parsed, QString("  myfilename001_01"));
}
