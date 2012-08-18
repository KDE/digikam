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

#ifndef UIFILEVALIDATORTEST_H
#define UIFILEVALIDATORTEST_H

// Qt includes

#include <QtTest/QtTest>

class QByteArray;
class QFile;

class UiFileValidatorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testInvalidUiFile();
    void testValidUiFile();
    void testGetFixedContent();
    void testFixConfigFile();

private:

    bool isReadable(QFile& file) const;
    bool isWritable(QFile& file) const;

    QByteArray readContent(const QString& filename);

    bool removeFile(const QString& filename);
};

#endif /* UIFILEVALIDATORTEST_H */
