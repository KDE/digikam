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

#include <kurl.h>
#include <qtest_kde.h>

using namespace Digikam;

QTEST_KDEMAIN(AdvancedRenameWidgetTest, GUI)

const QString fileName("advancedrename_testimage.jpg");
const QString fileName2("advancedrename_testimage2.jpg");
const QString filePath(KDESRCDIR+fileName);
const QString filePath2(KDESRCDIR+fileName2);

void AdvancedRenameWidgetTest::testFileNameToken()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    KUrl url(filePath);
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles("[file]");

    QString parsed = manager.newName(filePath);
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

    QList<ParseSettings> files;
    ParseSettings ps;
    KUrl url(filePath);
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testFileOwnerToken()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    QString filePath = KDESRCDIR"/test.png";
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles("[user]");

    QFileInfo fi(ps.fileUrl.toLocalFile());
    QVERIFY(fi.exists());
    QVERIFY(fi.isReadable());

    QString userName = fi.owner();
    QVERIFY(!userName.isEmpty());

    QString result = userName + ".png";
    QString parsed = manager.newName(filePath);

    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testFileGroupToken()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    QString filePath = KDESRCDIR"/test.png";
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles("[group]");

    QFileInfo fi(ps.fileUrl.toLocalFile());
    QVERIFY(fi.exists());
    QVERIFY(fi.isReadable());

    QString groupName = fi.group();
    QVERIFY(!groupName.isEmpty());

    QString result = groupName + ".png";
    QString parsed = manager.newName(filePath);

    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testDirectoryNameToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    // The main directory of digikam can have different names, depending on how the
    // user named it. Therefore we have to detect the name here:
    const KUrl dir2up = KUrl(KDESRCDIR).upUrl();
    const QString dir2upString = dir2up.url();
    QString digikamDir = dir2upString.right(dir2upString.size()-dir2up.upUrl().url().size());
    digikamDir.chop(1);

    QTest::newRow("[dir]")          << QString("[dir]")          << QString("tests.jpg");
    QTest::newRow("[dir.]")         << QString("[dir.]")         << QString("%1.jpg").arg(digikamDir);
    QTest::newRow("[dir.]_[dir]")   << QString("[dir.]_[dir]")   << QString("%1_tests.jpg").arg(digikamDir);
    QTest::newRow("[dir......................................................................]")
       << QString("[dir......................................................................]")
       << fileName;
}

void AdvancedRenameWidgetTest::testDirectoryNameToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    KUrl url(filePath);
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testNumberToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("#")                      << QString("#")                      << QString("1.jpg");
    QTest::newRow("####[2,3]")              << QString("####[2,3]")              << QString("0002.jpg");
    QTest::newRow("####[2,3]_bla_## ###")   << QString("####[2,3]_bla_## ###")   << QString("0002_bla_01 001.jpg");
    QTest::newRow("####[2,3]_bla_## ###")   << QString("####[2,3]_bla_## ###")   << QString("0002_bla_01 001.jpg");
    QTest::newRow("####[2,3]_bla_## ###")   << QString("####[2,3]_bla_## ###")   << QString("0002_bla_01 001.jpg");
    QTest::newRow("###[100]_bla")           << QString("###[100]_bla")           << QString("100_bla.jpg");
    QTest::newRow("###[e,1,100]_bla")       << QString("###[e,1,100]_bla")       << QString("001_bla.jpg");
}

void AdvancedRenameWidgetTest::testNumberToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    KUrl url(filePath);
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testFirstLetterOfEachWordUppercaseModifier()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    KUrl url(filePath);
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles("[file]{firstupper}");

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, QString("Advancedrename_Testimage.jpg"));
}

void AdvancedRenameWidgetTest::testChainedModifiers_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("[file]*{upper}") << QString("[file]{firstupper}{upper}") << QString("ADVANCEDRENAME_TESTIMAGE.jpg");
    QTest::newRow("[file]{range:3,}*") << QString("[file]{range:3,}{firstupper}")    << QString("Vancedrename_Testimage.jpg");

    QTest::newRow("[file]{range:3,}{replace:\"name\",\"age\"}{firstupper}")
            << QString("[file]{range:3,}{replace:\"name\",\"age\"}{firstupper}")
            << QString("Vancedreage_Testimage.jpg");
}

void AdvancedRenameWidgetTest::testChainedModifiers()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testUppercaseModifier()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles("[file]{upper}");

    QString parsed = manager.newName(filePath);
    QFileInfo fi(filePath);
    QCOMPARE(parsed, fi.baseName().toUpper() + "." + fi.suffix());
}

void AdvancedRenameWidgetTest::testUniqueModifier()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = KUrl(filePath);
    files << ps;
    ps.fileUrl = KUrl(filePath2);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles("[file]_[dir]{unique}");

    QString parsed = manager.newName(filePath);
    QString parsed2 = manager.newName(filePath2);

    // parse again, unique tokens should not be modified
    manager.parseFiles("[file]_[dir]{unique}");

    QString parsed3 = manager.newName(filePath);
    QString parsed4 = manager.newName(filePath2);

    QCOMPARE(parsed,  parsed3);
    QCOMPARE(parsed2, parsed4);
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

    QList<ParseSettings> files;
    ParseSettings ps;
    KUrl url(filePath);
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testRangeModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("[file]{range:1}")    << QString("[file]{range:1}")   << QString("a.jpg");
    QTest::newRow("[file]{range:3}")    << QString("[file]{range:3}")   << QString("v.jpg");
    QTest::newRow("[file]{range:1,3}")  << QString("[file]{range:1,3}") << QString("adv.jpg");
    QTest::newRow("[file]{range:3,}")   << QString("[file]{range:3,}")  << QString("vancedrename_testimage.jpg");
}

void AdvancedRenameWidgetTest::testRangeModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    KUrl url(filePath);
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
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

    QList<ParseSettings> files;
    ParseSettings ps;
    KUrl url(filePath);
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameWidgetTest::testLowercaseModifier()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles("[file]{lower}");

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, fileName.toLower());
}

void AdvancedRenameWidgetTest::testEmptyParseString()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = KUrl(filePath);
    files << ps;
    AdvancedRenameManager manager(files);

    // test for empty parser string
    manager.parseFiles("");

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, fileName);

    manager.parseFiles("      ");
    parsed = manager.newName(filePath);
    QCOMPARE(parsed, fileName);
}
