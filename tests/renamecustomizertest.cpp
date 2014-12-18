/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the AdvancedRename utility
 *
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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


// Qt includes

#include <QFileInfo>
#include <QDateTime>

// KDE includes

#include <kurl.h>
#include <qtest_kde.h>

// Local includes

#include "advancedrenamemanager.h"
#include "defaultrenameparser.h"
#include "parsesettings.h"

#include "renamecustomizer.h"


using namespace Digikam;

const QString imagesDir("advancedrenameimages/");

QString createFilePath(const QString& file)
{
    return QString(KDESRCDIR + imagesDir + file);
}

const QString fileName  = "advancedrename_testimage.jpg";
const QString fileName2 = "advancedrename_testimage2.jpg";
const QString fileName3 = "001a.jpg";
const QString fileName4 = "test.png";
const QString fileName5 = "myfile.jpg";
const QString fileName6 = "my_file.jpg";
const QString fileName7 = "holiday_spain_2011_img001.jpg";
const QString fileName8 = "my images.jpg";
const QString fileName9 = "holiday_spain_2011_001img.jpg";

const QString filePath = createFilePath(fileName);
const QString filePath2 = createFilePath(fileName2);
const QString filePath3 = createFilePath(fileName3);
const QString filePath4 = createFilePath(fileName4);
const QString filePath5 = createFilePath(fileName5);
const QString filePath6 = createFilePath(fileName6);
const QString filePath7 = createFilePath(fileName7);
const QString filePath8 = createFilePath(fileName8);
const QString filePath9 = createFilePath(fileName9);


class RenameCustomizerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void newName_should_return_empty_string_with_empty_filename_data();
    void newName_should_return_empty_string_with_empty_filename();

    void setCaseType_set_to_none();
    void setCaseType_set_to_upper();
    void setCaseType_set_to_lower();

    void setUseDefault_true();
    void setUseDefault_false();
    void setUseDefault_case_none_should_deliver_original_filename();
    void setUseDefault_case_upper_should_deliver_uppercase_filename();
    void setUseDefault_case_lower_should_deliver_lowercase_filename();
};



QTEST_KDEMAIN(RenameCustomizerTest, GUI)
#include "renamecustomizertest.moc"



void RenameCustomizerTest::newName_should_return_empty_string_with_empty_filename_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("result");

    QTest::newRow("empty")          << QString("")      << QString("");
    QTest::newRow("whitespaces")    << QString("    ")  << QString("");
}

void RenameCustomizerTest::newName_should_return_empty_string_with_empty_filename()
{
    QFETCH(QString, filename);
    QFETCH(QString, result);

    RenameCustomizer customizer(0, "Unit Tests");
    QCOMPARE(customizer.newName(filename, QDateTime::currentDateTime()), result);
}

void RenameCustomizerTest::setCaseType_set_to_none()
{
    RenameCustomizer customizer(0, "Unit Tests");
    customizer.setChangeCase(RenameCustomizer::NONE);
    QCOMPARE(customizer.changeCase(), RenameCustomizer::NONE);
}

void RenameCustomizerTest::setCaseType_set_to_upper()
{
    RenameCustomizer customizer(0, "Unit Tests");
    customizer.setChangeCase(RenameCustomizer::UPPER);
    QCOMPARE(customizer.changeCase(), RenameCustomizer::UPPER);
}

void RenameCustomizerTest::setCaseType_set_to_lower()
{
    RenameCustomizer customizer(0, "Unit Tests");
    customizer.setChangeCase(RenameCustomizer::LOWER);
    QCOMPARE(customizer.changeCase(), RenameCustomizer::LOWER);
}

void RenameCustomizerTest::setUseDefault_true()
{
    RenameCustomizer customizer(0, "Unit Tests");
    customizer.setUseDefault(true);
    QVERIFY(customizer.useDefault());
}

void RenameCustomizerTest::setUseDefault_false()
{
    RenameCustomizer customizer(0, "Unit Tests");
    customizer.setUseDefault(false);
    QVERIFY(customizer.useDefault() == false);
}

void RenameCustomizerTest::setUseDefault_case_none_should_deliver_original_filename()
{
    RenameCustomizer customizer(0, "Unit Tests");
    customizer.setUseDefault(true);
    customizer.setChangeCase(RenameCustomizer::NONE);
    QCOMPARE(customizer.newName("TeSt.png", QDateTime::currentDateTime()), QString("TeSt.png"));
}

void RenameCustomizerTest::setUseDefault_case_upper_should_deliver_uppercase_filename()
{
    RenameCustomizer customizer(0, "Unit Tests");
    customizer.setUseDefault(true);
    customizer.setChangeCase(RenameCustomizer::UPPER);
    QCOMPARE(customizer.newName("TeSt.png", QDateTime::currentDateTime()), QString("TEST.PNG"));
}

void RenameCustomizerTest::setUseDefault_case_lower_should_deliver_lowercase_filename()
{
    RenameCustomizer customizer(0, "Unit Tests");
    customizer.setUseDefault(true);
    customizer.setChangeCase(RenameCustomizer::LOWER);
    QCOMPARE(customizer.newName("TeSt.pnG", QDateTime::currentDateTime()), QString("test.png"));
}
