/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : Jun 9, 2009
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

#ifndef MANUALRENAMEINPUTTEST_H_
#define MANUALRENAMEINPUTTEST_H_

// Qt includes

#include <QtCore/QObject>

class ManualRenameInputTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testNumberToken();
    void testNumberToken_data();

    void testEachLetterUppercaseToken();
    void testEachLetterUppercaseToken_data();

    void testUppercaseToken();
    void testUppercaseToken_data();

    void testLowercaseToken();
    void testLowercaseToken_data();

    void testCameraToken();
    void testCameraToken_data();
};

#endif /* MANUALRENAMEINPUTTEST_H_ */
