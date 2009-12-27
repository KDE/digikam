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

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
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
    QTest::newRow("[ext]_lala_####.[ext]{upper}") << QString("[ext]_lala_####.[ext]{upper}") << filename << QString("jpg_lala_0001.JPG");
    QTest::newRow("[ext]_lala_####[ext]{upper}") << QString("[ext]_lala_####[ext]{upper}") << filename << QString("jpg_lala_0001JPG.jpg");
}

void AdvancedRenameWidgetTest::testFileExtensionToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testDirectoryNameToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString filename("/mnt/data/photos/2009/digikam_tests/myfile001.jpg");

    QTest::newRow("[dir]")          << QString("[dir]")          << filename << QString("digikam_tests.jpg");
    QTest::newRow("[dir.]")         << QString("[dir.]")         << filename << QString("2009.jpg");
    QTest::newRow("[dir.]_[dir]")   << QString("[dir.]_[dir]")   << filename << QString("2009_digikam_tests.jpg");
    QTest::newRow("[dir.......]")   << QString("[dir.......]")   << filename << QString("myfile001.jpg");
}

void AdvancedRenameWidgetTest::testDirectoryNameToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testNumberToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    // filename is "myimage_1234.jpg", it is not defined in here to avoid typing it into the results all the time.
    // it will be defined in the test method

    QTest::newRow("#")                      << QString("#")                      << QString("1.jpg");
    QTest::newRow("####[2,3]")              << QString("####[2,3]")              << QString("0002.jpg");
    QTest::newRow("####[2,3]_bla_## ###")   << QString("####[2,3]_bla_## ###")   << QString("0002_bla_01 001.jpg");
    QTest::newRow("####[2, 3]_bla_## ###")  << QString("####[2, 3]_bla_## ###")  << QString("0002_bla_01 001.jpg");
    QTest::newRow("####[ 2, 3]_bla_## ###") << QString("####[ 2, 3]_bla_## ###") << QString("0002_bla_01 001.jpg");
    QTest::newRow("###[100]_bla")           << QString("###[100]_bla")           << QString("100_bla.jpg");
    QTest::newRow("###[100,]_bla")          << QString("###[100,]_bla")          << QString("100_bla.jpg");
    QTest::newRow("###[100,   ]_bla")       << QString("###[100,   ]_bla")       << QString("100_bla.jpg");
    QTest::newRow("###[100   ,   ]_bla")    << QString("###[100   ,   ]_bla")    << QString("100_bla.jpg");
}

void AdvancedRenameWidgetTest::testNumberToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = QString("myimage_1234.jpg");
    settings.parseString = parseString;
    QString parsed    = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testTrimmedModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("01") << QString("[file]{trim}") << QString("myfilename001 .jpg")  << fileName;
    QTest::newRow("02") << QString("[file]{trim}") << QString(" myfilename001 .jpg") << fileName;

    QTest::newRow("03") << QString("[file]{trim}") << QString("       myfilename001      .jpg")    << fileName;
    QTest::newRow("04") << QString("[file]{trim}") << QString("        myfilename    001    .jpg") << QString("myfilename 001.jpg");
}

void AdvancedRenameWidgetTest::testTrimmedModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testFirstLetterOfEachWordUppercaseModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("[file]{firstupper}") << QString("[file]{firstupper}")
                                        << fileName << QString("Myfilename001.jpg");

    QTest::newRow("[file]{firstupper}{firstupper}") << QString("[file]{firstupper}{firstupper}")
                                                    << fileName << QString("Myfilename001.jpg");

    QTest::newRow("my image.jpg") << QString("[file]{firstupper}")
                                  << QString("my image.jpg") << QString("My Image.jpg");

    QTest::newRow("my_image.jpg") << QString("[file]{firstupper}")
                                  << QString("my_image.jpg") << QString("My_Image.jpg");
}

void AdvancedRenameWidgetTest::testFirstLetterOfEachWordUppercaseModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testChainedModifiers_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("[file]*{upper}") << QString("[file]{firstupper}{upper}") << fileName << QString("MYFILENAME001.jpg");
    QTest::newRow("[file]{3-}*")    << QString("[file]{3-}{firstupper}")    << fileName << QString("Filename001.jpg");

    QTest::newRow("[file]{3-}{replace:\"name\",\"age\"}{firstupper}")
            << QString("[file]{3-}{replace:\"name\",\"age\"}{firstupper}")
            << fileName << QString("Fileage001.jpg");
}

void AdvancedRenameWidgetTest::testChainedModifiers()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testUppercaseModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("myfilename001.jpg")  << QString("[file]{upper}") << fileName << QString("MYFILENAME001.jpg");
    QTest::newRow("my/filename001.jpg") << QString("[file]{upper}") << QString("my/filename001.jpg")
                                        << QString("FILENAME001.jpg");

    QTest::newRow("myfilename001.jpg(token in filename)") << QString("[file]{upper}") << QString("myfilename0&01.jpg")
                                                          << QString("MYFILENAME0&01.jpg");

    QTest::newRow("my image.jpg") << QString("[file]{upper}") << QString("my image.jpg")
                                  << QString("MY IMAGE.jpg");

    QTest::newRow("my_image.jpg") << QString("[file]{upper}") << QString("my_image.jpg")
                                  << QString("MY_IMAGE.jpg");
}

void AdvancedRenameWidgetTest::testUppercaseModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testUniqueModifier()
{
    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = QString("myfile001.jpg");
    settings.cameraName  = QString("Nikon D50");
    settings.parseString = QString("[file]{unique}_T[date:hhmmss]{unique}_[cam]{unique}");

#define DIGITS_STR(VALUE, DIGITS) QString("%1").arg(VALUE, DIGITS, 10, QChar('0'))

    QStringList validResults;
    validResults << QString("myfile001_T100012_Nikon D50.jpg");
    validResults << QString("myfile001_%1_T100012_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(1, 1));
    validResults << QString("myfile001_%1_T214536_Nikon D50_%2.jpg")
            .arg(DIGITS_STR(2, 1)).arg(DIGITS_STR(2, 1));
    validResults << QString("myfile001_%1_T214536_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(3, 1)).arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(3, 1));
    validResults << QString("myfile001_%1_T214536_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(4, 1)).arg(DIGITS_STR(2, 1)).arg(DIGITS_STR(4, 1));

    QTime t1;
    t1.setHMS(10, 00, 12);

    QTime t2;
    t2.setHMS(21, 45, 36);

    QDateTime date = QDateTime::currentDateTime();
    date.setTime(t1);
    settings.dateTime = date;

    QStringList results;
    results << parser.parse(settings);
    results << parser.parse(settings);
    date.setTime(t2);
    settings.dateTime = date;
    results << parser.parse(settings);
    results << parser.parse(settings);
    results << parser.parse(settings);

    QCOMPARE(results, validResults);

    // --------------------------------------------------------

    settings.parseString = QString("[file]{unique:2}_T[date:hhmmss]{unique}_[cam]{unique:4}");
    results.clear();
    validResults.clear();
    parser.reset();
    date.setTime(t1);
    settings.dateTime = date;
    validResults << QString("myfile001_T100012_Nikon D50.jpg");
    validResults << QString("myfile001_%1_T100012_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(1, 2)).arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(1, 4));
    validResults << QString("myfile001_%1_T214536_Nikon D50_%2.jpg")
            .arg(DIGITS_STR(2, 2)).arg(DIGITS_STR(2, 4));
    validResults << QString("myfile001_%1_T214536_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(3, 2)).arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(3, 4));
    validResults << QString("myfile001_%1_T214536_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(4, 2)).arg(DIGITS_STR(2, 1)).arg(DIGITS_STR(4, 4));

    results << parser.parse(settings);
    results << parser.parse(settings);
    date.setTime(t2);
    settings.dateTime = date;
    results << parser.parse(settings);
    results << parser.parse(settings);
    results << parser.parse(settings);

    QCOMPARE(results, validResults);

#undef DIGITS_STR
}

void AdvancedRenameWidgetTest::testReplaceModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("DSC1234.jpg");

    QTest::newRow("[file]{replace:\"DSC\",\"AAA\"}")
       << QString("[file]{replace:\"DSC\",\"AAA\"}") << fileName << QString("AAA1234.jpg");

    QTest::newRow("[file]{replace:\"Dsc\",\"AAA\"}")
       << QString("[file]{replace:\"Dsc\",\"AAA\"}") << fileName << QString("DSC1234.jpg");

    QTest::newRow("[file]{replace:\"Dsc\",\"AAA\",i}")
       << QString("[file]{replace:\"Dsc\",\"AAA\",i}") << fileName << QString("AAA1234.jpg");

    QTest::newRow("[file]{replace:\"Dsc\",\"AAA\",ri}")
       << QString("[file]{replace:\"Dsc\",\"AAA\",ri}") << fileName << QString("AAA1234.jpg");

    QTest::newRow("[file]{replace:\"Dsc\",\"AAA\",ir}")
       << QString("[file]{replace:\"Dsc\",\"AAA\",ir}") << fileName << QString("AAA1234.jpg");

    QTest::newRow("[file]{replace:\"D.C\",\"AAA\"}")
       << QString("[file]{replace:\"D.C\",\"AAA\"}") << fileName << QString("DSC1234.jpg");

    QTest::newRow("[file]{replace:\"D.C\",\"AAA\",r}")
       << QString("[file]{replace:\"D.C\",\"AAA\",r}") << fileName << QString("AAA1234.jpg");
}

void AdvancedRenameWidgetTest::testReplaceModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testRangeModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("DSC1234.jpg");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("[file]{1}")    << QString("[file]{1}")   << fileName << QString("D.jpg");
    QTest::newRow("[file]{3}")    << QString("[file]{3}")   << fileName << QString("C.jpg");
    QTest::newRow("[file]{1-3}")  << QString("[file]{1-3}") << fileName << QString("DSC.jpg");
    QTest::newRow("[file]{3-}")   << QString("[file]{3-}")  << fileName << QString("C1234.jpg");
}

void AdvancedRenameWidgetTest::testRangeModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
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

    QTest::newRow("[cam]_[file]") << QString("[cam]{default:\"Unknown\"}_[file]") << fileName << QString("Canon Powershot A80")
                                  << QString("Canon Powershot A80_DSC1234.jpg");

    QTest::newRow("[cam]{\"Unknown\"}_[file]") << QString("[cam]{default:\"Unknown\"}_[file]") << fileName << QString()
                                               << QString("Unknown_DSC1234.jpg");
}

void AdvancedRenameWidgetTest::testDefaultValueModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   camera);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;
    settings.cameraName  = camera;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testLowercaseModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QString fileName("myfilename001.jpg");

    QTest::newRow("myfilename001.jpg") << QString("[file]{lower}") << fileName << QString("myfilename001.jpg");

    QTest::newRow("my image.jpg") << QString("[file]{lower}") << QString("MY image.jpg")
                                  << QString("my image.jpg");

    QTest::newRow("my_image.jpg") << QString("[file]{lower}") << QString("mY_Image.jpg")
                                  << QString("my_image.jpg");
}

void AdvancedRenameWidgetTest::testLowercaseModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl     = filename;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testCameraToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("cameraName");
    QTest::addColumn<QDateTime>("cameraDate");
    QTest::addColumn<QString>("result");

    QString filename("myfile001.jpg");
    QString camname("Nikon D50");
    QString camname_ext("Nikon D50.jpg");
    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("   ")      << QString("[cam]")    << filename << QString("   ") << curdate << QString("myfile001.jpg");
    QTest::newRow("[cam]")    << QString("[cam]")    << filename << camname        << curdate << camname_ext;
    QTest::newRow("[ cam ]")  << QString("[ cam ]")  << filename << camname        << curdate << QString("[ cam ].jpg");
    QTest::newRow("[camcam]") << QString("[camcam]") << filename << camname        << curdate << QString("[camcam].jpg");
}

void AdvancedRenameWidgetTest::testCameraToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   filename);
    QFETCH(QString,   cameraName);
    QFETCH(QDateTime, cameraDate);
    QFETCH(QString,   result);

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl      = filename;
    settings.parseString  = parseString;
    settings.cameraName   = cameraName;
    settings.dateTime     = cameraDate;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testEmptyParseString()
{
    QString filename("myfilename001.jpg");
    QDateTime curdate = QDateTime::currentDateTime();

    DefaultRenameParser parser;

    ParseSettings settings;
    settings.fileUrl      = filename;
    settings.cameraName   = QString();
    settings.dateTime     = curdate;

    // test for empty parser string
    settings.parseString.clear();
    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, QString("myfilename001.jpg"));

    settings.parseString = QString("   ");
    parsed = parser.parse(settings);
    QCOMPARE(parsed, QString("myfilename001.jpg"));

    // the following is not invalid
    settings.parseString = QString("  [file]{lower}_##");
    parsed = parser.parse(settings);
    QCOMPARE(parsed, QString("  myfilename001_01.jpg"));
}
