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

// #include "advancedrenametest.h"
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

    QTest::newRow("[file]") << QString("[file]") << filename << QString("myfile001.jpg");
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

    QTest::newRow(filename.toAscii())  << QString("[ext]") << filename  << QString("jpg.jpg");
    QTest::newRow(filename2.toAscii()) << QString("[ext]") << filename2 << QString("jpg.jpg");
    QTest::newRow("[ext]_lala_####") << QString("[ext]_lala_####") << filename << QString("jpg_lala_0001.jpg");
    QTest::newRow("[ext]_lala_####[ext]") << QString("[ext]_lala_####[ext]") << filename << QString("jpg_lala_0001jpg.jpg");
    QTest::newRow("[ext]_lala_####.[ext]") << QString("[ext]_lala_####.[ext]") << filename << QString("jpg_lala_0001.jpg");
    QTest::newRow("[ext]_lala_####.[ext]&") << QString("[ext]_lala_####.[ext]&") << filename << QString("jpg_lala_0001.JPG");
    QTest::newRow("[ext]_lala_####[ext]&") << QString("[ext]_lala_####[ext]&") << filename << QString("jpg_lala_0001JPG.jpg");
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

    QTest::newRow("[dir]")        << QString("[dir]")        << filename << QString("digikam_tests.jpg");
    QTest::newRow("[dir.]")       << QString("[dir.]")       << filename << QString("2009.jpg");
    QTest::newRow("[dir.]_[dir]") << QString("[dir.]_[dir]") << filename << QString("2009_digikam_tests.jpg");
    QTest::newRow("[dir.....]")   << QString("[dir.....]")   << filename << QString("myfile001.jpg");
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

    // filename is "myimage_1234.jpg", it is not defined in here to avoid typing it into the results all the time.
    // it will be defined in the test method

    QTest::newRow("#")                      << QString("#")                      << 1  << QString("1.jpg");
    QTest::newRow("# (index:20)")           << QString("#")                      << 20 << QString("20.jpg");
    QTest::newRow("##")                     << QString("##")                     << 2  << QString("02.jpg");
    QTest::newRow("###")                    << QString("###")                    << 4  << QString("004.jpg");
    QTest::newRow("### (index:40)")         << QString("###")                    << 40 << QString("040.jpg");
    QTest::newRow("###_bla_##")             << QString("###_bla_##")             << 10 << QString("010_bla_10.jpg");
    QTest::newRow("####[2,3]")              << QString("####[2,3]")              << 1  << QString("0002.jpg");
    QTest::newRow("####[2,3](10)")          << QString("####[2,3]")              << 10 << QString("0029.jpg");
    QTest::newRow("####[ 2, 3]")            << QString("####[ 2, 3]")            << 10 << QString("0029.jpg");
    QTest::newRow("####[2,3]_bla_## ###")   << QString("####[2,3]_bla_## ###")   << 1  << QString("0002_bla_01 001.jpg");
    QTest::newRow("####[2, 3]_bla_## ###")  << QString("####[2, 3]_bla_## ###")  << 1  << QString("0002_bla_01 001.jpg");
    QTest::newRow("####[ 2, 3]_bla_## ###") << QString("####[ 2, 3]_bla_## ###") << 1  << QString("0002_bla_01 001.jpg");
    QTest::newRow("###[100]_bla")           << QString("###[100]_bla")           << 1  << QString("100_bla.jpg");
    QTest::newRow("###[100,]_bla")          << QString("###[100,]_bla")          << 1  << QString("100_bla.jpg");
    QTest::newRow("###[100,   ]_bla")       << QString("###[100,   ]_bla")       << 1  << QString("100_bla.jpg");
    QTest::newRow("###[100   ,   ]_bla")    << QString("###[100   ,   ]_bla")    << 1  << QString("100_bla.jpg");
}

void AdvancedRenameWidgetTest::testNumberToken()
{
    QFETCH(QString,   parseString);
    QFETCH(int,       index);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath = QString("myimage_1234.jpg");
    info.index = index;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testTrimmedModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("01") << QString("[file]!") << QString("myfilename001 .jpg")  << fileName;
    QTest::newRow("02") << QString("[file]!") << QString(" myfilename001 .jpg") << fileName;

    QTest::newRow("03") << QString("[file]!") << QString("       myfilename001      .jpg")    << fileName;
    QTest::newRow("04") << QString("[file]!") << QString("        myfilename    001    .jpg") << QString("myfilename 001.jpg");
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

    QTest::newRow("[file]*")      << QString("[file]*")  << fileName                << QString("Myfilename001.jpg");
    QTest::newRow("[file]**")     << QString("[file]**") << fileName                << QString("Myfilename001.jpg");
    QTest::newRow("my image.jpg") << QString("[file]*")  << QString("my image.jpg") << QString("My Image.jpg");
    QTest::newRow("my_image.jpg") << QString("[file]*")  << QString("my_image.jpg") << QString("My_Image.jpg");

    QTest::newRow("[file]*(token in filename)") << QString("[file]*") << QString("myfilename*001.jpg")
                                                << QString("Myfilename*001.jpg");
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

    QTest::newRow("[file]*&")    << QString("[file]*&")    << fileName << QString("MYFILENAME001.jpg");
    QTest::newRow("[file]{3-}*") << QString("[file]{3-}*") << fileName << QString("Filename001.jpg");

    QTest::newRow("[file]{3-}{\"name\",\"age\"}*") << QString("[file]{3-}{\"name\",\"age\"}*")
                                                   << fileName << QString("Fileage001.jpg");
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

    QTest::newRow("myfilename001.jpg")  << QString("[file]&") << fileName << QString("MYFILENAME001.jpg");
    QTest::newRow("my/filename001.jpg") << QString("[file]&") << QString("my/filename001.jpg")
                                        << QString("FILENAME001.jpg");

    QTest::newRow("myfilename001.jpg(token in filename)") << QString("[file]&") << QString("myfilename0&01.jpg")
                                                          << QString("MYFILENAME0&01.jpg");

    QTest::newRow("my image.jpg") << QString("[file]&") << QString("my image.jpg")
                                  << QString("MY IMAGE.jpg");

    QTest::newRow("my_image.jpg") << QString("[file]&") << QString("my_image.jpg")
                                  << QString("MY_IMAGE.jpg");
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
                                << QString("D.jpg");

    QTest::newRow("[file]{3 }")  << QString("[file]{3 }") << fileName
                                 << QString("C.jpg");

    QTest::newRow("[file]{ 3 }")  << QString("[file]{ 3 }") << fileName
                                  << QString("C.jpg");

    QTest::newRow("[file]{ 3}")  << QString("[file]{ 3}") << fileName
                                 << QString("C.jpg");

    QTest::newRow("[file]{1-3}")  << QString("[file]{1-3}") << fileName
                                  << QString("DSC.jpg");

    QTest::newRow("[file]{ 1 - 3 }")  << QString("[file]{ 1 - 3 }") << fileName
                                      << QString("DSC.jpg");

    QTest::newRow("[file]{3-}")  << QString("[file]{3-}") << fileName
                                 << QString("C1234.jpg");
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

void AdvancedRenameWidgetTest::testDefaultValueModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("camera");
    QTest::addColumn<QString>("result");

    QString fileName("DSC1234.jpg");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("[cam]_[file]") << QString("[cam]{\"Unknown\"}_[file]") << fileName << QString("Canon Powershot A80")
                                  << QString("Canon Powershot A80_DSC1234.jpg");

    QTest::newRow("[cam]{\"Unknown\"}_[file]") << QString("[cam]{\"Unknown\"}_[file]") << fileName << QString()
                                               << QString("Unknown_DSC1234.jpg");
}

void AdvancedRenameWidgetTest::testDefaultValueModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   camera);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseInformation info;
    info.filePath   = filename;
    info.cameraName = camera;

    QString parsed = parser.parse(parseString, info);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testLowercaseModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("myfilename001.jpg") << QString("[file]%") << fileName << QString("myfilename001.jpg");

    QTest::newRow("myfilename001.jpg(token in filename)") << QString("[file]%") << QString("mYfilenAme0%01.jpg")
                                                          << QString("myfilename0%01.jpg");

    QTest::newRow("my image.jpg") << QString("[file]%") << QString("MY image.jpg")
                                  << QString("my image.jpg");

    QTest::newRow("my_image.jpg") << QString("[file]%") << QString("mY_Image.jpg")
                                  << QString("my_image.jpg");
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
    QString camname_ext("Nikon D50.jpg");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("   ")      << QString("[cam]") << filename << QString("   ") << curdate << 1
                              << QString("myfile001.jpg");

    QTest::newRow("[cam]")    << QString("[cam]") << filename << camname << curdate << 1
                              << camname_ext;

    QTest::newRow("[Cam]")    << QString("[Cam]") << filename << camname << curdate << 1
                              << camname_ext;

    QTest::newRow("[CAM]")    << QString("[CAM]") << filename << camname << curdate << 1
                              << camname_ext;

    QTest::newRow("[ cam ]")  << QString("[ cam ]") << filename << camname << curdate << 1
                              << QString("[ cam ].jpg");

    QTest::newRow("[camcam]") << QString("[camcam]") << filename << camname << curdate << 1
                              << QString("[camcam].jpg");
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
    QCOMPARE(parsed, QString("myfilename001.jpg"));

    parsed = parser.parse(QString(""), info);
    QCOMPARE(parsed, QString("myfilename001.jpg"));

    parsed = parser.parse(QString("   "), info);
    QCOMPARE(parsed, QString("myfilename001.jpg"));

    // the following is not invalid
    parsed = parser.parse(QString("  [file]%_##"), info);
    QCOMPARE(parsed, QString("  myfilename001_01.jpg"));
}
