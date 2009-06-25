/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the ManualRenameInput widget
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

#include "manualrenameinputtest.h"
#include "manualrenameinputtest.moc"

// KDE includes

#include <qtest_kde.h>

// Local includes

#include "manualrenameinput.h"

using namespace Digikam;

QTEST_KDEMAIN(ManualRenameInputTest, GUI)

void ManualRenameInputTest::testNumberToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString filename("myfile001.jpg");
    QString cameraName("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("#") << QString("#") << filename << cameraName << curdate << 1
                       << QString("1");

    QTest::newRow("# (index:20)") << QString("#") << filename << cameraName << curdate << 20
                                  << QString("20");

    QTest::newRow("##") << QString("##") << filename << cameraName << curdate << 2
                        << QString("02");

    QTest::newRow("###") << QString("###") << filename << cameraName << curdate << 4
                         << QString("004");

    QTest::newRow("### (index:40)") << QString("###") << filename << cameraName << curdate << 40
                                    << QString("040");

    QTest::newRow("###_bla_##") << QString("###_bla_##") << filename << cameraName << curdate << 10
                                << QString("010_bla_10");

    QTest::newRow("####{2,3}") << QString("####{2,3}") << filename << cameraName << curdate << 1
                               << QString("0002");

    QTest::newRow("####{2,3}(10)") << QString("####{2,3}") << filename << cameraName << curdate << 10
                                   << QString("0029");

    QTest::newRow("####{ 2, 3}") << QString("####{ 2, 3}") << filename << cameraName << curdate << 10
                                 << QString("0029");

    QTest::newRow("####{2,3}_bla_## ###") << QString("####{2,3}_bla_## ###") << filename << cameraName << curdate << 1
                                          << QString("0002_bla_01 001");

    QTest::newRow("####{2, 3}_bla_## ###") << QString("####{2, 3}_bla_## ###") << filename << cameraName << curdate << 1
                                           << QString("0002_bla_01 001");

    QTest::newRow("####{ 2, 3}_bla_## ###") << QString("####{ 2, 3}_bla_## ###") << filename << cameraName << curdate << 1
                                            << QString("0002_bla_01 001");
}

void ManualRenameInputTest::testNumberToken()
{
    QFETCH(QString,     parseString);
    QFETCH(QString,     filename);
    QFETCH(QString,     cameraName);
    QFETCH(QDateTime,   cameraDate);
    QFETCH(int,         index);
    QFETCH(QString,     result);

    QString parsed = ManualRenameInput::parser(parseString, filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);

    // test again with bound parse method
    ManualRenameInput mrename;
    mrename.setText(parseString);
    parsed = mrename.parse(filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);
}

void ManualRenameInputTest::testFirstLetterOfEachWordUppercaseToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString cameraName("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("myfilename001.jpg") << QString("*") << QString("myfilename001.jpg")
                                       << cameraName << curdate << 1
                                       << QString("Myfilename001");

    QTest::newRow("myfilename001.jpg(token: **)") << QString("**") << QString("myfilename001.jpg")
                                                  << cameraName << curdate << 1
                                                  << QString("Myfilename001Myfilename001");

    QTest::newRow("myfilename001.jpg(token: * *)") << QString("* *") << QString("myfilename001.jpg")
                                                   << cameraName << curdate << 1
                                                   << QString("Myfilename001 Myfilename001");

    QTest::newRow("my image.jpg") << QString("*") << QString("my image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("My Image");

    QTest::newRow("my_image.jpg") << QString("*") << QString("my_image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("My_Image");
}

void ManualRenameInputTest::testFirstLetterOfEachWordUppercaseToken()
{
    QFETCH(QString,     parseString);
    QFETCH(QString,     filename);
    QFETCH(QString,     cameraName);
    QFETCH(QDateTime,   cameraDate);
    QFETCH(int,         index);
    QFETCH(QString,     result);

    QString parsed = ManualRenameInput::parser(parseString, filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);

    // test again with bound parse method
    ManualRenameInput mrename;
    mrename.setText(parseString);
    parsed = mrename.parse(filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);
}

void ManualRenameInputTest::testUppercaseToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString cameraName("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("myfilename001.jpg") << QString("&") << QString("myfilename001.jpg")
                                       << cameraName << curdate << 1
                                       << QString("MYFILENAME001");

    QTest::newRow("my/filename001.jpg") << QString("&") << QString("my/filename001.jpg")
                                       << cameraName << curdate << 1
                                       << QString("FILENAME001");

    QTest::newRow("myfilename001.jpg(token: &&)") << QString("&&") << QString("myfilename001.jpg")
                                                  << cameraName << curdate << 1
                                                  << QString("MYFILENAME001MYFILENAME001");

    QTest::newRow("myfilename001.jpg(token: & &)") << QString("& &") << QString("myfilename001.jpg")
                                                   << cameraName << curdate << 1
                                                   << QString("MYFILENAME001 MYFILENAME001");

    QTest::newRow("my image.jpg") << QString("&") << QString("my image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("MY IMAGE");

    QTest::newRow("my_image.jpg") << QString("&") << QString("my_image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("MY_IMAGE");

    QTest::newRow("all chars") << QString("&") << QString("1234567890'qwertzuiop+asdfghjkl#<yxcvbnm,.-.png")
                               << cameraName << curdate << 1
                               << QString("1234567890'QWERTZUIOP+ASDFGHJKL#<YXCVBNM,");
}

void ManualRenameInputTest::testUppercaseToken()
{
    QFETCH(QString,     parseString);
    QFETCH(QString,     filename);
    QFETCH(QString,     cameraName);
    QFETCH(QDateTime,   cameraDate);
    QFETCH(int,         index);
    QFETCH(QString,     result);

    QString parsed = ManualRenameInput::parser(parseString, filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);

    // test again with bound parse method
    ManualRenameInput mrename;
    mrename.setText(parseString);
    parsed = mrename.parse(filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);
}

void ManualRenameInputTest::testLowercaseToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString cameraName("Nikon D50");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("myfilename001.jpg") << QString("%") << QString("MyFileName001.jpg")
                                       << cameraName << curdate << 1
                                       << QString("myfilename001");

    QTest::newRow("myfilename001.jpg(token: %%)") << QString("%%") << QString("mYfilenAme001.jpg")
                                                  << cameraName << curdate << 1
                                                  << QString("myfilename001myfilename001");

    QTest::newRow("myfilename001.jpg(token: % %)") << QString("% %") << QString("myFILEname001.jpg")
                                                   << cameraName << curdate << 1
                                                   << QString("myfilename001 myfilename001");

    QTest::newRow("my image.jpg") << QString("%") << QString("MY image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("my image");

    QTest::newRow("my_image.jpg") << QString("%") << QString("mY_Image.jpg")
                                  << cameraName << curdate << 1
                                  << QString("my_image");

    QTest::newRow("all chars") << QString("%") << QString("1234567890'QWERTZUIOP+ASDFGHJKL#<yxcvbnm,.-.png")
                               << cameraName << curdate << 1
                               << QString("1234567890'qwertzuiop+asdfghjkl#<yxcvbnm,");
}

void ManualRenameInputTest::testLowercaseToken()
{
    QFETCH(QString,     parseString);
    QFETCH(QString,     filename);
    QFETCH(QString,     cameraName);
    QFETCH(QDateTime,   cameraDate);
    QFETCH(int,         index);
    QFETCH(QString,     result);

    QString parsed = ManualRenameInput::parser(parseString, filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);

    // test again with bound parse method
    ManualRenameInput mrename;
    mrename.setText(parseString);
    parsed = mrename.parse(filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);
}

void ManualRenameInputTest::testCameraToken_data()
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

    QTest::newRow("empty") << QString("[cam]") << filename << QString() << curdate << 1
                           << QString();

    QTest::newRow("   ") << QString("[cam]") << filename << QString("   ") << curdate << 1
                         << QString();

    QTest::newRow("[cam]") << QString("[cam]") << filename << camname << curdate << 1
                           << camname;

    QTest::newRow("[Cam]") << QString("[Cam]") << filename << camname << curdate << 1
                           << camname;

    QTest::newRow("[CAM]") << QString("[CAM]") << filename << camname << curdate << 1
                           << camname;

    QTest::newRow("[cam$]") << QString("[cam$]") << filename << camname << curdate << 1
                            << camname;

    QTest::newRow("[cAm$]") << QString("[cAm$]") << filename << camname << curdate << 1
                            << camname;

    QTest::newRow("[cam$*]") << QString("[cam$*]") << filename << camname << curdate << 1
                             << camname;

    QTest::newRow("[cam$$$$]") << QString("[cam$$$$]") << filename << camname << curdate << 1
                               << camname;

    QTest::newRow("[cam%]") << QString("[cam%]") << filename << camname << curdate << 1
                            << camname.toLower();

    QTest::newRow("[cam%*]") << QString("[cam%*]") << filename << camname << curdate << 1
                             << camname;

    QTest::newRow("[cam%%]") << QString("[cam%%]") << filename << camname << curdate << 1
                            << camname;

    QTest::newRow("[cam&]") << QString("[cam&]") << filename << camname << curdate << 1
                            << camname.toUpper();

    QTest::newRow("[cam*]") << QString("[cam*]") << filename << QString("My super camera") << curdate << 1
                            << QString("My Super Camera");

    QTest::newRow("[ cam ]") << QString("[ cam ]") << filename << camname << curdate << 1
                             << QString("[ cam ]");

    QTest::newRow("[camcam]") << QString("[camcam]") << filename << camname << curdate << 1
                              << QString("[camcam]");
}

void ManualRenameInputTest::testCameraToken()
{
    QFETCH(QString,     parseString);
    QFETCH(QString,     filename);
    QFETCH(QString,     cameraName);
    QFETCH(QDateTime,   cameraDate);
    QFETCH(int,         index);
    QFETCH(QString,     result);

    QString parsed = ManualRenameInput::parser(parseString, filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);

    // test again with bound parse method
    ManualRenameInput mrename;
    mrename.setText(parseString);
    parsed = mrename.parse(filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);
}

void ManualRenameInputTest::testCompleteParse_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("result");

    QString default_filename("myfile001.jpg");
    QString default_camname("Nikon D50");
    QDateTime default_curdate = QDateTime::currentDateTime();

    QTest::newRow("new-###") << QString("new-###") << default_filename << default_camname << default_curdate << 20
                             << QString("new-020");

    QTest::newRow("new-###_% (&-token in filename)") << QString("new-###_%")
                                                     << QString("my_#_file.jpg") << default_camname
                                                     << default_curdate << 20
                                                     << QString("new-020_my_#_file");

    QTest::newRow("#_new-###") << QString("#_new-###") << default_filename << default_camname << default_curdate << 20
                               << QString("20_new-020");

    QTest::newRow("%_##_&") << QString("%_##_&") << default_filename << default_camname << default_curdate << 1000
                            << QString("myfile001_1000_MYFILE001");

    QTest::newRow("[cam*]*") << QString("[cam*]*") << default_filename << QString("My super camera")
                             << default_curdate << 1
                             << QString("My Super CameraMyfile001");

    QTest::newRow("[cam*]_*") << QString("[cam*]_*") << default_filename << QString("My super camera")
                              << default_curdate << 1
                              << QString("My Super Camera_Myfile001");

    QTest::newRow("cam*") << QString("cam*") << default_filename << QString("My super camera")
                          << default_curdate << 1
                          << QString("camMyfile001");
}

void ManualRenameInputTest::testCompleteParse()
{
    QFETCH(QString,     parseString);
    QFETCH(QString,     filename);
    QFETCH(QString,     cameraName);
    QFETCH(QDateTime,   cameraDate);
    QFETCH(int,         index);
    QFETCH(QString,     result);

    QString parsed = ManualRenameInput::parser(parseString, filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);

    // test again with bound parse method
    ManualRenameInput mrename;
    mrename.setText(parseString);
    parsed = mrename.parse(filename, cameraName, cameraDate, index);
    QCOMPARE(parsed, result);
}

void ManualRenameInputTest::testEmptyParseString()
{
    QString filename("myfilename001.jpg");
    QDateTime curdate = QDateTime::currentDateTime();

    // for tests with the bound parse method
    ManualRenameInput mrename;

    // test for empty parser string
    QString parsed = ManualRenameInput::parser(QString(), filename, QString(), curdate, 1);
    QCOMPARE(parsed, QString("myfilename001"));
    parsed = mrename.parse(filename, QString(), curdate, 1);
    QCOMPARE(parsed, QString("myfilename001"));

    parsed = ManualRenameInput::parser(QString(""), filename, QString(), curdate, 1);
    QCOMPARE(parsed, QString("myfilename001"));
    mrename.setText("");
    parsed = mrename.parse(filename, QString(), curdate, 1);
    QCOMPARE(parsed, QString("myfilename001"));

    parsed = ManualRenameInput::parser(QString("   "), filename, QString(), curdate, 1);
    QCOMPARE(parsed, QString("myfilename001"));
    mrename.setText("   ");
    parsed = mrename.parse(filename, QString(), curdate, 1);
    QCOMPARE(parsed, QString("myfilename001"));

    // the following is not invalid
    parsed = ManualRenameInput::parser(QString("  %_##"), filename, QString(), curdate, 1);
    QCOMPARE(parsed, QString("  myfilename001_01"));
    mrename.setText("  %_##");
    parsed = mrename.parse(filename, QString(), curdate, 1);
    QCOMPARE(parsed, QString("  myfilename001_01"));
}

void ManualRenameInputTest::testSetters()
{
    ManualRenameInput mrename;

    mrename.setText("this is a test");
    QVERIFY(!mrename.text().isEmpty());
    QCOMPARE(mrename.text(), QString("this is a test"));
}
