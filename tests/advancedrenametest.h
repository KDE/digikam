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

#ifndef ADVANCEDRENAMEINPUTTEST_H
#define ADVANCEDRENAMEINPUTTEST_H

// Qt includes

#include <QtCore/QObject>

class AdvancedRenameWidgetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    /*
     * TOKEN TESTS
     */
    void testFileNameToken();
    void testFileNameToken_data();

    void testFileExtensionToken();
    void testFileExtensionToken_data();

    void testDirectoryNameToken();
    void testDirectoryNameToken_data();

    void testNumberToken();
    void testNumberToken_data();

    void testCameraToken();
    void testCameraToken_data();

    /*
     * MODIFIER TESTS
     */
    void testRangeModifier();
    void testRangeModifier_data();

    void testDefaultValueModifier();
    void testDefaultValueModifier_data();

    void testUppercaseModifier();
    void testUppercaseModifier_data();

    void testLowercaseModifier();
    void testLowercaseModifier_data();

    void testTrimmedModifier();
    void testTrimmedModifier_data();

    void testFirstLetterOfEachWordUppercaseModifier();
    void testFirstLetterOfEachWordUppercaseModifier_data();

    void testChainedModifiers();
    void testChainedModifiers_data();

    /*
     * OTHER TESTS
     */
    void testEmptyParseString();
};

#endif /* ADVANCEDRENAMEINPUTTEST_H_ */
