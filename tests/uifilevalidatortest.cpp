/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-05
 * Description : a test for the UiFileValidator class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "uifilevalidatortest.moc"

// Qt includes

#include <QByteArray>
#include <QFileInfo>
#include <QString>

// Local includes

#include "uifilevalidator.h"

const QString goodFile(KDESRCDIR"digikamui_good.rc");
const QString badFile(KDESRCDIR"digikamui_bad.rc");
const QString fixedFile(KDESRCDIR"fixedui.rc");

using namespace Digikam;

QTEST_MAIN(UiFileValidatorTest)

bool UiFileValidatorTest::isReadable(QFile& file) const
{
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    return true;
}

bool UiFileValidatorTest::isWritable(QFile& file) const
{
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    return true;
}

QByteArray UiFileValidatorTest::readContent(const QString& filename)
{
    QFile fi(filename);

    if (!isReadable(fi))
    {
        return QByteArray();
    }

    QByteArray content = fi.readAll();
    fi.close();
    return content;
}

bool UiFileValidatorTest::removeFile(const QString& filename)
{
    QFile fi(filename);

    if (fi.exists())
    {
        return fi.remove();
    }

    return false;
}

void UiFileValidatorTest::testInvalidUiFile()
{
    QFileInfo fi(badFile);
    QVERIFY(fi.isReadable());

    // check for valid results
    UiFileValidator validator(fi.absoluteFilePath());
    QVERIFY(!validator.isValid());
}

void UiFileValidatorTest::testValidUiFile()
{
    QFileInfo fi(goodFile);
    QVERIFY(fi.isReadable());

    // check for valid results
    UiFileValidator validator(fi.absoluteFilePath());
    QVERIFY(validator.isValid());
}

void UiFileValidatorTest::testGetFixedContent()
{
    UiFileValidator validator(badFile);

    QByteArray goodContent = readContent(goodFile);
    QByteArray badContent  = readContent(badFile);

    // check for valid results
    QVERIFY(!goodContent.isEmpty());
    QVERIFY(!badContent.isEmpty());
    QVERIFY(badContent != goodContent);
    QCOMPARE(validator.getFixedContent(), goodContent);
}

void UiFileValidatorTest::testFixConfigFile()
{

    UiFileValidator validator(badFile);
    validator.fixConfigFile(fixedFile);

    QByteArray goodContent  = readContent(goodFile);
    QByteArray fixedContent = readContent(fixedFile);

    // remove the temporary fixed file again
    QVERIFY(removeFile(fixedFile));

    // check for valid results
    QVERIFY(!goodContent.isEmpty());
    QVERIFY(!fixedContent.isEmpty());
    QCOMPARE(fixedContent, goodContent);
}
