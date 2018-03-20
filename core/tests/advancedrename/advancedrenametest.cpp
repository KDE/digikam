/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the AdvancedRename utility
 *
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
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

// C++ includes

#include <algorithm>

// Qt includes

#include <QFileInfo>
#include <QTest>
#include <QDir>

using namespace Digikam;

QTEST_MAIN(AdvancedRenameTest)

const QString imagesDir = QLatin1String("data/");

QString createFilePath(const QString& file)
{
    return QString(QFINDTESTDATA(imagesDir) + file);
}

Q_DECLARE_METATYPE(QList<int>)

const QString fileName  = QLatin1String("advancedrename_testimage.jpg");
const QString fileName2 = QLatin1String("advancedrename_testimage2.jpg");
const QString fileName3 = QLatin1String("001a.jpg");
const QString fileName4 = QLatin1String("test.png");
const QString fileName5 = QLatin1String("myfile.jpg");
const QString fileName6 = QLatin1String("my_file.jpg");
const QString fileName7 = QLatin1String("holiday_spain_2011_img001.jpg");
const QString fileName8 = QLatin1String("my images.jpg");
const QString fileName9 = QLatin1String("holiday_spain_2011_001img.jpg");

const QString filePath  = createFilePath(fileName);
const QString filePath2 = createFilePath(fileName2);
const QString filePath3 = createFilePath(fileName3);
const QString filePath4 = createFilePath(fileName4);
const QString filePath5 = createFilePath(fileName5);
const QString filePath6 = createFilePath(fileName6);
const QString filePath7 = createFilePath(fileName7);
const QString filePath8 = createFilePath(fileName8);
const QString filePath9 = createFilePath(fileName9);

void AdvancedRenameTest::testFileNameToken()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(QLatin1String("[file]"));

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, fileName);
}

void AdvancedRenameTest::testFileExtensionToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("[ext]")
               << "[ext]"
               << "jpg.jpg";

    QTest::newRow(".[ext]")
               << ".[ext]"
               << ".jpg";

    QTest::newRow("[ext].[ext]")
               << "[ext].[ext]"
               << "jpg.jpg";

    QTest::newRow("[ext].[ext]{upper}")
               << "[ext].[ext]{upper}"
               << "jpg.JPG";

    QTest::newRow("[ext]{upper}.[ext]{upper}")
               << "[ext]{upper}.[ext]{upper}"
               << "JPG.JPG";

    QTest::newRow("[ext]_lala_####")
               << "[ext]_lala_####"
               << "jpg_lala_0001.jpg";

    QTest::newRow("[ext]_lala_####[ext]")
               << "[ext]_lala_####[ext]"
               << "jpg_lala_0001jpg.jpg";

    QTest::newRow("[ext]_lala_####.[ext]")
               << "[ext]_lala_####.[ext]"
               << "jpg_lala_0001.jpg";

    QTest::newRow("[ext]_lala_####.[ext]{upper}")
               << "[ext]_lala_####.[ext]{upper}"
               << "jpg_lala_0001.JPG";

    QTest::newRow("[ext]_lala_####[ext]{upper}")
               << "[ext]_lala_####[ext]{upper}"
               << "jpg_lala_0001JPG.jpg";
}

void AdvancedRenameTest::testFileExtensionToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testFileOwnerToken()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath4);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(QLatin1String("[user]"));

    QFileInfo fi(ps.fileUrl.toLocalFile());
    QVERIFY(fi.exists());
    QVERIFY(fi.isReadable());

    QString userName = fi.owner();
    QVERIFY(!userName.isEmpty());

    QString result = userName + QLatin1String(".png");
    QString parsed = manager.newName(filePath4);

    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testFileGroupToken()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath4);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(QLatin1String("[group]"));

    QFileInfo fi(ps.fileUrl.toLocalFile());
    QVERIFY(fi.exists());
    QVERIFY(fi.isReadable());

    QString groupName = fi.group();
    QVERIFY(!groupName.isEmpty());

    QString result = groupName + QLatin1String(".png");
    QString parsed = manager.newName(filePath4);

    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testDirectoryNameToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    // The main directory of digiKam can have different names, depending on how the
    // user named it. Therefore we have to detect the name here.
    QDir dir(QFINDTESTDATA(imagesDir));
    dir.cdUp();
    const QString dir2up = dir.path();
    QDir dir2 = dir;
    dir2.cdUp();
    QString digikamDir   = dir2up.right(dir2up.size() - dir2.path().size() -1) +
                           QLatin1Char('/');
    qDebug() << "digikamDir: " << digikamDir;

    digikamDir.chop(1);

    QTest::newRow("[dir]")
            << "[dir]"
            << "data.jpg";

    QTest::newRow("[dir.]")
            << "[dir.]"
            << QString::fromUtf8("%1.jpg").arg(digikamDir).toLatin1().constData();

    QTest::newRow("[dir.]_[dir]")
            << "[dir.]_[dir]"
            << QString::fromUtf8("%1_data.jpg").arg(digikamDir).toLatin1().constData();

    QTest::newRow("[dir......................................................................]")
            << "[dir......................................................................]"
            << fileName;
}

void AdvancedRenameTest::testDirectoryNameToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testNumberToken_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("#")
            << "#"
            << "1.jpg";

    QTest::newRow("####[2,3]")
            << "####[2,3]"
            << "0002.jpg";

    QTest::newRow("####[2,3]_bla_## ###")
            << "####[2,3]_bla_## ###"
            << "0002_bla_01 001.jpg";

    QTest::newRow("####[2,3]_bla_## ###")
            << "####[2,3]_bla_## ###"
            << "0002_bla_01 001.jpg";

    QTest::newRow("####[2,3]_bla_## ###")
            << "####[2,3]_bla_## ###"
            << "0002_bla_01 001.jpg";

    QTest::newRow("###[100]_bla")
            << "###[100]_bla"
            << "100_bla.jpg";

    QTest::newRow("###[e,1,100]_bla")
            << "###[e,1,100]_bla"
            << "001_bla.jpg";
}

void AdvancedRenameTest::testNumberToken()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testFirstLetterOfEachWordUppercaseModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("result");

    QTest::newRow("myfile")
            << "[file]{firstupper}"
            << filePath5
            << "Myfile.jpg";

    QTest::newRow("my_file")
            << "[file]{firstupper}"
            << filePath6
            << "My_File.jpg";

    QTest::newRow("holiday_spain_2011_img001")
            << "[file]{firstupper}"
            << filePath7
            << "Holiday_Spain_2011_Img001.jpg";

    QTest::newRow("holiday_spain_2011_001img")
            << "[file]{firstupper}"
            << filePath9
            << "Holiday_Spain_2011_001Img.jpg";

    QTest::newRow("001a")
            << "[file]{firstupper}"
            << filePath3
            << "001A.jpg";

    QTest::newRow("my images")
            << "[file]{firstupper}"
            << filePath8
            << "My Images.jpg";

    QTest::newRow("<empty>")
            << "[file]{firstupper}"
            << ""
            << "";

    QTest::newRow(fileName.toLatin1().constData())
            << "[file]{firstupper}"
            << filePath
            << "Advancedrename_Testimage.jpg";
}

void AdvancedRenameTest::testFirstLetterOfEachWordUppercaseModifier()
{
    QFETCH(QString, parseString);
    QFETCH(QString, file);
    QFETCH(QString, result);

    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(file);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(file);
    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testChainedModifiers_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("[file]*{upper}")
            << "[file]{firstupper}{upper}"
            << "ADVANCEDRENAME_TESTIMAGE.jpg";

    QTest::newRow("[file]{range:3,}*")
            << "[file]{range:3,}{firstupper}"
            << "Vancedrename_Testimage.jpg";

    QTest::newRow("[file]{range:3,}{replace:\"name\",\"age\"}{firstupper}")
            << "[file]{range:3,}{replace:\"name\",\"age\"}{firstupper}"
            << "Vancedreage_Testimage.jpg";
}

void AdvancedRenameTest::testChainedModifiers()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testUppercaseModifier()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(QLatin1String("[file]{upper}"));

    QString parsed = manager.newName(filePath);
    QFileInfo fi(filePath);
    QString tmp = fi.baseName().toUpper() + QLatin1Char('.') + fi.suffix();
    QCOMPARE(parsed, tmp);
}

void AdvancedRenameTest::testUniqueModifier()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath2);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(QLatin1String("[file]_[dir]{unique}"));

    QString parsed = manager.newName(filePath);
    QString parsed2 = manager.newName(filePath2);

    // parse again, unique tokens should not be modified
    manager.parseFiles(QLatin1String("[file]_[dir]{unique}"));

    QString parsed3 = manager.newName(filePath);
    QString parsed4 = manager.newName(filePath2);

    QCOMPARE(parsed,  parsed3);
    QCOMPARE(parsed2, parsed4);
}

void AdvancedRenameTest::addFiles_should_only_add_files()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath2);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath3);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath4);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath5);
    files << ps;
    AdvancedRenameManager manager(files);
    QCOMPARE(manager.fileList().count(), 5);

    QList<ParseSettings> additionalFiles;
    ps.fileUrl = QUrl::fromLocalFile(filePath6);
    additionalFiles << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath7);
    additionalFiles << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath8);
    additionalFiles << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath9);
    additionalFiles << ps;
    manager.addFiles(additionalFiles);
    QCOMPARE(manager.fileList().count(), 9);
}

void AdvancedRenameTest::addFiles_should_only_add_files2()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath2);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath3);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath4);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath5);
    files << ps;
    AdvancedRenameManager manager;
    manager.addFiles(files);
    QCOMPARE(manager.fileList().count(), 5);
}

void AdvancedRenameTest::reset_removes_everything()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath2);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath3);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath4);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath5);
    files << ps;
    AdvancedRenameManager manager;
    manager.addFiles(files);
    QCOMPARE(manager.fileList().count(), 5);

    manager.reset();
    QCOMPARE(manager.fileList().count(), 0);
    QCOMPARE(manager.newFileList().count(), 0);
}

void AdvancedRenameTest::parseFiles_does_nothing_without_assigned_widget()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath2);
    files << ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath3);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles();

    QCOMPARE(manager.newName(filePath), filePath);
    QCOMPARE(manager.newName(filePath2), filePath2);
    QCOMPARE(manager.newName(filePath3), filePath3);
}

void AdvancedRenameTest::setStartIndex_invalid_index()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.setStartIndex(-1);
    manager.parseFiles(QLatin1String("####"));

    QCOMPARE(manager.newName(filePath), QLatin1String("0001.jpg"));
}

void AdvancedRenameTest::setStartIndex_sequencenumber_no_custom_start()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;

    QString parseString(QLatin1String("####"));

    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QCOMPARE(manager.newName(filePath), QLatin1String("0001.jpg"));

    manager.setStartIndex(12);
    manager.parseFiles(parseString);
    QCOMPARE(manager.newName(filePath), QLatin1String("0012.jpg"));

    manager.setStartIndex(-1000);
    manager.parseFiles(parseString);
    QCOMPARE(manager.newName(filePath), QLatin1String("0001.jpg"));
}

void AdvancedRenameTest::setStartIndex_sequencenumber_with_custom_start()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;

    QString parseString(QLatin1String("####[666]"));

    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QCOMPARE(manager.newName(filePath), QLatin1String("0666.jpg"));

    manager.setStartIndex(12);
    manager.parseFiles(parseString);
    QCOMPARE(manager.newName(filePath), QLatin1String("0666.jpg"));

    manager.setStartIndex(-1000);
    manager.parseFiles(parseString);
    QCOMPARE(manager.newName(filePath), QLatin1String("0666.jpg"));
}

void AdvancedRenameTest::sequencenumber_tests_data()
{
    QStringList files;
    files << filePath << filePath2 << filePath3;

    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QStringList>("files");
    QTest::addColumn<QStringList>("results");

    QTest::newRow("####")
            << "####"
            << files
            << (QStringList() << QLatin1String("0001.jpg") << QLatin1String("0002.jpg") << QLatin1String("0003.jpg"));

    QTest::newRow("###[-2]")
            << "###[-2]"
            << files
            << (QStringList() << QLatin1String("001.jpg") << QLatin1String("002.jpg") << QLatin1String("003.jpg"));

    QTest::newRow("###[2]")
            << "###[2]"
            << files
            << (QStringList() << QLatin1String("002.jpg") << QLatin1String("003.jpg") << QLatin1String("004.jpg"));

    QTest::newRow("##[3,3]")
            << "##[3,3]"
            << files
            << (QStringList() << QLatin1String("03.jpg") << QLatin1String("06.jpg") << QLatin1String("09.jpg"));

    QTest::newRow("#[4,4]")
            << "#[4,4]"
            << files
            << (QStringList() << QLatin1String("4.jpg") << QLatin1String("8.jpg") << QLatin1String("12.jpg"));

    QTest::newRow("#[4,-4]")
            << "#[4,-4]"
            << files
            << (QStringList() << QLatin1String("4.jpg") << QLatin1String("5.jpg") << QLatin1String("6.jpg"));
}

void AdvancedRenameTest::sequencenumber_tests()
{
    QFETCH(QString,       parseString);
    QFETCH(QStringList,   files);
    QFETCH(QStringList,   results);

    ParseSettings ps;

    QList<ParseSettings> files2;
    foreach (const QString& file, files)
    {
        ps.fileUrl = QUrl::fromLocalFile(file);
        files2 << ps;
    }

    AdvancedRenameManager manager(files2);
    manager.parseFiles(parseString);

    QVERIFY(files.count() == results.count());

    for (int i = 0; i < files.count(); ++i)
    {
        QCOMPARE(manager.newName(files.at(i)), results.at(i));
    }
}

void AdvancedRenameTest::newFileList_tests_data()
{
    QStringList files;
    files << filePath << filePath2 << filePath3;

    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QStringList>("files");
    QTest::addColumn<QStringList>("results");

    QTest::newRow("####")
            << "####"
            << files
            << (QStringList() << QLatin1String("0001.jpg") << QLatin1String("0002.jpg") << QLatin1String("0003.jpg"));

    QTest::newRow("###[-2]")
            << "###[-2]"
            << files
            << (QStringList() << QLatin1String("001.jpg") << QLatin1String("002.jpg") << QLatin1String("003.jpg"));

    QTest::newRow("###[2]")
            << "###[2]"
            << files
            << (QStringList() << QLatin1String("002.jpg") << QLatin1String("003.jpg") << QLatin1String("004.jpg"));

    QTest::newRow("##[3,3]")
            << "##[3,3]"
            << files
            << (QStringList() << QLatin1String("03.jpg") << QLatin1String("06.jpg") << QLatin1String("09.jpg"));

    QTest::newRow("#[4,4]")
            << "#[4,4]"
            << files
            << (QStringList() << QLatin1String("4.jpg") << QLatin1String("8.jpg") << QLatin1String("12.jpg"));

    QTest::newRow("#[4,-4]")
            << "#[4,-4]"
            << files
            << (QStringList() << QLatin1String("4.jpg") << QLatin1String("5.jpg") << QLatin1String("6.jpg"));
}

void AdvancedRenameTest::newFileList_tests()
{
    QFETCH(QString,       parseString);
    QFETCH(QStringList,   files);
    QFETCH(QStringList,   results);

    ParseSettings ps;

    QList<ParseSettings> files2;
    foreach (const QString& file, files)
    {
        ps.fileUrl = QUrl::fromLocalFile(file);
        files2 << ps;
    }

    AdvancedRenameManager manager(files2);
    manager.parseFiles(parseString);

    QVERIFY(files.count() == results.count());

    QMap<QString, QString> newFileList = manager.newFileList();

    for (int i = 0; i < files.count(); ++i)
    {
        QCOMPARE(newFileList.value(files.at(i)), results.at(i));
    }
}

void AdvancedRenameTest::indexOfFile_sorting_data()
{
    QStringList files;
    files << filePath << filePath2 << filePath3;

    QTest::addColumn<int>("sortAction");
    QTest::addColumn<int>("sortDirection");
    QTest::addColumn<QStringList>("files");
    QTest::addColumn<QList<int> >("results");

    QTest::newRow("custom_asc")
            << static_cast<int>(AdvancedRenameManager::SortCustom)
            << static_cast<int>(AdvancedRenameManager::SortAscending)
            << files
            << (QList<int>() << 1 << 2 << 3);

    QTest::newRow("custom_desc")
            << static_cast<int>(AdvancedRenameManager::SortCustom)
            << static_cast<int>(AdvancedRenameManager::SortDescending)
            << files
            << (QList<int>() << 1 << 2 << 3);

    QTest::newRow("name_asc")
            << static_cast<int>(AdvancedRenameManager::SortName)
            << static_cast<int>(AdvancedRenameManager::SortAscending)
            << files
            << (QList<int>() << 2 << 3 << 1);

    QTest::newRow("name_desc")
            << static_cast<int>(AdvancedRenameManager::SortName)
            << static_cast<int>(AdvancedRenameManager::SortDescending)
            << files
            << (QList<int>() << 2 << 1 << 3);
}

void AdvancedRenameTest::indexOfFile_sorting()
{
    QFETCH(int,           sortAction);
    QFETCH(int,           sortDirection);
    QFETCH(QStringList,   files);
    QFETCH(QList<int>,    results);

    ParseSettings ps;

    QList<ParseSettings> files2;
    foreach (const QString& file, files)
    {
        ps.fileUrl = QUrl::fromLocalFile(file);
        files2 << ps;
    }

    AdvancedRenameManager manager(files2);
    manager.setSortAction(static_cast<AdvancedRenameManager::SortAction>(sortAction));
    manager.setSortDirection(static_cast<AdvancedRenameManager::SortDirection>(sortDirection));

    QVERIFY(files.count() == results.count());

    for (int i = 0; i < files.count(); ++i)
    {
        QCOMPARE(manager.indexOfFile(files.at(i)), results.at(i));
    }
}

void AdvancedRenameTest::indexOfFile_invalid_input_returns_minus_one()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;

    AdvancedRenameManager manager(files);
    QCOMPARE(manager.indexOfFile(QLatin1String("none_existent_file.png")), -1);
}

void AdvancedRenameTest::indexOfFolder_invalid_input_returns_minus_one()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;

    AdvancedRenameManager manager(files);
    QCOMPARE(manager.indexOfFolder(QLatin1String("none_existent_file.png")), -1);
}

void AdvancedRenameTest::indexOfFileGroup_invalid_input_returns_minus_one()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;

    AdvancedRenameManager manager(files);
    QCOMPARE(manager.indexOfFileGroup(QLatin1String("none_existent_file.png")), -1);
}

void AdvancedRenameTest::sequencenumber_tests_startIndex_data()
{
    QStringList files;
    files << filePath << filePath2 << filePath3;

    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QStringList>("files");
    QTest::addColumn<QStringList>("results");

    QTest::newRow("####")
            << "####"
            << files
            << (QStringList() << QLatin1String("0025.jpg") << QLatin1String("0026.jpg") << QLatin1String("0027.jpg"));

    QTest::newRow("###[-2]")
            << "###[-2]"
            << files
            << (QStringList() << QLatin1String("025.jpg") << QLatin1String("026.jpg") << QLatin1String("027.jpg"));

    QTest::newRow("###[2]")
            << "###[2]"
            << files
            << (QStringList() << QLatin1String("002.jpg") << QLatin1String("003.jpg") << QLatin1String("004.jpg"));

    QTest::newRow("##[3,3]")
            << "##[3,3]"
            << files
            << (QStringList() << QLatin1String("03.jpg") << QLatin1String("06.jpg") << QLatin1String("09.jpg"));
}

void AdvancedRenameTest::sequencenumber_tests_startIndex()
{
    QFETCH(QString,       parseString);
    QFETCH(QStringList,   files);
    QFETCH(QStringList,   results);

    ParseSettings ps;

    QList<ParseSettings> files2;
    foreach (const QString& file, files)
    {
        ps.fileUrl = QUrl::fromLocalFile(file);
        files2 << ps;
    }

    AdvancedRenameManager manager(files2);
    manager.setStartIndex(25);
    manager.parseFiles(parseString);

    QVERIFY(files.count() == results.count());

    for (int i = 0; i < files.count(); ++i)
    {
        QCOMPARE(manager.newName(files.at(i)), results.at(i));
    }
}

void AdvancedRenameTest::sortAction_custom_asc_should_not_sort()
{
    QList<ParseSettings> files;
    ParseSettings ps;

    QStringList filePaths;
    filePaths << filePath << filePath2 << filePath3
              << filePath4 << filePath5 << filePath6
              << filePath7 << filePath8 << filePath9;

    foreach (const QString& filePath, filePaths)
    {
        ps.fileUrl = QUrl::fromLocalFile(filePath);
        files << ps;
    }

    AdvancedRenameManager manager(files);

    QStringList managedFiles = manager.fileList();
    QVERIFY(managedFiles.count() == filePaths.count());

    for (int i = 0; i < managedFiles.count(); ++i)
    {
        QCOMPARE(managedFiles.at(i), filePaths.at(i));
    }
}

void AdvancedRenameTest::sortAction_custom_desc_should_not_sort()
{
    QList<ParseSettings> files;
    ParseSettings ps;

    QStringList filePaths;
    filePaths << filePath << filePath2 << filePath3
              << filePath4 << filePath5 << filePath6
              << filePath7 << filePath8 << filePath9;

    foreach (const QString& filePath, filePaths)
    {
        ps.fileUrl = QUrl::fromLocalFile(filePath);
        files << ps;
    }

    AdvancedRenameManager manager(files);
    manager.setSortDirection(AdvancedRenameManager::SortDescending);

    QStringList managedFiles = manager.fileList();
    QVERIFY(managedFiles.count() == filePaths.count());

    for (int i = 0; i < managedFiles.count(); ++i)
    {
        QCOMPARE(managedFiles.at(i), filePaths.at(i));
    }
}

void AdvancedRenameTest::sortAction_name_asc()
{
    QList<ParseSettings> files;
    ParseSettings ps;

    QStringList filePaths;
    filePaths << filePath << filePath2 << filePath3
              << filePath4 << filePath5 << filePath6
              << filePath7 << filePath8 << filePath9;

    foreach (const QString& filePath, filePaths)
    {
        ps.fileUrl = QUrl::fromLocalFile(filePath);
        files << ps;
    }
    filePaths.sort();

    AdvancedRenameManager manager(files);
    manager.setSortAction(AdvancedRenameManager::SortName);

    QStringList managedFiles = manager.fileList();
    QVERIFY(managedFiles.count() == filePaths.count());

    for (int i = 0; i < managedFiles.count(); ++i)
    {
        QCOMPARE(managedFiles.at(i), filePaths.at(i));
    }
}

void AdvancedRenameTest::sortAction_name_desc()
{
    QList<ParseSettings> files;
    ParseSettings ps;

    QStringList filePaths;
    filePaths << filePath << filePath2 << filePath3
              << filePath4 << filePath5 << filePath6
              << filePath7 << filePath8 << filePath9;

    foreach (const QString& filePath, filePaths)
    {
        ps.fileUrl = QUrl::fromLocalFile(filePath);
        files << ps;
    }
    filePaths.sort();
    std::reverse(filePaths.begin(), filePaths.end());

    AdvancedRenameManager manager(files);
    manager.setSortAction(AdvancedRenameManager::SortName);
    manager.setSortDirection(AdvancedRenameManager::SortDescending);

    QStringList managedFiles = manager.fileList();
    QVERIFY(managedFiles.count() == filePaths.count());

    for (int i = 0; i < managedFiles.count(); ++i)
    {
        QCOMPARE(managedFiles.at(i), filePaths.at(i));
    }
}

void AdvancedRenameTest::sortAction_size_asc()
{
    QList<ParseSettings> files;
    ParseSettings ps;

    QStringList filePaths;
    filePaths << filePath7 << filePath8 << filePath9
              << filePath3 << filePath << filePath2
              << filePath5 << filePath4 << filePath6;

    foreach (const QString& filePath, filePaths)
    {
        ps.fileUrl = QUrl::fromLocalFile(filePath);
        files << ps;
    }

    AdvancedRenameManager manager(files);
    manager.setSortAction(AdvancedRenameManager::SortSize);

    QStringList managedFiles = manager.fileList();
    QVERIFY(managedFiles.count() == filePaths.count());

    for (int i = 0; i < managedFiles.count(); ++i)
    {
        QCOMPARE(managedFiles.at(i), filePaths.at(i));
    }
}

void AdvancedRenameTest::sortAction_size_desc()
{
    QList<ParseSettings> files;
    ParseSettings ps;

    QStringList filePaths;
    filePaths << filePath6 << filePath4 << filePath5
              << filePath2 << filePath << filePath3
              << filePath9 << filePath8 << filePath7;

    foreach (const QString& filePath, filePaths)
    {
        ps.fileUrl = QUrl::fromLocalFile(filePath);
        files << ps;
    }

    AdvancedRenameManager manager(files);
    manager.setSortAction(AdvancedRenameManager::SortSize);
    manager.setSortDirection(AdvancedRenameManager::SortDescending);

    QStringList managedFiles = manager.fileList();
    QVERIFY(managedFiles.count() == filePaths.count());

    for (int i = 0; i < managedFiles.count(); ++i)
    {
        QCOMPARE(managedFiles.at(i), filePaths.at(i));
    }
}

/*
void AdvancedRenameTest::sortAction_date_asc()
{
    QList<ParseSettings> files;
    ParseSettings ps;

    QStringList filePaths;
    filePaths << filePath4 << filePath3 << filePath
              << filePath2 << filePath9 << filePath7
              << filePath8 << filePath6 << filePath5;

    foreach (const QString& filePath, filePaths)
    {
        ps.fileUrl = QUrl::fromLocalFile(filePath);
        files << ps;
    }

    AdvancedRenameManager manager(files);
    manager.setSortAction(AdvancedRenameManager::SortDate);

    QStringList managedFiles = manager.fileList();
    QVERIFY(managedFiles.count() == filePaths.count());

    for (int i = 0; i < managedFiles.count(); ++i)
    {
        QCOMPARE(managedFiles.at(i), filePaths.at(i));
    }
}

void AdvancedRenameTest::sortAction_date_desc()
{
    QList<ParseSettings> files;
    ParseSettings ps;

    QStringList filePaths;
    filePaths << filePath5 << filePath6 << filePath8
              << filePath7 << filePath9 << filePath2
              << filePath << filePath3 << filePath4;

    foreach (const QString& filePath, filePaths)
    {
        ps.fileUrl = QUrl::fromLocalFile(filePath);
        files << ps;
    }

    AdvancedRenameManager manager(files);
    manager.setSortAction(AdvancedRenameManager::SortDate);
    manager.setSortDirection(AdvancedRenameManager::SortDescending);

    QStringList managedFiles = manager.fileList();
    QVERIFY(managedFiles.count() == filePaths.count());

    for (int i = 0; i < managedFiles.count(); ++i)
    {
        QCOMPARE(managedFiles.at(i), filePaths.at(i));
    }
}
*/

void AdvancedRenameTest::testReplaceModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QTest::newRow("[file]{replace:\"adv\",\"AAA\"}")
            << "[file]{replace:\"adv\",\"AAA\"}"
            << "AAAancedrename_testimage.jpg";

    QTest::newRow("[file]{replace:\"Adv\",\"AAA\"}")
            << "[file]{replace:\"Adv\",\"AAA\"}"
            << "advancedrename_testimage.jpg";

    QTest::newRow("[file]{replace:\"Adv\",\"AAA\",i}")
            << "[file]{replace:\"Adv\",\"AAA\",i}"
            << "AAAancedrename_testimage.jpg";

    QTest::newRow("[file]{replace:\"Adv\",\"AAA\",ri}")
            << "[file]{replace:\"Adv\",\"AAA\",ri}"
            << "AAAancedrename_testimage.jpg";

    QTest::newRow("[file]{replace:\"Adv\",\"AAA\",ir}")
            << "[file]{replace:\"Adv\",\"AAA\",ir}"
            << "AAAancedrename_testimage.jpg";

    QTest::newRow("[file]{replace:\"a.v\",\"AAA\"}")
            << "[file]{replace:\"a.v\",\"AAA\"}"
            << "advancedrename_testimage.jpg";

    QTest::newRow("[file]{replace:\"a.v\",\"AAA\",r}")
            << "[file]{replace:\"a.v\",\"AAA\",r}"
            << "AAAancedrename_testimage.jpg";
}

void AdvancedRenameTest::testReplaceModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testRangeModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("[file]{range:1}")
            << "[file]{range:1}"
            << "a.jpg";

    QTest::newRow("[file]{range:3}")
            << "[file]{range:3}"
            << "v.jpg";

    QTest::newRow("[file]{range:1,3}")
            << "[file]{range:1,3}"
            << "adv.jpg";

    QTest::newRow("[file]{range:3,}")
            << "[file]{range:3,}"
            << "vancedrename_testimage.jpg";

    QTest::newRow("[file]{range:0}")
            << "[file]{range:0}"
            << "advancedrename_testimage.jpg";

    QTest::newRow("[file]{range:-100}")
            << "[file]{range:-100}"
            << "a.jpg";

    QTest::newRow("[file]{range:-100,}")
            << "[file]{range:-100,}"
            << "advancedrename_testimage.jpg";

    QTest::newRow("[file]{range:-100,2}")
            << "[file]{range:-100,2}"
            << "ad.jpg";
}

void AdvancedRenameTest::testRangeModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testDefaultValueModifier_data()
{
    QTest::addColumn<QString>("parseString");
    QTest::addColumn<QString>("result");

    QDateTime curdate = QDateTime::currentDateTime();

    QTest::newRow("[meta:Iptc.Application2.Keywords]_[file]")
            << "[meta:Iptc.Application2.Keywords]{default:\"Unknown\"}_[file]"
            << "Colca Canyon_advancedrename_testimage.jpg";

    QTest::newRow("[meta:Exif.GPSInfo.GPSAltitude]_[file]")
            << "[meta:Exif.GPSInfo.GPSAltitude]{default:\"Unknown\"}_[file]"
            << "Unknown_advancedrename_testimage.jpg";
}

void AdvancedRenameTest::testDefaultValueModifier()
{
    QFETCH(QString,   parseString);
    QFETCH(QString,   result);

    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(parseString);

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, result);
}

void AdvancedRenameTest::testLowercaseModifier()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);
    manager.parseFiles(QLatin1String("[file]{lower}"));

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, fileName.toLower());
}

void AdvancedRenameTest::testEmptyParseString()
{
    QList<ParseSettings> files;
    ParseSettings ps;
    ps.fileUrl = QUrl::fromLocalFile(filePath);
    files << ps;
    AdvancedRenameManager manager(files);

    // test for empty parser string
    manager.parseFiles(QLatin1String(""));

    QString parsed = manager.newName(filePath);
    QCOMPARE(parsed, fileName);

    manager.parseFiles(QLatin1String("      "));
    parsed = manager.newName(filePath);
    QCOMPARE(parsed, fileName);
}
