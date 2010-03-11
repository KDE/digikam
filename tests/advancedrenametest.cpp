/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the AdvancedRename utility
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

// Qt includes

#include <QFileInfo>

// KDE includes

#include <qtest_kde.h>

using namespace Digikam;

QTEST_KDEMAIN(AdvancedRenameWidgetTest, GUI)

const QString fileName("advancedrename_testimage.jpg");
const QString filePath(KDESRCDIR+fileName);

void AdvancedRenameWidgetTest::testFileNameToken()
{
    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = "[file]";

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, fileName);
}

void AdvancedRenameWidgetTest::testFileExtensionToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow(fileName.toAscii())  << QString("[ext]") << QString("jpg.jpg");
    QTest::newRow("[ext]_lala_####") << QString("[ext]_lala_####") << QString("jpg_lala_0001.jpg");
    QTest::newRow("[ext]_lala_####[ext]") << QString("[ext]_lala_####[ext]") << QString("jpg_lala_0001jpg.jpg");
    QTest::newRow("[ext]_lala_####.[ext]") << QString("[ext]_lala_####.[ext]") << QString("jpg_lala_0001.jpg");
    QTest::newRow("[ext]_lala_####.[ext]{upper}") << QString("[ext]_lala_####.[ext]{upper}") << QString("jpg_lala_0001.JPG");
    QTest::newRow("[ext]_lala_####[ext]{upper}") << QString("[ext]_lala_####[ext]{upper}") << QString("jpg_lala_0001JPG.jpg");
}

void AdvancedRenameWidgetTest::testFileExtensionToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testFileOwnerToken()
{
    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = KUrl(KDESRCDIR"/test.png");
    settings.parseString = "[user]";

    QFileInfo fi(settings.fileUrl.toLocalFile());
    QVERIFY(fi.exists());
    QVERIFY(fi.isReadable());

    QString userName = fi.owner();
    QVERIFY(!userName.isEmpty());

    QString result = userName + ".png";
    QString parsed = parser.parse(settings);

    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testFileGroupToken()
{
    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = KUrl(KDESRCDIR"/test.png");
    settings.parseString = "[group]";

    QFileInfo fi(settings.fileUrl.toLocalFile());
    QVERIFY(fi.exists());
    QVERIFY(fi.isReadable());

    QString groupName = fi.group();
    QVERIFY(!groupName.isEmpty());

    QString result = groupName + ".png";
    QString parsed = parser.parse(settings);

    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testDirectoryNameToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("[dir]")          << QString("[dir]")          << QString("tests.jpg");
    QTest::newRow("[dir.]")         << QString("[dir.]")         << QString("digikam.jpg");
    QTest::newRow("[dir.]_[dir]")   << QString("[dir.]_[dir]")   << QString("digikam_tests.jpg");
    QTest::newRow("[dir.......]")   << QString("[dir.......]")   << fileName;
}

void AdvancedRenameWidgetTest::testDirectoryNameToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testNumberToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

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

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;

    QString parsed    = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testTrimmedModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("camera");
    QTest::addColumn<QString>("result");

    QTest::newRow("01") << QString("[cam]{trim}") << QString("Nikon D50")   << QString("Nikon D50");
    QTest::newRow("02") << QString("[cam]{trim}") << QString(" Nikon D50 ") << QString("Nikon D50");
    QTest::newRow("03") << QString("[cam]{trim}") << QString("        Nikon     D50    ") << QString("Nikon D50");
}

void AdvancedRenameWidgetTest::testTrimmedModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   camera);
    QFETCH(QString,   result);

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;
    settings.cameraName  = camera;
    settings.useOriginalFileExtension = false;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testFirstLetterOfEachWordUppercaseModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("camera");
    QTest::addColumn<QString>("result");

    QTest::newRow("nikond50") << QString("[cam]{firstupper}")
                              << QString("nikond50") << QString("Nikond50");

    QTest::newRow("nikon d50") << QString("[cam]{firstupper}")
                              << QString("nikon d50") << QString("Nikon D50");

    QTest::newRow("nikon_d50") << QString("[cam]{firstupper}")
                              << QString("nikon_d50") << QString("Nikon_D50");
}

void AdvancedRenameWidgetTest::testFirstLetterOfEachWordUppercaseModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   camera);
    QFETCH(QString,   result);

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;
    settings.cameraName  = camera;
    settings.useOriginalFileExtension = false;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testChainedModifiers_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("[file]*{upper}") << QString("[file]{firstupper}{upper}") << QString("ADVANCEDRENAME_TESTIMAGE.jpg");
    QTest::newRow("[file]{3-}*")    << QString("[file]{3-}{firstupper}")    << QString("Vancedrename_Testimage.jpg");

    QTest::newRow("[file]{3-}{replace:\"name\",\"age\"}{firstupper}")
            << QString("[file]{3-}{replace:\"name\",\"age\"}{firstupper}")
            << QString("Vancedreage_Testimage.jpg");
}

void AdvancedRenameWidgetTest::testChainedModifiers()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testUppercaseModifier()
{
    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = "[file]{upper}";

    QString parsed = parser.parse(settings);
    QFileInfo fi(filePath);
    QCOMPARE(parsed, fi.baseName().toUpper() + "." + fi.suffix());
}

void AdvancedRenameWidgetTest::testUniqueModifier()
{
    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.cameraName  = QString("Nikon D50");
    settings.parseString = QString("[file]{unique}_T[date:hhmmss]{unique}_[cam]{unique}");

#define DIGITS_STR(VALUE, DIGITS) QString("%1").arg(VALUE, DIGITS, 10, QChar('0'))

    QStringList validResults;
    validResults << QString("advancedrename_testimage_T100012_Nikon D50.jpg");
    validResults << QString("advancedrename_testimage_%1_T100012_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(1, 1));
    validResults << QString("advancedrename_testimage_%1_T214536_Nikon D50_%2.jpg")
            .arg(DIGITS_STR(2, 1)).arg(DIGITS_STR(2, 1));
    validResults << QString("advancedrename_testimage_%1_T214536_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(3, 1)).arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(3, 1));
    validResults << QString("advancedrename_testimage_%1_T214536_%2_Nikon D50_%3.jpg")
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
    validResults << QString("advancedrename_testimage_T100012_Nikon D50.jpg");
    validResults << QString("advancedrename_testimage_%1_T100012_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(1, 2)).arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(1, 4));
    validResults << QString("advancedrename_testimage_%1_T214536_Nikon D50_%2.jpg")
            .arg(DIGITS_STR(2, 2)).arg(DIGITS_STR(2, 4));
    validResults << QString("advancedrename_testimage_%1_T214536_%2_Nikon D50_%3.jpg")
            .arg(DIGITS_STR(3, 2)).arg(DIGITS_STR(1, 1)).arg(DIGITS_STR(3, 4));
    validResults << QString("advancedrename_testimage_%1_T214536_%2_Nikon D50_%3.jpg")
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

void AdvancedRenameWidgetTest::testFillModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("[file]{fill:30}")
       << QString("[file]{fill:30}")  << QString("advancedrename_testimage______.jpg");

    QTest::newRow("[file]{fill:30,l}")
       << QString("[file]{fill:30,l}")  << QString("advancedrename_testimage______.jpg");

    QTest::newRow("[file]{fill:30,r}")
       << QString("[file]{fill:30,r}")  << QString("______advancedrename_testimage.jpg");

    QTest::newRow("[file]{fill:30,l,\"x\"}")
       << QString("[file]{fill:30,l,\"x\"}")  << QString("advancedrename_testimagexxxxxx.jpg");

    QTest::newRow("[file]{fill:30,r,\"x\"}")
       << QString("[file]{fill:30,r,\"x\"}")  << QString("xxxxxxadvancedrename_testimage.jpg");
}

void AdvancedRenameWidgetTest::testFillModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testReplaceModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("[file]{replace:\"adv\",\"AAA\"}")
       << QString("[file]{replace:\"adv\",\"AAA\"}") << QString("AAAancedrename_testimage.jpg");

    QTest::newRow("[file]{replace:\"Adv\",\"AAA\"}")
       << QString("[file]{replace:\"Adv\",\"AAA\"}") << QString("advancedrename_testimage.jpg");

    QTest::newRow("[file]{replace:\"Adv\",\"AAA\",i}")
       << QString("[file]{replace:\"Adv\",\"AAA\",i}") << QString("AAAancedrename_testimage.jpg");

    QTest::newRow("[file]{replace:\"Adv\",\"AAA\",ri}")
       << QString("[file]{replace:\"Adv\",\"AAA\",ri}") << QString("AAAancedrename_testimage.jpg");

    QTest::newRow("[file]{replace:\"Adv\",\"AAA\",ir}")
       << QString("[file]{replace:\"Adv\",\"AAA\",ir}") << QString("AAAancedrename_testimage.jpg");

    QTest::newRow("[file]{replace:\"a.v\",\"AAA\"}")
       << QString("[file]{replace:\"a.v\",\"AAA\"}") << QString("advancedrename_testimage.jpg");

    QTest::newRow("[file]{replace:\"a.v\",\"AAA\",r}")
       << QString("[file]{replace:\"a.v\",\"AAA\",r}") << QString("AAAancedrename_testimage.jpg");
}

void AdvancedRenameWidgetTest::testReplaceModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testRangeModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("[file]{1}")    << QString("[file]{1}")   << QString("a.jpg");
    QTest::newRow("[file]{3}")    << QString("[file]{3}")   << QString("v.jpg");
    QTest::newRow("[file]{1-3}")  << QString("[file]{1-3}") << QString("adv.jpg");
    QTest::newRow("[file]{3-}")   << QString("[file]{3-}")  << QString("vancedrename_testimage.jpg");
}

void AdvancedRenameWidgetTest::testRangeModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testDefaultValueModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("[meta:Iptc.Application2.Keywords]_[file]") << QString("[meta:Iptc.Application2.Keywords]{default:\"Unknown\"}_[file]")
                                  << QString("Colca Canyon_advancedrename_testimage.jpg");

    QTest::newRow("[meta:Exif.GPSInfo.GPSAltitude]_[file]") << QString("[meta:Exif.GPSInfo.GPSAltitude]{default:\"Unknown\"}_[file]")
                                                            << QString("Unknown_advancedrename_testimage.jpg");
}

void AdvancedRenameWidgetTest::testDefaultValueModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = parseString;
//    settings.cameraName  = camera;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testLowercaseModifier()
{
    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl     = filePath;
    settings.parseString = "[file]{lower}";

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, fileName.toLower());
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

    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl      = filename;
    settings.parseString  = parseString;
    settings.cameraName   = cameraName;
    settings.dateTime     = cameraDate;

    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testEmptyParseString()
{
    ParseSettings settings;
    DefaultRenameParser parser;

    settings.fileUrl = filePath;

    // test for empty parser string
    settings.parseString.clear();
    QString parsed = parser.parse(settings);
    QCOMPARE(parsed, fileName);

    settings.parseString = QString("   ");
    parsed = parser.parse(settings);
    QCOMPARE(parsed, fileName);
}
